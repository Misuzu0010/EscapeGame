# GameThread / WorkerThread 与风力组件经验笔记

生成日期：2026-05-25

本文档记录本轮讨论中沉淀出的 UE 多线程边界、数据快照、异步加载和风力组件调试经验。重点不是马上把项目改成多线程，而是建立后续扩展时不容易出错的设计边界。

## 1. GameThread 与 WorkerThread 的基本边界

在 UE 玩法代码里，可以先用一句话理解：

```text
WorkerThread 负责算，GameThread 负责改 UE 世界。
```

推荐的数据流：

```text
GameThread
  收集 Actor / Component / UObject 数据，复制成普通数据快照

WorkerThread
  只处理普通 C++ 数据，完成排序、评分、压缩、数学计算等任务

GameThread
  把结果应用回 Actor / Component / UI / World
```

GameThread 适合做：

- 访问 `UObject`、`AActor`、`UActorComponent`、`UWorld`。
- 播放 Montage。
- Spawn / Destroy Actor。
- 执行碰撞查询。
- 修改 Widget。
- 广播动态委托。
- 修改背包、战斗、交互、状态机等玩法状态。

WorkerThread 适合做：

- 排序、筛选、搜索。
- AI 评分。
- 存档序列化、压缩、写文件。
- 复杂数学计算。
- 对普通 `struct` / 数组做批处理。

WorkerThread 不建议直接做：

- 访问 Actor / Component / Widget / World。
- 修改 `UPROPERTY` 玩法状态。
- 播放动画或调用蓝图事件。
- 执行碰撞查询。
- 广播动态委托。
- 同步加载资源。

## 2. 死锁与 Data Race 是两类问题

死锁关注的是：

```text
线程 A 等线程 B
线程 B 又等线程 A
双方互相等待，程序卡住
```

Data Race 关注的是：

```text
线程 A 正在读或写某份数据
线程 B 同时写同一份数据
最终读到半更新状态，或者覆盖彼此结果
```

判断风险时可以用这个规则：

```text
两个线程同时读同一份数据：通常没问题
一个线程读，另一个线程写：可能有问题
两个线程同时写同一份数据：高风险
```

当前项目中，背包、战斗、交互、UI、冲刺、状态机等主要玩法逻辑基本都在 GameThread 上运行，因此暂时没有明显的“线程 A/B 同时修改同一份玩法数据”的风险。

需要长期注意的运行时数据：

- `UInventoryComponent::Items`
- `UEscapeCombatComponent` 的动作状态、Tag、ComboCount
- `USprintComponent` 的耐力和速度 Buff 状态
- `UStateMachineComponent` 的当前状态
- `UCharacterAnimInstance` 的动画缓存变量

这些数据后续如果要跨线程使用，优先做快照，不要让 WorkerThread 直接改组件成员。

## 3. AnimInstance 中的线程边界

当前项目最清晰的线程边界在 `UCharacterAnimInstance`。

`NativeUpdateAnimation()` 可以理解为 GameThread 数据采集阶段：

```cpp
void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
```

它适合读取：

- `MovementComponent`
- `SprintComp`
- `WindComponent`
- `ClothLODComponent`
- `OwnerCharacter`

`NativeThreadSafeUpdateAnimation()` 可以理解为动画 WorkerThread 的计算阶段：

```cpp
void UCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaTime)
```

它应该只读取已经缓存好的普通变量，并计算动画参数。这里不要访问 Actor、Component、World、Widget，也不要修改背包、战斗或状态机。

当前项目中的模式是正确方向：

```text
NativeUpdateAnimation
  从组件读取数据，写入缓存变量

NativeThreadSafeUpdateAnimation
  只读缓存变量，计算 LocomotionAngle、LocomotionPlayRate、PhysicsAlpha 等动画输出
```

## 4. Snapshot 结构体模式

Snapshot 结构体不是 UE 特殊机制，而是一种组织方式：把某一帧需要跨阶段或跨线程使用的数据打包成一份普通 C++ 数据。

