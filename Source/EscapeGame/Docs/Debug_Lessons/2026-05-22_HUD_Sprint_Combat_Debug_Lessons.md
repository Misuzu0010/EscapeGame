# 2026-05-22 HUD / Sprint / Combat 踩坑经验记录

本文件记录本轮调试中遇到的典型问题、根因判断方式和对应解决方案。后续遇到类似现象时，优先按这里的检查顺序排查。

## 1. 协作与修改规则

本项目协作规则要求：涉及 `.h` / `.cpp` / 蓝图 / 资产 / 架构调整时，先分析、再提案、最后由用户确认后执行。

例外情况：

- 读取、搜索、分析文件可以直接执行。
- 仅添加 `UE_LOG` 调试日志可以直接执行。
- 仅格式化代码可以直接执行。

经验：

- 不确定根因时，优先加诊断日志，不要直接重构。
- 每次只处理用户确认的一个问题和一个方案。
- 修改后立即停止并汇报，不连续处理下一个问题。

## 2. Combat：攻击播放失败后卡死在攻击状态

### 现象

轻击或蓄力攻击在 Montage 播放失败时，角色会卡在攻击状态，之后无法继续攻击。

### 根因

原逻辑在确认 Montage 成功播放之前就添加状态 Tag：

```cpp
ActiveTags.AddTag(EscapeGameplayTags::Action_State_Attacking);
TryPlayActionByTag(FirstAttackTag);
```

如果 `CharacterAnimData` 缺失、体力不足、ActionDef 缺失、Montage 为空，或 `PlayAnimMontage` 返回 0，就不会触发 Montage 结束回调，状态 Tag 也不会被清理。

### 解决方向

保留蓝图可见接口：

```cpp
UFUNCTION(BlueprintCallable, Category = "Combat")
void TryPlayActionByTag(FGameplayTag ActionTag);
```

新增仅 C++ 内部使用的 bool 函数：

```cpp
bool TryPlayActionByTagInternal(FGameplayTag ActionTag);
```

只有内部函数返回 `true` 时，才添加攻击 / 蓄力状态 Tag。

经验：

- 不要把 `BlueprintCallable` 的返回值从 `void` 改成 `bool`，否则可能影响已有蓝图节点。
- 蓝图接口保持不变，C++ 内部新增返回值函数，是兼顾兼容性和安全性的做法。

## 3. HUD：Widget 创建成功但血条体力条不显示

### 现象

组件全部挂载成功，HUDWidgetClass 也有值，但屏幕上看不到血条和体力条。

关键诊断日志：

```text
HUD诊断：IsLocallyControlled=true, HUDWidgetClass=GameHUDWidget, Controller=...
HUD诊断：CreateWidget 成功，准备 AddToViewport。
HUD诊断：InitializeWidget 被调用。
HUD诊断：BindWidget 状态 HealthBar=None, HealthText=None, StaminaBar=None, StaminaText=None, Hotbar=None
```

### 根因

HUD 已经创建并 AddToViewport，但 C++ `BindWidget` 没有绑定到任何 UMG 控件。

高概率原因是角色蓝图中的 `HUDWidgetClass` 选成了 C++ 原生类 `GameHUDWidget`，而不是带 Designer 控件的 Widget Blueprint。

### 解决方向

创建或使用 `WBP_GameHUDWidget`：

- Parent Class 必须是 `GameHUDWidget`。
- 角色蓝图里的 `HUDWidgetClass` 必须选择 `WBP_GameHUDWidget`，不要直接选 C++ 类 `GameHUDWidget`。
- Designer 里的控件名必须和 C++ 完全一致。

必须匹配的控件名：

```text
HealthProgressBar
HealthText
StaminaProgressBar
StaminaText
HotbarWidget
```

经验：

- `AddToViewport` 成功不代表 HUD 有内容。
- `BindWidget=None` 时，优先检查 Widget Blueprint 父类、控件命名和 `Is Variable`。

## 4. Sprint：Shift 有日志但无法持续冲刺

### 现象

按下 Shift 能看到日志，体力只掉一点，例如从 100 掉到 99，速度没有明显变化。

### 根因

原逻辑只允许 `Idle` / `Moving` 进入冲刺：

```cpp
bool bCanSprint =
    (CurrentState == ECharacterState::Moving || CurrentState == ECharacterState::Idle)
    && !OwnerCharacter->GetVelocity().IsZero();
```

第一帧进入 `Sprinting` 后，下一帧 `CurrentState == Sprinting`，但 `bCanSprint` 不允许 `Sprinting`，于是冲刺被自己打断。

### 解决方向

拆分“能否开始冲刺”和“能否维持冲刺”：

```cpp
const bool bHasMovement = !OwnerCharacter->GetVelocity().IsZero();
const bool bCanStartSprint =
    CurrentState == ECharacterState::Moving ||
    CurrentState == ECharacterState::Idle;
const bool bCanKeepSprint =
    CurrentState == ECharacterState::Sprinting;

bIsActurallySprinting =
    bSprintRequested &&
    (bCanStartSprint || bCanKeepSprint) &&
    bHasMovement &&
    !bStaminaDrained &&
    MovementComp->IsMovingOnGround() &&
    !OwnerCharacter->bIsCrouched;
```

经验：

- 状态机进入某状态后，维持条件必须允许当前状态自身存在。
- “开始条件”和“维持条件”不要混在一起，否则容易出现只生效一帧的问题。

