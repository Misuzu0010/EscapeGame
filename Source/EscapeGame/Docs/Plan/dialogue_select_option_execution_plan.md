# Dialogue SelectOption Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

> 2026-06-29 更新：本计划中的 `SelectOption` 最小跳转闭环已完成并通过 Automation Test。后续已推进并完成 `GetAvailableOptions` 最小条件筛选（`GlobalFlagIs` / `DialogueNodeSeen`）。下一建议切片为 `ApplyEffect` 最小效果执行（`CloseDialogue` / `SetGlobalFlag`）。

**Goal:** 完成 `UDialogueQuestSubsystem::SelectOption` 的最小跳转闭环，让当前对话节点可以通过玩家选项跳到下一个节点、记录已选选项，并在需要时关闭对话。

**Architecture:** 本切片只修改 Dialogue 运行时主控，不接 UI、不接任务、不接奖励、不接存档。`UDialogueQuestSubsystem` 继续作为会话状态中心，`FConversationSession` 记录当前会话，`FDialogueRuntimeState` 记录长期对话进度。

**Tech Stack:** Unreal Engine 5 C++、Automation Test、`UGameInstanceSubsystem`、`UDataAsset`、`GameplayTags`、`TMap` / `TSet`。

---

## 1. 本轮任务边界

只做：

- 从当前节点里找到 `OptionID` 对应的 `FDialogueOption`。
- 记录 `ActiveSession.LastSelectedOptionID`。
- 记录 `DialogueRuntimeMap[DialogueID].VisitedOptionIDs`。
- 如果选项有 `NextNodeID`，切换 `ActiveSession.CurrentNodeID`。
- 切换节点后记录新的 `SeenNodeIDs` 和 `CurrentNodeID`。
- 如果 `bCloseDialogueAfterSelected=true`，调用 `EndConversation(EInterruptReason::NormalEnd)`。

暂时不做：

- `EvaluateCondition` 条件筛选。
- `ApplyEffect` 效果执行。
- `StartQuest` / `SetGlobalFlag` / `GiveItem`。
- UI Widget。
- SaveGame 映射。
- Boss / Encounter 接入。

---

## 2. 涉及文件

### 需要阅读

- `D:/unreal project/EscapeGame/Source/EscapeGame/Dialogue/DialogueQuestSubsystem.h`
- `D:/unreal project/EscapeGame/Source/EscapeGame/Dialogue/DialogueQuestSubsystem.cpp`
- `D:/unreal project/EscapeGame/Source/EscapeGame/Dialogue/EscapeDialogueTypes.h`
- `D:/unreal project/EscapeGame/Source/EscapeGame/Tests/WeaponImportToolLibraryTest.cpp`

### 需要创建

- `D:/unreal project/EscapeGame/Source/EscapeGame/Tests/DialogueQuestSubsystemTest.cpp`

### 需要修改

- `D:/unreal project/EscapeGame/Source/EscapeGame/Dialogue/DialogueQuestSubsystem.cpp`

### 不需要修改

- `DialogueQuestSubsystem.h`
- `EscapeDialogueTypes.h`
- `DialogueDefinition.h`
- `DialogueParticipantComponent.h`
- `DialogueNPC.h/.cpp`
- `EscapeDialogueSaveGame.h/.cpp`

如果实现过程中发现必须修改这些文件，先停下来说明原因，不要顺手改。

---

## 3. 当前你要先理解的概念

### 3.1 当前会话状态

`ActiveSession` 表示“现在正在进行的一次对话”。

这类数据只在本次对话打开期间有效：

- `SessionId`
- `NPC_ID`
- `DialogueID`
- `CurrentNodeID`
- `LastSelectedOptionID`
- `bIsActive`
- `bLockPlayer`
- `bCanSkip`

### 3.2 长期对话进度

`DialogueRuntimeMap` 表示“这段对话在当前游戏进程 / 当前存档里的长期进度”。