示例：

```cpp
struct FAnimRuntimeSnapshot
{
	FVector Velocity = FVector::ZeroVector;
	FRotator Rotation = FRotator::ZeroRotator;
	float GroundSpeed = 0.f;
	float ClothLODFactor = 0.f;
	bool bIsFalling = false;
	bool bIsRunning = false;
};
```

它的意义类似于：

```text
GameThread 给当前角色状态拍一张照片。
WorkerThread 不碰角色本体，只看这张照片做计算。
```

推荐用法：

```cpp
FAnimRuntimeSnapshot NewSnapshot;

// GameThread: 从组件读取数据
NewSnapshot.Velocity = MovementComponent->Velocity;
NewSnapshot.GroundSpeed = NewSnapshot.Velocity.Size2D();
NewSnapshot.Rotation = OwnerCharacter->GetActorRotation();

AnimSnapshot = NewSnapshot;
```

WorkerThread 侧：

```cpp
const FAnimRuntimeSnapshot Snapshot = AnimSnapshot;

const FVector LocalVelocity = Snapshot.Rotation.UnrotateVector(Snapshot.Velocity);
const float Angle = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));
```

注意：

- Snapshot 本身不等于自动线程安全。
- 它的主要价值是让数据边界更清楚。
- 如果未来有严格并发读写需求，再考虑双缓冲、锁或原子变量。

适合使用 Snapshot 的未来功能：

- 动画缓存：`FAnimRuntimeSnapshot`
- 存档系统：`FGameSaveSnapshot`
- 背包排序/搜索：`FInventorySortSnapshot`
- AI 评分：`FAIWorldSnapshot`
- 多角色布料 LOD 批处理：`FClothLODSnapshot`

不建议用 Snapshot 绕一圈的场景：

- 普通拾取。
- 开门。
- 使用物品。
- 播放 Montage。
- UI 刷新。
- 碰撞查询。
- Actor Spawn / Destroy。

经验口诀：

```text
一帧内立刻完成的 GameThread 玩法逻辑：不用快照
跨线程 / 跨帧 / 异步回调的数据：适合快照
WorkerThread 需要的数据：优先快照化
```

## 5. 异步加载回调

异步加载回调可以理解为：

```text
先发起资源加载请求，不阻塞当前帧
资源加载完成后，UE 调用提前绑定的回调函数
回调中再把资源缓存或应用到游戏对象
```

当前战斗组件中有同步加载点：

```cpp
UAnimMontage* MontageToPlay = ActionDef->Montage.LoadSynchronous();
```

这通常不会造成死锁，但如果资源第一次加载发生在攻击输入时，可能造成卡顿。

更适合的方向：

```text
BeginPlay / 初始化阶段
  收集 CombatActionMap 中的 SoftObjectPath
  发起异步加载

加载完成回调
  回到 GameThread
  检查组件和角色是否仍然有效
  缓存 UAnimMontage*

攻击输入
  直接从缓存取 Montage 播放
```

回调中必须注意：

- 角色可能已经销毁。
- 组件可能已经 EndPlay。
- 玩家状态可能已经变化。
- 加载完成不等于当前仍然应该播放动作。

因此回调中要重新验证 `IsValid()` 和当前状态。

## 6. 锁机制不要当成优化手段

当前项目不建议主动加锁。

原因：

- 当前没有大量自建 WorkerThread。
- 大部分玩法数据只在 GameThread 修改。
- 强行加锁会增加复杂度，甚至引入死锁和卡顿。

如果未来确实需要锁，建议只锁普通数据：

```cpp
FCriticalSection DataLock;

{
	FScopeLock Lock(&DataLock);
	// 只读写普通 C++ 数据
}
```

锁内不要做：

- `LoadSynchronous`
- `Broadcast`
- `SpawnActor`
- `DestroyActor`
- 播放 Montage
- 刷新 UI
- 调用蓝图事件
- 等待其他线程

