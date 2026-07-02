# C++ Bug 审查问题清单

生成日期：2026-06-06

审查范围：`Source/EscapeGame` 当前主源码中的 `.cpp` 文件，共 37 个。排除范围沿用项目记忆约定：`Variant_Combat`、`Variant_Platforming`、`Variant_SideScrolling`、`.cursor`。

参考上下文：`Docs/AI_Memory/architecture.md`

说明：本轮只把当前 `.cpp` 中仍可复现或由 `.cpp` 直接触发的问题列为 Bug。历史清单中已经修复的问题不重复计入；`ItemDefinition.h::MaxStackCount` 缺少编辑器 `ClampMin` 仍是配置约束问题，但不计入本轮 `.cpp` 审查数量。

## 总览

本次审查发现当前仍需处理的问题数：**10 个**

严重性统计：

```text
严重：1 个
警告：8 个
建议：1 个
```

建议优先修复：

```text
1. 问题 1：门已打开后仍可重复交互并继续消耗钥匙。
2. 问题 2：UEscapeCombatComponent 未在 EndPlay 清理 Montage 委托和输入 Timer。
3. 问题 5：速度 Buff 参数未校验，可能造成永久 Buff、负速度或无法移动。
```

历史问题状态：

```text
旧问题 1-10：本轮未在 .cpp 中发现回归。
旧问题 11：CombatComponent 未清理委托和 Timer，仍存在，已作为本轮问题 2 记录。
旧问题 12：攻击输入路径同步加载 Montage，仍存在，已作为本轮问题 10 记录。
```

## 1. 门已打开后仍可重复交互并继续消耗钥匙

| 项目 | 内容 |
|------|------|
| 文件 | `Source/EscapeGame/InterectComponent.cpp:107`，`Source/EscapeGame/WorldInteractObject/InteractDoor.cpp:41` |
| 严重性 | 严重 |
| 类型 | 逻辑错误 |
| 当前状态 | 未修复 |

代码片段：

```cpp
else if (HitActor->Implements<UInteractableInterface>())
{
    APawn* PawnOwner = Cast<APawn>(GetOwner());
    if(PawnOwner)
    {
        IInteractableInterface::Execute_Interact(HitActor, PawnOwner);
    }
}
```

```cpp
if (!InstigatorPawn || !bIsInteractable)return false;
...
bIsOpen = true;
OnDoorOpen();
if (bConsumeKey)
{
    InventoryComp->RemoveItem(RequireKeyID, 1);
}
```

问题描述：`CanInteract_Implementation` 虽然返回 `bIsInteractable && !bIsOpen`，但交互组件没有调用 `CanInteract`，门自身 `Interact` 也没有检查 `bIsOpen`，所以已打开的门可被再次触发并继续消耗钥匙。

修复方案：

```text
[方案A]：在 UInterectComponent 调用 Interact 前先调用 CanInteract。
优点：所有交互对象统一受 CanInteract 保护；缺点：依赖每个交互对象正确实现 CanInteract。

[方案B]：在 AInteractDoor::Interact_Implementation 开头增加 bIsOpen 保护。
优点：直接阻止门重复开门和重复扣钥匙；缺点：只修门，其他交互对象仍可能绕过 CanInteract。

[方案C]：同时做 A 和 B，交互入口统一检查，门自身也做防御。
优点：双层保护，最稳；缺点：改动涉及两个文件。

[我的建议]：选择方案C，因为这是消耗钥匙类 Bug，入口和对象自身都应该防御。
```

## 2. CombatComponent 未在 EndPlay 清理 Montage 委托和输入 Timer

| 项目 | 内容 |
|------|------|
| 文件 | `Source/EscapeGame/EscapeCombatComponent.cpp:40`，`Source/EscapeGame/EscapeCombatComponent.cpp:437` |
| 严重性 | 警告 |
| 类型 | 生命周期 |
| 当前状态 | 未修复 |

代码片段：

```cpp
CachedAnimInstance->OnMontageEnded.AddDynamic(this, &UEscapeCombatComponent::HandleAttackMontageEnded);
...
GetWorld()->GetTimerManager().SetTimer(
    RuntimeState.InputBufferTimer,
    this,
    &UEscapeCombatComponent::BeginOrUpdateChargedAttack,
    CombatWindow,
    false
);
```

问题描述：战斗组件绑定了 Montage 结束委托并设置攻击输入 Timer，但没有 `EndPlay` 解绑和清理，关卡切换、角色销毁或组件销毁时存在残留回调风险。

