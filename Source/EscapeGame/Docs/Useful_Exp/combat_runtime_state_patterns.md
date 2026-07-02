# 战斗系统协作中的可复用设计经验

生成日期：2026-05-25

本文档记录本轮完善战斗系统时沉淀出的可复用设计模式。重点不是某一段代码本身，而是这些设计思想可以迁移到技能、Buff、交互、AI 行为、背包拖拽等其他系统中。

## 1. Runtime State / Runtime Context 模式

### 场景

当一个组件同时维护多个会一起变化的运行时字段时，不建议把它们全部散落为组件成员。

战斗组件中原本分散的字段包括：

```cpp
ComboCount
bHasSavedComboInput
CurrentActionTag
ActiveTags
CurrentPlayingMontage
InputBufferTimer
```

这些字段本质上都在描述同一件事：当前战斗动作的运行现场。

### 做法

将这些字段收拢到一个轻量结构体中：

```cpp
USTRUCT()
struct FCombatRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	int32 ComboCount = 0;

	UPROPERTY(Transient)
	bool bHasSavedComboInput = false;

	UPROPERTY(Transient)
	FGameplayTag CurrentActionTag;

	UPROPERTY(Transient)
	FGameplayTagContainer ActiveTags;

	UPROPERTY(Transient)
	FGameplayTagContainer CurrentActionTags;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> CurrentPlayingMontage = nullptr;

	UPROPERTY(Transient)
	FTimerHandle InputBufferTimer;
};
```

组件负责流程决策，Runtime State 负责记录现场。

```text
Component = 做决策的人
RuntimeState = 记录现场的本子
DataAsset = 查规则的表
GameplayTag = 状态标签语言
```

### 价值

- 动作开始和结束时，状态提交点更集中。
- 排查卡状态时，可以直接打印一个 Runtime State。
- 后续扩展连击、蓄力、闪避、受击、处决时，不容易把状态写散。
- 适合所有“开始-运行-结束”的玩法流程。

### 可迁移场景

- 技能系统：当前技能 Tag、施法目标、Montage、冷却 Timer、技能阶段。
- Buff 系统：Buff Tag、来源 Actor、持续时间、叠层数、Timer。
- 交互系统：当前交互对象、交互进度、锁定状态、提示文本。
- 背包拖拽：源背包、源槽位、目标槽位、拖拽状态。
- AI 行为：当前行为 Tag、目标 Actor、行为 Montage、是否可打断。

## 2. 读开放，写收口

### 场景

`ActiveTags` 这种状态容器经常会被很多地方读取。如果每个函数都能随意 `AddTag` / `RemoveTag`，状态生命周期会很难追踪。

### 做法

查询可以开放：

```cpp
bool HasCombatTag(FGameplayTag TagToCheck) const
{
	return RuntimeState.ActiveTags.HasTag(TagToCheck);
}
```

修改要收口：

```cpp
RuntimeState.AddCombatTag(Tag);
RuntimeState.RemoveCombatTag(Tag);
RuntimeState.AddActionTag(Tag);
RuntimeState.ClearCurrentActionTags();
```

原则：

```text
读可以开放，写要收口。
```

### 价值

- 后续排查“谁加了这个 Tag / 谁清了这个 Tag”更容易。
- 可以在 Add / Remove 函数中统一加日志、断言和调试统计。
- 避免多个系统直接操作同一个容器导致状态互相污染。

## 3. Active Tags 与 Current Action Tags 分层

### 场景

战斗系统中有两类 Tag：

```text
ActiveTags = 当前战斗组件身上所有运行时 Tag
CurrentActionTags = 当前动作自己添加、动作结束应该回收的 Tag
```

如果动作结束时直接 `ActiveTags.Reset()`，会把无敌、霸体、Debuff、Cooldown 等其他状态也一起删掉。

### 做法

动作开始时记录本次动作产生的 Tag：

```cpp
void AddActionTag(FGameplayTag Tag)
{
	if (Tag.IsValid())
	{
		ActiveTags.AddTag(Tag);
		CurrentActionTags.AddTag(Tag);
	}
}
```

动作结束时只清理本次动作 Tag：

```cpp
void ClearCurrentActionTags()
{
	for (auto It = CurrentActionTags.CreateConstIterator(); It; ++It)
	{
		ActiveTags.RemoveTag(*It);
	}

	CurrentActionTags.Reset();
}
```

### 经验