它应该记录：

- 这段对话是否打开过。
- 最近停在哪个节点。
- 看过哪些节点。
- 点过哪些选项。
- 最近和哪个参与者说话。

本轮最重要的是分清：

```text
ActiveSession.CurrentNodeID：当前这次对话此刻显示哪个节点
DialogueRuntimeMap[DialogueID].CurrentNodeID：这段对话长期记录里最近到过哪个节点
```

`SelectOption` 成功跳转时，两者都应该更新。

---

## 4. 执行任务

### Task 1: 先读代码并手写流程图

**Files:**

- Read: `D:/unreal project/EscapeGame/Source/EscapeGame/Dialogue/DialogueQuestSubsystem.cpp`
- Read: `D:/unreal project/EscapeGame/Source/EscapeGame/Dialogue/EscapeDialogueTypes.h`

- [ ] **Step 1: 阅读 `StartConversation`**

重点看它写入了哪些状态：

```cpp
ActiveSession.DialogueID = DialogueAsset->DialogueID;
ActiveSession.CurrentNodeID = StartNodeID;
ActiveSession.bIsActive = true;

FDialogueRuntimeState& State = DialogueRuntimeMap.FindOrAdd(ActiveSession.DialogueID);
State.DialogueID = ActiveSession.DialogueID;
State.bHasSeenDialogue = true;
State.CurrentNodeID = StartNodeID;
State.LastTalkPartnerID = Participant->ParticipantID;
State.SeenNodeIDs.Add(StartNodeID);
```

- [ ] **Step 2: 阅读 `GetCurrentNode`**

你要确认它依赖这两个值：

```cpp
ActiveSession.DialogueID
ActiveSession.CurrentNodeID
```

所以 `SelectOption` 只要正确改 `ActiveSession.CurrentNodeID`，下一次 `GetCurrentNode()` 就应该能拿到新节点。

- [ ] **Step 3: 阅读 `FDialogueOption`**

重点字段：

```cpp
FGameplayTag OptionID;
TArray<FDialogueCondition> Conditions;
TArray<FDialogueEffect> Effects;
FGameplayTag NextNodeID;
bool bCloseDialogueAfterSelected = false;
```

本轮只使用：

```cpp
OptionID
NextNodeID
bCloseDialogueAfterSelected
```

- [ ] **Step 4: 在纸面或笔记中写出目标流程**

你要写出类似下面的流程：

```text
SelectOption(OptionID)
  -> 检查当前有激活对话
  -> 检查 OptionID 有效
  -> 读取当前节点
  -> 在 CurrentNode.Options 里找到匹配选项
  -> 找不到就 return false
  -> 记录 LastSelectedOptionID
  -> 记录 VisitedOptionIDs
  -> 如果 bCloseDialogueAfterSelected，EndConversation(NormalEnd)
  -> 否则如果 NextNodeID 有效，切换 CurrentNodeID
  -> 记录新节点 SeenNodeIDs
  -> return true
```

**验证方式：**

你能不用看代码，用自己的话解释：

```text
为什么 SelectOption 要同时更新 ActiveSession 和 DialogueRuntimeMap？
```

---

### Task 2: 创建失败的 Automation Test

**Files:**

- Create: `D:/unreal project/EscapeGame/Source/EscapeGame/Tests/DialogueQuestSubsystemTest.cpp`

- [ ] **Step 1: 新建测试文件**

创建：

```text
D:/unreal project/EscapeGame/Source/EscapeGame/Tests/DialogueQuestSubsystemTest.cpp
```

写入以下内容：

