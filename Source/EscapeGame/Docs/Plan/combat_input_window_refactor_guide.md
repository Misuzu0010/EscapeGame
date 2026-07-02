# 战斗输入缓冲与动画窗口重构指导

本文档用于指导 `UEscapeCombatComponent` 中轻击连段、长按重击、轻击转重击窗口的重构。

目标是把当前逐渐膨胀的输入逻辑，从多个专用 bool 和专用 Notify 函数，整理成一套统一模型：

```text
输入意图 BufferedInput
+
动画窗口 ActiveWindows
+
统一消费函数 TryConsumeBufferedInput()
```

## 1. 当前问题

当前战斗输入逻辑中，轻击、连击、蓄力重击、轻击转重击共用同一个 Attack 输入，但内部概念没有完全拆开。

典型问题：

```text
bHasSavedComboInput 只适合表示轻击连段输入，但未来容易被重击派生复用或误用。

bComboWindowActive 只服务 ComboWindow，后续如果增加 HeavyCancelWindow、DodgeCancelWindow，会继续增加 bool。

BeginComboWindow / TickComboWindow / EndComboWindow 是专用窗口函数，后续每种窗口都复制一套会变冗余。

CanStartCombatAction 同时处理通用动作合法性、连击链路、蓄力状态锁和未来重击派生特例，容易变成条件地狱。

轻击转重击目前缺少明确的 Montage 窗口控制，容易变成 Timer 到点后硬切动作。
```

## 2. 重构目标

重构后的输入层应该满足：

```text
玩家输入只记录意图，不直接决定能否切动作。

AnimNotifyState 只打开或关闭窗口，不直接承担复杂战斗判断。

CombatComponent 根据当前动作、输入意图、活动窗口统一决定是否消费输入。

轻击连段和轻击转重击都通过 Montage 上的窗口调手感。

以后扩展闪避取消、技能取消时，不再新增一整套 BeginXWindow / TickXWindow / EndXWindow。
```

最终手感目标：

```text
空闲短按 Attack -> 第一段轻击

空闲长按 Attack -> Heavy.Charge

轻击中短按 Attack -> 缓存 Light 输入，进入 Combo 窗口后接下一段轻击

轻击中长按 Attack -> 缓存 Heavy 输入，进入 HeavyCancel 窗口后切 Heavy.Charge

进入 Heavy.Charge 后松开 Attack -> ReleaseChargedAttack
```

## 3. 核心数据结构

建议新增两个枚举。

```cpp
UENUM(BlueprintType)
enum class ECombatBufferedInput : uint8
{
    None,
    Light,
    Heavy
};
```

```cpp
UENUM(BlueprintType)
enum class ECombatWindowType : uint8
{
    Combo,
    HeavyCancel
};
```

建议把运行时字段收进 `FCombatRuntimeState`，避免状态散落在组件外。

```cpp
UPROPERTY(Transient)
ECombatBufferedInput BufferedInput = ECombatBufferedInput::None;

UPROPERTY(Transient)
TSet<ECombatWindowType> ActiveWindows;
```

如果暂时不想大改 `FCombatRuntimeState`，第一阶段也可以先放在 `UEscapeCombatComponent` private 字段中。长期建议收进 RuntimeState。

## 4. 统一接口设计

新增输入缓冲接口：

```cpp
void BufferCombatInput(ECombatBufferedInput Input);
void ClearBufferedInput();
```

新增动画窗口接口：

```cpp
UFUNCTION(BlueprintCallable, Category = "Combat|Window")
void BeginCombatWindow(ECombatWindowType WindowType);

UFUNCTION(BlueprintCallable, Category = "Combat|Window")
void EndCombatWindow(ECombatWindowType WindowType);
```

新增统一消费函数：

```cpp
void TryConsumeBufferedInput();
```

可选查询函数：

```cpp
bool HasCombatWindow(ECombatWindowType WindowType) const;
```

## 5. 输入层重构

### 5.1 Attack Started

当前 `Input_AttackStarted()` 负责开长按 Timer。保留这个思路，但 Timer 回调不要再直接调用 `BeginOrUpdateChargedAttack()`。

建议改为：