修复方案：

```text
[方案A]：给 UEscapeCombatComponent 增加 EndPlay，RemoveDynamic 并 ClearTimer。
优点：最符合 UE 组件生命周期习惯；缺点：需要同步修改 .h 声明和 .cpp 实现。

[方案B]：在 EndPlay 中调用 ClearAllTimersForObject(this)，委托仍单独 RemoveDynamic。
优点：可清理未来新增 Timer；缺点：范围更宽，需要确认不会误清理本组件其他有意保留的 Timer。

[我的建议]：选择方案A，清理目标明确，改动范围小。
```

## 3. 重击释放在验证失败前修改 Tag，可能残留释放状态

| 项目 | 内容 |
|------|------|
| 文件 | `Source/EscapeGame/EscapeCombatComponent.cpp:184` |
| 严重性 | 警告 |
| 类型 | 逻辑错误 / 生命周期 |
| 当前状态 | 未修复 |

代码片段：

```cpp
RuntimeState.RemoveCombatTag(EscapeGameplayTags::Action_Combat_Heavy_Charge);
RuntimeState.AddActionTag(EscapeGameplayTags::Action_ChargedAttack_Release);

ACharacter* OwnerChar = GetOwnerCharacter();
if (!OwnerChar || !CharacterAnimData || !RuntimeState.CurrentActionTag.IsValid()) return;
```

问题描述：`ReleaseChargedAttack` 先移除蓄力 Tag、添加释放 Tag，再验证角色、动画实例和当前动作；如果后续校验失败，`Action_ChargedAttack_Release` 可能残留并阻止后续动作。

修复方案：

```text
[方案A]：先完成 OwnerChar、AnimInstance、ActionDef、Montage 校验，再切换 Tag。
优点：状态只在确认可释放时变化；缺点：需要调整函数内执行顺序。

[方案B]：保留当前顺序，但任何失败分支都调用 RuntimeState.ResetAction()。
优点：修复直接；缺点：可能把当前动作清得过早，影响蓄力 Montage 的收尾。

[我的建议]：选择方案A，因为状态提交应发生在验证成功之后。
```

## 4. HUD 快捷栏初始化条件检查了生命组件而不是背包组件

| 项目 | 内容 |
|------|------|
| 文件 | `Source/EscapeGame/GameHUDWidget.cpp:53` |
| 严重性 | 警告 |
| 类型 | 逻辑错误 / UI |
| 当前状态 | 未修复 |

代码片段：

```cpp
if (HotbarWidget && AttributeCompRef.IsValid())
{
    HotbarWidget->InitializeHotbar(NewInventoryComp);
}
```

问题描述：快捷栏是否初始化取决于 `AttributeCompRef`，而不是 `NewInventoryComp`；如果生命组件有效但背包组件为空，会把空背包传入快捷栏，如果生命组件为空但背包有效，快捷栏也不会初始化。

修复方案：

```text
[方案A]：把条件改为 HotbarWidget && NewInventoryComp。
优点：直接匹配快捷栏真实依赖；缺点：仍不保存背包弱引用，后续重复初始化不易管理。

[方案B]：新增 InventoryCompRef，并用 HotbarWidget && InventoryCompRef.IsValid() 初始化。
优点：HUD 内部依赖更清晰；缺点：比当前需求多一个成员变量。

[我的建议]：选择方案A，当前问题只需要修正初始化条件。
```

## 5. 速度 Buff 参数未校验，可能造成永久 Buff 或异常移动速度

| 项目 | 内容 |
|------|------|
| 文件 | `Source/EscapeGame/SprintComponent.cpp:48` |
| 严重性 | 警告 |
| 类型 | 逻辑错误 |
| 当前状态 | 未修复 |

代码片段：

```cpp
void USprintComponent::StartSpeedBuff(float Duration, float Multiplier)
{
    SetSpeedBuffMultiplier(Multiplier);

    FTimerDelegate TimerDel;
    TimerDel.BindUObject(this, &USprintComponent::SetSpeedBuffMultiplier, 1.0f);

    GetWorld()->GetTimerManager().SetTimer(TimerHandle_Buff, TimerDel, Duration, false);
}
```

问题描述：`Duration <= 0` 时 Timer 可能不会触发恢复，`Multiplier <= 0` 时可能把 `MaxWalkSpeed` 推到 0 或负值，导致永久 Buff、无法移动或异常速度。

修复方案：