简单状态可以优先考虑 `TAtomic`：

```cpp
TAtomic<bool> bTaskFinished;
TAtomic<int32> PendingCount;
```

## 7. WindSimulationComponent 的排查经验

风力组件当前不是线程安全问题，主要是逻辑输出和调试方式问题。

### 日志看不到的常见原因

`UE_LOG` 默认不会显示在游戏画面上，它会输出到 Output Log 或 IDE 控制台。

如果希望在屏幕上看到，需要使用：

```cpp
GEngine->AddOnScreenDebugMessage(...);
```

排查顺序：

1. 确认当前玩家角色蓝图里是否真的挂了 `WindSimulationComponent`。
2. 确认组件 Tick 没有被禁用。
3. 运行 PIE 后打开 `Window -> Developer Tools -> Output Log`。
4. 搜索 `TargetWind`。
5. 如果完全没有日志，优先怀疑组件没有挂载或没有 Tick。

### 当前逻辑中的关键点

组件里计算了噪声风：

```cpp
CachedNoiseWind = FVector(NoiseX, NoiseY, NoiseZ) * NoiseIntensity;
TargetWind = CurrentWind + CachedNoiseWind;
```

但外部读取的是：

```cpp
inline FVector GetCurrentWind() const { return CurrentWind; }
```

这意味着动画当前很可能只拿到了基础风和移动风，没有拿到 `TargetWind` 中的噪声风。

推荐后续设计：

```cpp
FVector GetBaseWind() const;
FVector GetFinalWind() const;
```

语义：

```text
BaseWind / CurrentWind
  平滑后的基础风 + 移动风

FinalWind / TargetWind
  基础风 + 噪声风，是真正给动画或物理使用的最终风
```

然后 AnimInstance 使用最终风：

```text
KawaiiWind = WindComponent->GetFinalWind() * KawaiiMultiplier
```

### 关于 MovementWindScale

`MovementWindScale` 已经是 `UPROPERTY(EditAnywhere)`，可以在角色蓝图或组件实例中调参。C++ 默认值偏小不一定是代码 Bug，如果蓝图里已经覆盖，就以蓝图配置为准。

经验：

```text
C++ 默认值 = 兜底值
蓝图实例值 = 当前调参结果
```

分析数值问题时要同时看源码默认值和蓝图实际值。

### 关于 NoiseUpdateInterval / WindNoiseCurve / CurvePlayRate

这些字段目前更像未来扩展点。

短期建议：

```text
先不处理曲线和 NoiseUpdateInterval
先保证组件挂载、Tick 正常、最终风能被 AnimInstance 使用
```

中期建议：

```text
如果多角色都挂风力组件，再让 NoiseUpdateInterval 生效
按间隔更新噪声目标，每帧插值到目标噪声
```

后期建议：

```text
如果需要 TA 控制风的节奏，再接入 WindNoiseCurve
用曲线调制噪声强度或 Z 轴起伏
```

不要为了一个暂时没有明确需求的曲线字段，过早把风力系统复杂化。

## 8. 对当前项目的推荐优先级

近期最值得做：

1. 确认 `WindSimulationComponent` 是否挂到当前玩家蓝图，并正常 Tick。
2. 确认风力日志在 Output Log 中查看，而不是期待显示在屏幕上。
3. 拆分 `GetBaseWind()` / `GetFinalWind()`，让动画读取最终风。
4. 保留 `MovementWindScale` 作为蓝图调参项。
5. 暂缓 `NoiseUpdateInterval` 和 `WindNoiseCurve`，等性能或 TA 需求明确后再做。

长期设计原则：

```text
玩法状态默认 GameThread 修改
WorkerThread 只算纯数据
跨线程数据用 Snapshot 表达边界
异步加载用回调，回调中重新验证状态
锁只保护普通数据，不包 UE 高层 API
调试日志先确认输出渠道和组件是否真的运行
```