```cpp
void UEscapeCombatComponent::Input_AttackStarted()
{
    bAttackButtonHeld = true;

    GetWorld()->GetTimerManager().SetTimer(
        RuntimeState.InputBufferTimer,
        this,
        &UEscapeCombatComponent::HandleAttackHoldThresholdReached,
        CombatWindow,
        false
    );
}
```

新增长按阈值回调：

```cpp
void UEscapeCombatComponent::HandleAttackHoldThresholdReached()
{
    BufferCombatInput(ECombatBufferedInput::Heavy);
    TryConsumeBufferedInput();
}
```

### 5.2 Attack Completed

松手时分两种：

```text
Timer 仍活跃：说明是短按，缓存 Light。

已经处于 Heavy.Charge：说明是蓄力结束，释放重击。
```

建议逻辑：

```cpp
void UEscapeCombatComponent::Input_AttackCompleted()
{
    bAttackButtonHeld = false;

    if (GetWorld()->GetTimerManager().IsTimerActive(RuntimeState.InputBufferTimer))
    {
        GetWorld()->GetTimerManager().ClearTimer(RuntimeState.InputBufferTimer);
        BufferCombatInput(ECombatBufferedInput::Light);
        TryConsumeBufferedInput();
        return;
    }

    if (RuntimeState.ActiveTags.HasTag(EscapeGameplayTags::Action_Combat_Heavy_Charge))
    {
        ReleaseChargedAttack();
    }
}
```

注意：如果长按阈值已经触发，`BufferedInput` 应该是 Heavy。此时松手不要再补 Light 输入。

## 6. 统一消费规则

`TryConsumeBufferedInput()` 是本次重构的核心。

推荐逻辑：

```cpp
void UEscapeCombatComponent::TryConsumeBufferedInput()
{
    if (RuntimeState.BufferedInput == ECombatBufferedInput::None)
    {
        return;
    }

    const bool bIsAttacking =
        RuntimeState.ActiveTags.HasTag(EscapeGameplayTags::Action_State_Attacking);

    if (!bIsAttacking)
    {
        if (RuntimeState.BufferedInput == ECombatBufferedInput::Light)
        {
            RuntimeState.BufferedInput = ECombatBufferedInput::None;
            RequestLightAttack();
            return;
        }

        if (RuntimeState.BufferedInput == ECombatBufferedInput::Heavy)
        {
            RuntimeState.BufferedInput = ECombatBufferedInput::None;
            BeginOrUpdateChargedAttack();
            return;
        }
    }

    if (RuntimeState.BufferedInput == ECombatBufferedInput::Light &&
        HasCombatWindow(ECombatWindowType::Combo))
    {
        RuntimeState.BufferedInput = ECombatBufferedInput::None;
        CommitNextLightCombo();
        return;
    }

    if (RuntimeState.BufferedInput == ECombatBufferedInput::Heavy &&
        HasCombatWindow(ECombatWindowType::HeavyCancel))
    {
        RuntimeState.BufferedInput = ECombatBufferedInput::None;
        BeginOrUpdateChargedAttack();
        return;
    }
}
```

建议逐步把 `CheckCombo()` 的核心逻辑迁移到 `CommitNextLightCombo()`。

## 7. 轻击连段提交函数

把当前 `CheckCombo()` 中“查 NextComboTag 并播放下一段”的逻辑抽出来。

建议函数：

```cpp
bool UEscapeCombatComponent::CommitNextLightCombo()
{
    if (!RuntimeState.ActiveTags.HasTag(EscapeGameplayTags::Action_State_Attacking))
    {
        return false;
    }

    if (!CharacterAnimData || !RuntimeState.CurrentActionTag.IsValid())
    {
        return false;
    }

    const FCombatActionDefinition* CurrentActionDef =
        CharacterAnimData->CombatActionMap.Find(RuntimeState.CurrentActionTag);

    if (!CurrentActionDef || !CurrentActionDef->NextComboTag.IsValid())
    {
        RuntimeState.ComboCount = 0;
        BroadcastComboChange(RuntimeState.ComboCount);
        return false;
    }

    if (!CharacterAnimData->CombatActionMap.Contains(CurrentActionDef->NextComboTag))
    {
        return false;
    }

    if (!TryPlayActionByTagInternal(CurrentActionDef->NextComboTag))
    {
        return false;
    }

    RuntimeState.ComboCount++;
    BroadcastComboChange(RuntimeState.ComboCount);
    return true;
}
```