```text
[方案A]：运行时校验 Duration > 0 且 Multiplier > 0，不合法则返回 false 或拒绝应用。
优点：防止坏数据进入运行时；缺点：调用方需要知道失败结果。

[方案B]：运行时 Clamp 参数，并在 DataAsset 属性上增加 ClampMin / UIMin。
优点：编辑器和运行时双重保护；缺点：涉及 .h 元数据修改。

[我的建议]：选择方案B，因为 Buff 参数来自资产配置，应该同时限制编辑器输入和运行时入口。
```

## 6. 眩晕时长无效时可能永久停在 Stunned

| 项目 | 内容 |
|------|------|
| 文件 | `Source/EscapeGame/statemachine/StateMachineComponent.cpp:83` |
| 严重性 | 警告 |
| 类型 | 逻辑错误 / 生命周期 |
| 当前状态 | 未修复 |

代码片段：

```cpp
void UStateMachineComponent::ApplyStun(float Duration)
{
    if (CurrentState == ECharacterState::Dead) return;

    SetState(ECharacterState::Stunned);

    if (UWorld* World = GetWorld()) 
    {
        World->GetTimerManager().SetTimer(TimerHandle_Stun, this, &UStateMachineComponent::OnStunFinished, Duration, false);
    }
}
```

问题描述：`Duration <= 0` 时角色已经切到 `Stunned`，但恢复 Timer 可能不会启动，角色会一直停在眩晕状态；此外该 Timer 也没有在 `EndPlay` 中清理。

修复方案：

```text
[方案A]：Duration <= 0 时直接拒绝眩晕，不调用 SetState(Stunned)。
优点：避免非法输入造成永久状态；缺点：无法表达“瞬时眩晕”。

[方案B]：Duration <= 0 时执行一次短暂最小值，例如 0.01 秒。
优点：兼容外部误传 0 的情况；缺点：语义不如直接拒绝清晰。

[方案C]：在 A 或 B 基础上增加 EndPlay 清理 Timer。
优点：同时解决非法参数和生命周期问题；缺点：需要改 .h 和 .cpp。

[我的建议]：选择方案C，并采用方案A的非法参数拒绝策略。
```

## 7. 动画线程安全更新中访问 World，并存在可配置除零风险

| 项目 | 内容 |
|------|------|
| 文件 | `Source/EscapeGame/CharacterAnimInstance.cpp:78`，`Source/EscapeGame/CharacterAnimInstance.cpp:101` |
| 严重性 | 警告 |
| 类型 | 生命周期 / 逻辑错误 |
| 当前状态 | 未修复 |

代码片段：

```cpp
const float SpeedAlpha=FMath::Clamp((Speed-HairRunBounceMinSpeed)/(HairRunBounceMaxSpeed-HairRunBounceMinSpeed), 0.0f, 1.0f);
const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
const float BounceZ = FMath::Sin(TimeSeconds*HairRunBounceFrequency*TWO_PI)*HairRunBounceStrength*SpeedAlpha;
...
LocomotionPlayRate = FMath::Clamp(GroundSpeed / AuthoredRunSpeed, 0.5f, 2.0f);
```

问题描述：`NativeThreadSafeUpdateAnimation` 中访问 `GetWorld()` 不符合线程安全更新的约束；同时 `HairRunBounceMaxSpeed == HairRunBounceMinSpeed` 或 `AuthoredRunSpeed <= 0` 会产生除零或 NaN。

修复方案：

```text
[方案A]：在 NativeUpdateAnimation 中缓存 TimeSeconds，并在 NativeThreadSafeUpdateAnimation 中只读取缓存值；所有除法前保护分母。
优点：保留线程安全设计；缺点：需要增加一个缓存字段。

[方案B]：把相关计算移回 NativeUpdateAnimation。
优点：实现简单；缺点：减少线程安全更新带来的并行动画收益。

[我的建议]：选择方案A，既修安全性又保留性能设计。
```

## 8. 最大耐力降低后 CurrentStamina 未同步 Clamp

| 项目 | 内容 |
|------|------|
| 文件 | `Source/EscapeGame/SprintComponent.cpp:208` |
| 严重性 | 警告 |
| 类型 | 逻辑错误 / UI |
| 当前状态 | 未修复 |

代码片段：

```cpp
void USprintComponent::ApplyMaxChange(float Delta)
{
    MaxStamina += Delta;
    if (MaxStamina < 0.0f)MaxStamina = 0.0f;
    if (OnStaminaChanged.IsBound())
    {
        OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
    }
}
```