```cpp
#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Dialogue/DialogueDefinition.h"
#include "Dialogue/DialogueQuestSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueQuestSubsystemSelectOptionChangesNodeTest,
	"EscapeGame.Dialogue.SelectOption.ChangesCurrentNode",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FDialogueQuestSubsystemSelectOptionChangesNodeTest::RunTest(const FString& Parameters)
{
	UDialogueQuestSubsystem* Subsystem = NewObject<UDialogueQuestSubsystem>();
	TestNotNull(TEXT("Subsystem can be created"), Subsystem);

	UDialogueDefinition* Dialogue = NewObject<UDialogueDefinition>();
	TestNotNull(TEXT("Dialogue asset can be created"), Dialogue);

	const FGameplayTag DialogueID = FGameplayTag::RequestGameplayTag(TEXT("Dialogue.Test.SelectOption"));
	const FGameplayTag RootNodeID = FGameplayTag::RequestGameplayTag(TEXT("Dialogue.Node.Test.Root"));
	const FGameplayTag NextNodeID = FGameplayTag::RequestGameplayTag(TEXT("Dialogue.Node.Test.Next"));
	const FGameplayTag OptionID = FGameplayTag::RequestGameplayTag(TEXT("Dialogue.Option.Test.GoNext"));
	const FGameplayTag NPCID = FGameplayTag::RequestGameplayTag(TEXT("NPC.Test.Dialogue"));

	Dialogue->DialogueID = DialogueID;

	FDialogueOption GoNextOption;
	GoNextOption.OptionID = OptionID;
	GoNextOption.OptionText = FText::FromString(TEXT("Go next"));
	GoNextOption.NextNodeID = NextNodeID;

	FDialogueNode RootNode;
	RootNode.NodeID = RootNodeID;
	RootNode.SpeakerID = NPCID;
	RootNode.SpeakerText = FText::FromString(TEXT("Root"));
	RootNode.Options.Add(GoNextOption);

	FDialogueNode NextNode;
	NextNode.NodeID = NextNodeID;
	NextNode.SpeakerID = NPCID;
	NextNode.SpeakerText = FText::FromString(TEXT("Next"));

	Dialogue->Nodes.Add(RootNode);
	Dialogue->Nodes.Add(NextNode);

	Subsystem->LoadedDialogueDefinitions.Add(DialogueID, Dialogue);
	Subsystem->bConversationOpen = true;
	Subsystem->ActiveSession.DialogueID = DialogueID;
	Subsystem->ActiveSession.NPC_ID = NPCID;
	Subsystem->ActiveSession.CurrentNodeID = RootNodeID;
	Subsystem->ActiveSession.bIsActive = true;

	FDialogueRuntimeState& RuntimeState = Subsystem->DialogueRuntimeMap.FindOrAdd(DialogueID);
	RuntimeState.DialogueID = DialogueID;
	RuntimeState.CurrentNodeID = RootNodeID;
	RuntimeState.bHasSeenDialogue = true;
	RuntimeState.SeenNodeIDs.Add(RootNodeID);

	const bool bSelected = Subsystem->SelectOption(OptionID);

	TestTrue(TEXT("SelectOption should succeed for an option on the current node"), bSelected);
	TestEqual(TEXT("Active session moves to next node"), Subsystem->ActiveSession.CurrentNodeID, NextNodeID);
	TestEqual(TEXT("Last selected option is recorded"), Subsystem->ActiveSession.LastSelectedOptionID, OptionID);

	const FDialogueRuntimeState* UpdatedState = Subsystem->DialogueRuntimeMap.Find(DialogueID);
	TestNotNull(TEXT("Runtime state still exists"), UpdatedState);

	if (UpdatedState)
	{
		TestEqual(TEXT("Runtime current node moves to next node"), UpdatedState->CurrentNodeID, NextNodeID);
		TestTrue(TEXT("Visited option is recorded"), UpdatedState->VisitedOptionIDs.Contains(OptionID));
		TestTrue(TEXT("Next node is marked as seen"), UpdatedState->SeenNodeIDs.Contains(NextNodeID));
	}

	return true;
}

#endif
```

- [ ] **Step 2: 编译或运行测试，确认它失败**

先用你平时编译 UE 项目的方式编译项目。