旧 `CheckCombo()` 可以先保留，并改成：

```cpp
void UEscapeCombatComponent::CheckCombo()
{
    CommitNextLightCombo();
}
```

这样旧蓝图 Notify 不会立刻失效。

## 8. 通用动画窗口

### 8.1 C++ 接口逻辑

```cpp
void UEscapeCombatComponent::BeginCombatWindow(ECombatWindowType WindowType)
{
    RuntimeState.ActiveWindows.Add(WindowType);
    TryConsumeBufferedInput();
}
```

```cpp
void UEscapeCombatComponent::EndCombatWindow(ECombatWindowType WindowType)
{
    RuntimeState.ActiveWindows.Remove(WindowType);
}
```

```cpp
bool UEscapeCombatComponent::HasCombatWindow(ECombatWindowType WindowType) const
{
    return RuntimeState.ActiveWindows.Contains(WindowType);
}
```

大多数情况下不再需要 Notify Tick。窗口打开时消费一次，输入发生时消费一次，就足够覆盖：

```text
先输入，后进窗口
先进窗口，后输入
```

### 8.2 通用 AnimNotifyState

建议创建一个统一 NotifyState：

```text
ANS_CombatWindow
```

暴露一个变量：

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
ECombatWindowType WindowType = ECombatWindowType::Combo;
```

蓝图实现：

```text
Received Notify Begin
  Mesh Comp
  -> Get Owner
  -> Get Component By Class: EscapeCombatComponent
  -> BeginCombatWindow(WindowType)

Received Notify End
  Mesh Comp
  -> Get Owner
  -> Get Component By Class: EscapeCombatComponent
  -> EndCombatWindow(WindowType)
```

不建议在 `Received Notify Tick` 中持续调用消费逻辑，除非后续确实需要持续检测某种条件。

## 9. Montage 配置方式

### 9.1 轻击连段窗口

在轻击 Montage 上放置：

```text
ANS_CombatWindow
WindowType = Combo
```

建议位置：

```text
起点：攻击命中帧之后一点
终点：收招前 20% 左右
```

### 9.2 轻击转重击窗口

在允许派生重击的轻击 Montage 上放置：

```text
ANS_CombatWindow
WindowType = HeavyCancel
```

建议位置：

```text
起点：轻击动作已经明确挥出之后
终点：普通连击窗口附近或略早
```

调手感方式：

```text
HeavyCancel 窗口提前：重击派生更快，但可能显得动作被截断。

HeavyCancel 窗口延后：动作更完整，但响应感变慢。

HeavyCancel 窗口变长：容错更高。

HeavyCancel 窗口变短：更硬核，输入要求更准。
```

## 10. CanStartCombatAction 整理

当前 `CanStartCombatAction()` 在攻击中只允许 `NextComboTag`：

```cpp
if (CurrentActionDef->NextComboTag != ActionTag)
{
    return false;
}
```

重构后，攻击中需要允许一种特殊情况：

```text
请求 Action.Combat.Heavy.Charge
并且 HeavyCancel 窗口处于激活状态
```

建议把攻击中校验抽出为单独函数：

```cpp
bool UEscapeCombatComponent::CanStartActionDuringAttack(
    FGameplayTag ActionTag,
    FString* OutFailReason) const;
```

核心规则：

```cpp
const bool bIsHeavyCancel =
    ActionTag == EscapeGameplayTags::Action_Combat_Heavy_Charge &&
    HasCombatWindow(ECombatWindowType::HeavyCancel);

if (bIsHeavyCancel)
{
    return true;
}

if (CurrentActionDef->NextComboTag == ActionTag)
{
    return true;
}

return false;
```

这样重击派生不会污染普通轻击连段规则。

## 11. 旧接口兼容策略

为了降低风险，不建议一次删除旧函数。

第一阶段可以让旧函数转调新接口：

```cpp
void UEscapeCombatComponent::BeginComboWindow()
{
    BeginCombatWindow(ECombatWindowType::Combo);
}
```

```cpp
void UEscapeCombatComponent::EndComboWindow()
{
    EndCombatWindow(ECombatWindowType::Combo);
}
```

```cpp
void UEscapeCombatComponent::TickComboWindow()
{
    TryConsumeBufferedInput();
}
```

旧 `bHasSavedComboInput` 可以先保留，但输入源逐步迁移到 `BufferedInput` 后再删除。

## 12. 推荐实施顺序

```text
1. 新增 ECombatBufferedInput 和 ECombatWindowType。