## 5. Sprint：速度生效但动画姿态直立

### 现象

冲刺速度和体力消耗都正常，但角色动画像直立滑行，没有进入正确冲刺姿态。

### 根因判断

如果只是 BlendSpace 速度轴不匹配，通常表现为脚滑或播放速率不对。

如果进入 SprintSpeed 后姿态变成直立，通常是 AnimBP 的状态机切到了错误 Pose，例如：

- Sprint State 内部接了 Idle Pose。
- Sprint State 内部没有接正确的 Sprint 动画 / BlendSpace。
- Transition 条件进入了不该进入的 State。
- `bIsRunning` / `bIsSprinting` 变量语义混用。

C++ 中当前动画变量来源：

```cpp
bIsSprinting = SprintComp->ReturnSprintState();
bIsRunning = bIsSprinting;
```

### 解决方向

优先在 AnimBP Debug 中观察运行时状态机：

- Shift 冲刺时进入了哪个 State。
- 该 State 内部接的动画是否正确。
- Transition Rule 是否使用了正确变量。

经验：

- Movement 速度正常不代表动画状态正常。
- 姿态错误优先查 AnimBP 状态机，而不是先改 WalkSpeed / SprintSpeed。
- `bIsRunning = bIsSprinting` 语义不够清楚，后续建议拆分普通跑和冲刺。

## 6. Sprint：走路和冲刺切换动画突变

### 现象

走路和冲刺切换时动作像瞬间切换，不够平滑。

### 根因

当前速度逻辑直接写 `MaxWalkSpeed`：

```cpp
MovementComp->MaxWalkSpeed = SprintSpeed * CurrentBuffMultiplier;
MovementComp->MaxWalkSpeed = WalkSpeed * CurrentBuffMultiplier;
```

速度瞬间跳变，会让动画参数和移动表现都突然变化。

### 推荐解决方向

用平滑插值统一控制 `MaxWalkSpeed`。

头文件中保留或添加：

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
float SpeedInterpRate = 6.0f;

float CurrentSmoothedSpeed = 0.f;
```

`BeginPlay` 中初始化：

```cpp
CurrentSmoothedSpeed = WalkSpeed * CurrentBuffMultiplier;
MovementComp->MaxWalkSpeed = CurrentSmoothedSpeed;
```

`TickComponent` 中统一写速度：

```cpp
const float TargetSpeed =
    (bIsActurallySprinting ? SprintSpeed : WalkSpeed) * CurrentBuffMultiplier;

CurrentSmoothedSpeed =
    FMath::FInterpTo(CurrentSmoothedSpeed, TargetSpeed, DeltaTime, SpeedInterpRate);

MovementComp->MaxWalkSpeed = CurrentSmoothedSpeed;
```

`StartSprinting()` 和 `StopSprinting()` 只改请求标志，不直接写速度：

```cpp
void USprintComponent::StartSprinting()
{
    if (bStaminaDrained) return;
    bSprintRequested = true;
}

void USprintComponent::StopSprinting()
{
    bSprintRequested = false;
}
```

经验：

- `SpeedInterpRate` 是插值速率，不是速度倍率。
- 初始化不能写成 `WalkSpeed * SpeedInterpRate`。
- 算出 `CurrentSmoothedSpeed` 后，必须实际赋值给 `MovementComp->MaxWalkSpeed`。
- 速度最好只有一个地方写入，否则 Tick、Buff、输入事件会互相抢控制权。

## 7. 平滑插值验收清单

检查 `SprintComponent` 时按以下顺序验收：

1. `CurrentSmoothedSpeed` 初始值是否是 `WalkSpeed * CurrentBuffMultiplier`。
2. `TickComponent` 是否每帧计算 `TargetSpeed`。
3. `TickComponent` 是否调用 `FMath::FInterpTo`。
4. 插值结果是否赋值给 `MovementComp->MaxWalkSpeed`。
5. `StartSprinting()` 是否没有直接调用 `UpdateMovementSpeed()`。
6. `SetSpeedBuffMultiplier()` 是否没有直接硬改 `MaxWalkSpeed`。
7. 是否还残留以下直接写速度的代码：

```cpp
MovementComp->MaxWalkSpeed = SprintSpeed * CurrentBuffMultiplier;
MovementComp->MaxWalkSpeed = WalkSpeed * CurrentBuffMultiplier;
```

特殊情况：

- 体力耗尽时可以强制回到 WalkSpeed，但最好也走同一套目标速度逻辑，避免突然跳变。
- Buff 倍率改变时，不需要立即硬改速度，让 Tick 自动插值到新目标即可。

## 8. 后续建议

短期建议：

- 修正平滑插值中 `CurrentSmoothedSpeed = WalkSpeed * SpeedInterpRate` 的错误。
- 在插值后补上 `MovementComp->MaxWalkSpeed = CurrentSmoothedSpeed`。
- 清理 `SprintComponent.h/.cpp` 中的行尾空格。

中期建议：

- 给 `SprintComponent` 加一个只读调试日志开关，输出当前状态、目标速度、平滑速度、体力和 `bIsActurallySprinting`。
- AnimBP 中将普通移动、跑步、冲刺变量语义拆清楚，避免 `bIsRunning = bIsSprinting` 造成误解。
- HUD 诊断日志在问题解决后可以删除或改成低频 Debug 开关。