如果从 Unreal Editor 里验证：

```text
Tools -> Test Automation
搜索：EscapeGame.Dialogue.SelectOption.ChangesCurrentNode
运行该测试
```

预期结果：

```text
测试失败
原因：SelectOption 当前返回 false，且不会切换 CurrentNodeID
```

如果编译失败，先不要改 `SelectOption`，把第一条编译错误贴出来。

---

### Task 3: 实现最小 SelectOption 跳转

**Files:**

- Modify: `D:/unreal project/EscapeGame/Source/EscapeGame/Dialogue/DialogueQuestSubsystem.cpp`

- [ ] **Step 1: 找到当前占位实现**

当前代码是：

```cpp
UE_LOG(LogTemp, Warning, TEXT("SelectOption 尚未落地：已撤回本轮选项执行实现。OptionID=%s"), *OptionID.ToString());
return false;
```

只替换这一段之后的逻辑，不改函数签名。

- [ ] **Step 2: 写最小实现**

目标实现：

```cpp
const FDialogueNode CurrentNode = GetCurrentNode();
if (!CurrentNode.NodeID.IsValid())
{
	UE_LOG(LogTemp, Error, TEXT("SelectOption 失败：当前节点无效。"));
	return false;
}

const FDialogueOption* SelectedOption = CurrentNode.Options.FindByPredicate(
	[OptionID](const FDialogueOption& Option)
	{
		return Option.OptionID == OptionID;
	});

if (!SelectedOption)
{
	UE_LOG(LogTemp, Error, TEXT("SelectOption 失败：当前节点不存在该选项。OptionID=%s NodeID=%s"),
		*OptionID.ToString(),
		*CurrentNode.NodeID.ToString());
	return false;
}

ActiveSession.LastSelectedOptionID = OptionID;

FDialogueRuntimeState& State = DialogueRuntimeMap.FindOrAdd(ActiveSession.DialogueID);
State.DialogueID = ActiveSession.DialogueID;
State.bHasSeenDialogue = true;
State.CurrentNodeID = ActiveSession.CurrentNodeID;
State.LastTalkPartnerID = ActiveSession.NPC_ID;
State.VisitedOptionIDs.Add(OptionID);

if (SelectedOption->bCloseDialogueAfterSelected)
{
	return EndConversation(EInterruptReason::NormalEnd);
}

if (!SelectedOption->NextNodeID.IsValid())
{
	UE_LOG(LogTemp, Warning, TEXT("SelectOption 成功：选项没有 NextNodeID，保持当前节点。OptionID=%s"),
		*OptionID.ToString());
	return true;
}

ActiveSession.CurrentNodeID = SelectedOption->NextNodeID;
State.CurrentNodeID = SelectedOption->NextNodeID;
State.SeenNodeIDs.Add(SelectedOption->NextNodeID);

return true;
```

注意：

- 本轮不要调用 `ApplyEffect`。
- 本轮不要判断 `Conditions`。
- 本轮不要排序选项。
- 本轮不要改 `GetAvailableOptions`。

- [ ] **Step 3: 编译**

预期：

```text
项目编译通过
```

如果编译失败，先处理第一条错误。不要连锁修改无关文件。

---

### Task 4: 运行 SelectOption 跳转测试

**Files:**

- Test: `D:/unreal project/EscapeGame/Source/EscapeGame/Tests/DialogueQuestSubsystemTest.cpp`

- [ ] **Step 1: 运行测试**

在 Unreal Editor 的 Automation Test 里搜索并运行：

```text
EscapeGame.Dialogue.SelectOption.ChangesCurrentNode
```

预期：

```text
PASS
```

- [ ] **Step 2: 如果测试失败，按失败点定位**

如果失败信息是：

```text
Active session moves to next node
```

说明没有正确更新：

```cpp
ActiveSession.CurrentNodeID
```

如果失败信息是：

```text
Visited option is recorded
```

说明没有正确更新：