问题描述：如果 `Delta` 为负或外部降低最大耐力，`CurrentStamina` 可能大于新的 `MaxStamina`，HUD 百分比会超过 100%，后续逻辑也会短暂使用不一致的耐力数据。

修复方案：

```text
[方案A]：修改 MaxStamina 后立刻 CurrentStamina = Clamp(CurrentStamina, 0, MaxStamina)。
优点：数据始终一致；缺点：降低上限会立即扣掉超出的当前耐力。

[方案B]：禁止 ApplyMaxChange 接受负数，只把它作为“增强耐力”入口。
优点：符合当前 UItemAction_EnhanceStamina 的语义；缺点：未来需要降低上限时还要新增接口。

[我的建议]：选择方案A，因为组件 API 已经是 Delta 语义，应支持正负变化并保持数据一致。
```

## 9. 武器导入失败会留下部分已导入资产

| 项目 | 内容 |
|------|------|
| 文件 | `Source/EscapeGame/WeaponImportToolLibrary.cpp:431` |
| 严重性 | 警告 |
| 类型 | Editor 工具 / 资产维护 |
| 当前状态 | 未修复 |

代码片段：

```cpp
UObject* ImportedMeshObject = WeaponImportTool::ImportSingleAsset(ObjFile, DestinationPath, Result, UStaticMesh::StaticClass());
UStaticMesh* ImportedMesh = Cast<UStaticMesh>(ImportedMeshObject);
if (!ImportedMesh)
{
    Result.AddMessage(TEXT("OBJ did not import as Static Mesh."));
    return Result;
}

UObject* ImportedDiffuseObject = WeaponImportTool::ImportSingleAsset(DiffuseTextureFile, DestinationPath, Result, UTexture2D::StaticClass());
UTexture2D* DiffuseTexture = Cast<UTexture2D>(ImportedDiffuseObject);
if (!DiffuseTexture)
{
    Result.AddMessage(TEXT("Diffuse texture did not import as Texture2D."));
    return Result;
}
```

问题描述：导入流程是分步执行的；如果 Mesh 已导入但 Diffuse、Material 或 WeaponDefinition 后续失败，函数直接返回，Content Browser 里可能留下未保存或半成品资产。

修复方案：

```text
[方案A]：记录本次创建/导入的资产路径，失败时删除或回滚这些资产。
优点：导入失败后 Content 不会污染；缺点：删除资产需要谨慎处理引用和包保存。

[方案B]：改成先做更完整的预检查，尽量减少中途失败概率。
优点：实现简单；缺点：无法覆盖导入器内部失败或资产创建失败。

[我的建议]：选择方案A，编辑器工具应保证失败路径可清理。
```

## 10. 攻击输入路径同步加载 Montage，首次攻击可能卡顿

| 项目 | 内容 |
|------|------|
| 文件 | `Source/EscapeGame/EscapeCombatComponent.cpp:120` |
| 严重性 | 建议 |
| 类型 | 性能 |
| 当前状态 | 未修复 |

代码片段：

```cpp
// 4. 加载资源 (同步加载)
UAnimMontage* MontageToPlay = ActionDef->Montage.LoadSynchronous();
```

问题描述：攻击输入路径中同步加载软引用 Montage，首次攻击或切换动作时可能产生游戏线程卡顿。

修复方案：

```text
[方案A]：在 BeginPlay 或装备/动画数据初始化阶段预加载 CombatActionMap 中的 Montage。
优点：短期实现简单，输入时不再同步加载；缺点：启动或角色初始化时会增加加载成本。

[方案B]：使用 StreamableManager 异步加载并缓存 Montage。
优点：长期性能最好；缺点：需要处理动作请求等待加载、取消和失败状态。

[我的建议]：短期选择方案A，后续动作系统稳定后再升级为方案B。
```

## 本轮补充观察

```text
1. 当前没有发现显式 Server / Client / NetMulticast RPC 或 Replicated 属性，本轮不把多人同步缺失计为 Bug。
2. HUD、交互、背包和 Tooltip 中仍保留大量诊断日志；不影响逻辑，但后续可统一加 Debug 开关。
3. WindSimulationComponent.cpp 当前在工作区已有未提交改动，本轮只读取并记录风险，没有回滚或覆盖。
4. rg 在本地环境执行被拒绝，本轮使用 PowerShell Get-ChildItem / Select-String 完成扫描。
```