- `Action.Combat.Light.1`、`Action.Combat.Light.2` 适合放入 `CurrentActionTags`。
- `Action.ChargedAttack.Release` 适合放入 `CurrentActionTags`。
- `Action.State.Attacking` 是整个攻击会话状态，连击切段时要保留，不应在 `Light.1 -> Light.2` 时被清掉。
- 会话级 Tag 可以用 `AddCombatTag()` 添加，在 Montage 彻底结束时单独移除。

## 4. 流程逻辑与数据配置分离

### 场景

战斗组件不应该硬编码每个动作的 Montage、伤害、Trace 半径、连击下一段。

### 做法

使用 `UCharacterAnimData` 作为动作数据库：

```cpp
TMap<FGameplayTag, FCombatActionDefinition> CombatActionMap;
```

运行时用 Tag 查表：

```cpp
const FCombatActionDefinition* ActionDef =
	CharacterAnimData->CombatActionMap.Find(ActionTag);
```

### 价值

- C++ 负责规则和流程，DataAsset 负责动作参数。
- 新增动作时优先改配置，不必频繁改代码。
- GameplayTag 成为动作系统、动画系统、调试日志之间的共同语言。

## 5. 先诊断，后重构

### 场景

出现“UI 显示满体力，但连击提示体力不足”时，不能马上假设是连击逻辑坏了。

### 做法

先保留原有逻辑，只在失败分支加诊断日志：

```cpp
UE_LOG(LogTemp, Warning,
	TEXT("体力不足，无法执行动作 [%s]！SprintComp=%s, CurrentStamina=%.2f, MaxStamina=%.2f"),
	*ActionTag.ToString(),
	*GetNameSafe(SprintComp),
	CurrentStamina,
	MaxStamina);
```

### 价值

- 不改变行为，先定位真实运行时数据。
- 能区分“逻辑确实判断失败”和“Editor 没加载新编译产物”。
- 避免为了一个现象过早重构多个系统。

## 6. 编译产物与运行时代码要确认一致

### 场景

源码中已经加了增强日志，但 PIE 中仍输出旧日志。

### 判断

这通常说明运行时没有加载到新的 C++ 编译结果，而不是新日志逻辑没走到。

### 排查顺序

1. 停止 PIE。
2. 重新编译 C++。
3. 如果使用 Live Coding 后仍异常，关闭 Editor 后从 IDE 或项目重新编译。
4. 重新打开 Editor，再进入 Demo 验证。
5. 用日志文本是否变化确认运行时是否加载到最新代码。

## 7. 平台专用头文件不要放进通用组件头

### 场景

Windows x64 编译时报：

```text
Cannot open type library file: 'UIKit/UIKit.h'
```

根因是通用头文件中误 include 了：

```cpp
#include "IOS/IOSAppDelegate.h"
```

### 经验

- `UIKit` 是 Apple/iOS 平台库，Windows 编译一定找不到。
- 平台专用头必须放在平台宏保护内，或者只在平台专用实现文件中 include。
- 通用组件头文件应尽量只 include 必要的跨平台头。

## 8. UE USTRUCT 字段要给默认值

### 场景

Editor 启动时出现：

```text
FloatProperty FCombatActionDefinition::HitStopDuration is not initialized properly
FloatProperty FCombatActionDefinition::CameraShakeScale is not initialized properly
EnumProperty FItemData::ItemType is not initialized properly
```

### 做法

给 `USTRUCT` 字段提供默认值：

```cpp
float HitStopDuration = 0.f;
float CameraShakeScale = 1.f;
```

枚举字段也应有明确默认项，例如：

```cpp
EItemType ItemType = EItemType::None;
```

如果枚举没有 `None`，建议补一个安全默认值。

### 价值

- 消除启动时 `LogClass Error`。
- 避免 Automation Test 被无关启动错误污染。
- 让新建 DataAsset / DataTable 行时默认状态更可控。

## 9. 小步重构的验收方式

### 场景

将战斗状态收拢到 `FCombatRuntimeState` 后，不应一次性改闪避、受击、体力消耗等更多规则。

### 做法

只验证原有流程不退化：

- 快速点击攻击，第一段轻击正常播放。
- 连续点击攻击，输入缓存能触发下一段连击。
- 长按攻击，能进入蓄力。
- 松开长按，能释放重击。
- Montage 结束后，能重新起手。
- `DoAttackTrace()` 仍能根据当前动作 Tag 找到 `FCombatActionDefinition`。

### 价值

- 每次只验证一个设计变化。
- 出问题时能快速定位是本次改动引入，还是历史问题。
- 适合战斗系统这种动画、输入、状态、数据资产互相耦合的模块。