```cpp
State.VisitedOptionIDs
```

如果失败信息是：

```text
Next node is marked as seen
```

说明没有正确更新：

```cpp
State.SeenNodeIDs
```

---

### Task 5: 补一个关闭对话测试

**Files:**

- Modify: `D:/unreal project/EscapeGame/Source/EscapeGame/Tests/DialogueQuestSubsystemTest.cpp`

- [ ] **Step 1: 在同一个测试文件里追加测试**

追加：

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueQuestSubsystemSelectOptionClosesConversationTest,
	"EscapeGame.Dialogue.SelectOption.ClosesConversation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FDialogueQuestSubsystemSelectOptionClosesConversationTest::RunTest(const FString& Parameters)
{
	UDialogueQuestSubsystem* Subsystem = NewObject<UDialogueQuestSubsystem>();
	TestNotNull(TEXT("Subsystem can be created"), Subsystem);

	UDialogueDefinition* Dialogue = NewObject<UDialogueDefinition>();
	TestNotNull(TEXT("Dialogue asset can be created"), Dialogue);

	const FGameplayTag DialogueID = FGameplayTag::RequestGameplayTag(TEXT("Dialogue.Test.CloseOption"));
	const FGameplayTag RootNodeID = FGameplayTag::RequestGameplayTag(TEXT("Dialogue.Node.Test.CloseRoot"));
	const FGameplayTag OptionID = FGameplayTag::RequestGameplayTag(TEXT("Dialogue.Option.Test.Close"));
	const FGameplayTag NPCID = FGameplayTag::RequestGameplayTag(TEXT("NPC.Test.Dialogue"));

	Dialogue->DialogueID = DialogueID;

	FDialogueOption CloseOption;
	CloseOption.OptionID = OptionID;
	CloseOption.OptionText = FText::FromString(TEXT("Close"));
	CloseOption.bCloseDialogueAfterSelected = true;

	FDialogueNode RootNode;
	RootNode.NodeID = RootNodeID;
	RootNode.SpeakerID = NPCID;
	RootNode.SpeakerText = FText::FromString(TEXT("Root"));
	RootNode.Options.Add(CloseOption);

	Dialogue->Nodes.Add(RootNode);

	Subsystem->LoadedDialogueDefinitions.Add(DialogueID, Dialogue);
	Subsystem->bConversationOpen = true;
	Subsystem->ActiveSession.DialogueID = DialogueID;
	Subsystem->ActiveSession.NPC_ID = NPCID;
	Subsystem->ActiveSession.CurrentNodeID = RootNodeID;
	Subsystem->ActiveSession.bIsActive = true;

	FDialogueRuntimeState& RuntimeState = Subsystem->DialogueRuntimeMap.FindOrAdd(DialogueID);
	RuntimeState.DialogueID = DialogueID;
	RuntimeState.CurrentNodeID = RootNodeID;
	RuntimeState.bHasSeenDialogue = true;
	RuntimeState.SeenNodeIDs.Add(RootNodeID);

	const bool bSelected = Subsystem->SelectOption(OptionID);

	TestTrue(TEXT("SelectOption should succeed for close option"), bSelected);
	TestFalse(TEXT("Conversation is no longer active"), Subsystem->IsConversationActive());
	TestFalse(TEXT("Conversation open flag is false"), Subsystem->bConversationOpen);
	TestEqual(TEXT("Interrupt reason is normal end"), Subsystem->ActiveSession.InterruptReason, EInterruptReason::NormalEnd);

	const FDialogueRuntimeState* UpdatedState = Subsystem->DialogueRuntimeMap.Find(DialogueID);
	TestNotNull(TEXT("Runtime state still exists"), UpdatedState);

	if (UpdatedState)
	{
		TestTrue(TEXT("Close option is recorded"), UpdatedState->VisitedOptionIDs.Contains(OptionID));
		TestEqual(TEXT("Runtime state remains on root node when closing"), UpdatedState->CurrentNodeID, RootNodeID);
	}

	return true;
}
```

- [ ] **Step 2: 运行关闭对话测试**

在 Automation Test 中运行：

```text
EscapeGame.Dialogue.SelectOption.ClosesConversation
```

预期：

```text
PASS
```

---

### Task 6: 手动复盘

**Files:**

- Modify only if you choose to record notes: `D:/unreal project/EscapeGame/Source/EscapeGame/Docs/Plan/dialogue_select_option_execution_plan.md`

- [ ] **Step 1: 回答三个问题**

写在你自己的笔记里，或者贴给 Agent：

```text
1. SelectOption 为什么不能只改 ActiveSession，不改 DialogueRuntimeMap？
2. 为什么本轮不应该顺手做 ApplyEffect？
3. bCloseDialogueAfterSelected=true 时，为什么可以复用 EndConversation(NormalEnd)？
```

- [ ] **Step 2: 记录验证结果**

记录：

```text
执行的测试：
- EscapeGame.Dialogue.SelectOption.ChangesCurrentNode
- EscapeGame.Dialogue.SelectOption.ClosesConversation