2. 在 RuntimeState 或 CombatComponent 中新增 BufferedInput 和 ActiveWindows。

3. 新增 BufferCombatInput、ClearBufferedInput、BeginCombatWindow、EndCombatWindow、HasCombatWindow、TryConsumeBufferedInput。

4. 让旧 BeginComboWindow / TickComboWindow / EndComboWindow 转调新接口，保证当前 ANS_CheckCombo 可继续工作。

5. 把 Input_AttackCompleted 的短按分支改为 BufferCombatInput(Light) + TryConsumeBufferedInput。

6. 把长按 Timer 回调改为 HandleAttackHoldThresholdReached，并在其中 BufferCombatInput(Heavy) + TryConsumeBufferedInput。

7. 把 CheckCombo 的核心逻辑迁移到 CommitNextLightCombo。

8. 整理 CanStartCombatAction，让攻击中允许 HeavyCancel 窗口内启动 Heavy.Charge。

9. 创建 ANS_CombatWindow，用 WindowType 控制 Combo 或 HeavyCancel。

10. 在 Montage 中用 ANS_CombatWindow 替换 ANS_CheckCombo，并新增 HeavyCancel 窗口。

11. 测试空闲轻击、轻击连段、空闲长按重击、轻击中长按转重击。

12. 确认稳定后删除旧 bool、旧专用窗口函数和旧蓝图 Notify。
```

## 13. 常用 UE API

Timer：

```cpp
GetWorld()->GetTimerManager().SetTimer(...)
GetWorld()->GetTimerManager().ClearTimer(...)
GetWorld()->GetTimerManager().IsTimerActive(...)
```

GameplayTag：

```cpp
RuntimeState.ActiveTags.HasTag(...)
RuntimeState.ActiveTags.HasTagExact(...)
RuntimeState.AddCombatTag(...)
RuntimeState.RemoveCombatTag(...)
```

Montage：

```cpp
OwnerChar->PlayAnimMontage(...)
AnimInstance->Montage_IsPlaying(...)
AnimInstance->Montage_JumpToSection(...)
```

组件查找：

```cpp
MeshComp->GetOwner()
Actor->FindComponentByClass<UEscapeCombatComponent>()
Actor->GetComponentByClass(...)
```

TSet：

```cpp
ActiveWindows.Add(WindowType)
ActiveWindows.Remove(WindowType)
ActiveWindows.Contains(WindowType)
ActiveWindows.Reset()
```

## 14. 测试清单

基础输入：

```text
空闲短按 Attack 能打出 Light.1。

空闲长按 Attack 能进入 Heavy.Charge。

Heavy.Charge 中松开 Attack 能 ReleaseChargedAttack。
```

轻击连段：

```text
Light.1 播放中，在 Combo 窗口前按攻击，进入窗口时能接 Light.2。

Light.1 播放中，在 Combo 窗口内按攻击，能立即或很快接 Light.2。

Combo 窗口外按攻击，不应错误接下一段。
```

轻击转重击：

```text
Light.1 播放中长按 Attack，在 HeavyCancel 窗口前达到长按阈值，进入窗口时能切 Heavy.Charge。

Light.1 播放中长按 Attack，在 HeavyCancel 窗口内达到长按阈值，能切 Heavy.Charge。

HeavyCancel 窗口外长按，不应乱切 Heavy.Charge。
```

边界状态：

```text
Dead / Stunned 状态不能启动新动作。

Heavy.Charge 中不能启动轻击。

ChargedAttack.Release 中不能启动新动作。

Montage 结束后 ActiveWindows 和 BufferedInput 应被清理。
```

## 15. 清理建议

当新模型稳定后，可以逐步删除：

```text
bHasSavedComboInput
bComboWindowActive
BeginComboWindow / TickComboWindow / EndComboWindow 专用实现
ANS_CheckCombo
所有依赖旧 CheckCombo 单帧触发的 Montage Notify
```

保留兼容函数的时间不要太长，否则新旧逻辑会同时存在，反而更难排查问题。