结果：
- PASS / FAIL

如果失败：
- 第一条失败信息是什么？
- 你判断它对应哪个状态没有更新？
```

---

## 5. 完成本轮后的状态

完成后，Dialogue 系统应该从：

```text
能开始对话
能获取当前节点
能结束对话
```

推进到：

```text
能开始对话
能获取当前节点
能选择选项
能跳到下一个节点
能记录已选择选项
能通过选项正常结束对话
```

这就是后续 `GetAvailableOptions`、`EvaluateCondition`、`ApplyEffect`、`UDialogueWidget` 的基础。

### 2026-06-29 实际完成状态

已通过测试：

```text
EscapeGame.Dialogue.SelectOption.ChangesCurrentNode
EscapeGame.Dialogue.SelectOption.ClosesConversation
EscapeGame.Dialogue.GetAvailableOptions.GlobalFlagConditionShowsMatchingOption
EscapeGame.Dialogue.GetAvailableOptions.GlobalFlagConditionHidesNonMatchingOption
EscapeGame.Dialogue.GetAvailableOptions.DialogueNodeSeenConditionHidesUnseenOption
EscapeGame.Dialogue.GetAvailableOptions.DialogueNodeSeenConditionShowsSeenOption
```

当前已具备：

```text
能开始对话
能获取当前节点
能选择选项
能跳到下一个节点
能记录已选择选项
能通过选项正常结束对话
能按 GlobalFlagIs 筛选选项
能按 DialogueNodeSeen 筛选选项
```

仍未纳入本计划完成范围：

```text
ApplyEffect
StartQuest
GiveItem / Reward
SaveGame 映射
完整 Dialogue UI 刷新与选项按钮点击
Boss / Encounter 接入
```

---

## 6. 自检清单

- [ ] 没有修改 `DialogueQuestSubsystem.h`。
- [ ] 没有修改 `EscapeDialogueTypes.h`。
- [ ] `SelectOption` 没有调用 `ApplyEffect`。
- [ ] `SelectOption` 没有判断 `Conditions`。
- [ ] `SelectOption` 找不到选项时返回 `false` 并输出 `UE_LOG(Error)`。
- [ ] 跳转选项成功时返回 `true`。
- [ ] 关闭选项成功时返回 `true`，并让 `IsConversationActive()` 变为 `false`。
- [ ] 至少两个 Automation Test 通过。

---

## 7. 你现在的第一条任务

先做 Task 1。

不要急着写代码。你先阅读：

```text
DialogueQuestSubsystem.cpp
EscapeDialogueTypes.h
```

然后用你自己的话回答：

```text
为什么 SelectOption 要同时更新 ActiveSession 和 DialogueRuntimeMap？
```

你回答后，我再 review 你的理解，再进入 Task 2。
