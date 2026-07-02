# Dialogue 文件夹架构梳理

最后更新：2026-07-01

本文档用于记录当前 `Source/EscapeGame/Dialogue` 文件夹中已经存在的架构，并对齐 `Docs/Plan/Mission_And_dialogue.md` 中的预期发展计划。它不是新功能实现方案，而是对现有代码和目标架构之间关系的学习型梳理，帮助后续按“小切片 + 手动验证 + 纠错复盘”的方式继续推进 Dialogue 系统。

---

## 1. 总体认知

当前 Dialogue 文件夹不是单纯的 NPC 对白文本系统，而是一个“对话 + 任务 + 全局剧情状态 + 存档”的系统雏形。

整体可以按 5 层理解：

```text
类型定义层
  ↓
静态配置资产层
  ↓
场景参与者层
  ↓
运行时管理层
  ↓
存档层
```

更具体地说：

```text
DataAsset 定义内容
        ↓
NPC / Component 提供“谁在说、用哪份对话”
        ↓
Subsystem 控制“现在聊到哪、能选什么、结束时记录什么”
        ↓
SaveGame 保存长期进度
```

---

## 2. 文件分层

### 2.1 类型定义层

文件：

```text
Source/EscapeGame/Dialogue/EscapeDialogueTypes.h
```

职责：

- 定义 Dialogue 系统能表达的节点、条件、效果、任务状态和中断原因。
- 定义静态配置结构体、运行时状态结构体和存档记录结构体。

关键类型：

- `EDialogueNodeType`：对话节点类型，例如普通对白、选项、奖励、结束、条件分支、Boss 开场。
- `EDialogueTriggerMode`：触发方式，例如手动交互、进入范围自动触发、战斗前强制触发。
- `EDialogueConditionType`：条件类型，例如任务状态、是否有物品、全局旗标、是否看过节点。
- `EDialogueEffectType`：效果类型，例如开始任务、设置任务状态、给物品、启动 Boss 战、保存游戏。
- `EQuestState`：任务状态，例如未开始、进行中、可交付、已完成、奖励已领取。
- `EInterruptReason`：对话中断原因，例如离开范围、打开菜单、死亡、切图、进入战斗。

关键结构体：

- `FDialogueCondition`：一条判断规则。
- `FDialogueEffect`：一条执行效果。
- `FDialogueOption`：玩家可点击的一条选项。
- `FDialogueNode`：对话树里的一个节点。
- `FQuestObjectiveDefinition`：任务目标配置。
- `FQuestRewardDefinition`：任务奖励配置。
- `FConversationSession`：当前正在进行的一次对话。
- `FDialogueRuntimeState`：某段对话的长期进度。
- `FQuestRuntimeState`：某个任务的运行时状态。
- `FGlobalFlagState`：跨系统共享的世界状态。
- `FDialogueSaveRecord`、`FQuestSaveRecord`、`FGlobalFlagSaveRecord`、`FWorldEventSaveRecord`：用于写入 `USaveGame` 的存档快照结构。

学习重点：

这个文件在做“数据建模”。它回答的是“系统里有哪些状态、条件、动作和记录”，而不是“这些东西如何执行”。

---

### 2.2 静态配置资产层

文件：

```text
Source/EscapeGame/Dialogue/DialogueDefinition.h
Source/EscapeGame/Dialogue/QuestDefinition.h
```

#### UDialogueDefinition

`UDialogueDefinition : UDataAsset` 表示一份对话内容资产，也就是一棵对话树。

关键字段：

- `DialogueID`：对话资产唯一 ID。
- `StartNodeID`：默认起始节点。
- `Nodes`：这段对话包含的全部节点。
- `bAllowRestart`：看完后是否允许重新从起点打开。
- `bUseLocalizedText`：预留本地化开关。
- `DefaultInterruptPolicy`：这段对话默认使用的中断规则。

职责边界：

- 它只描述“对话是什么”。
- 它不保存玩家当前进度。
- 它不负责开始、结束、跳转或执行效果。

#### UQuestDefinition

`UQuestDefinition : UDataAsset` 表示一份任务配置资产。

关键字段：

- `QuestID`
- `QuestTitle`
- `QuestDescription`
- `QuestGiverNPC_ID`
- `TurnInNPC_ID`
- `Prerequisites`
- `Objectives`
- `Rewards`
- `bCanRepeat`
- `bAutoComplete`

职责边界：

- 它只描述“任务是什么”。
- 它不保存玩家当前做到了哪一步。
- 任务进度应该放在 `FQuestRuntimeState` 和存档记录中。

---

### 2.3 场景参与者层

文件：

```text
Source/EscapeGame/Dialogue/DialogueParticipantComponent.h
Source/EscapeGame/Dialogue/DialogueNPC.h
Source/EscapeGame/Dialogue/DialogueNPC.cpp
```

#### UDialogueParticipantComponent

`UDialogueParticipantComponent` 是可挂在任意 Actor 上的对话参与者组件。

它可以让普通 NPC、Boss、敌人、机关都复用同一套对话身份和触发配置。

关键字段：

- `ParticipantID`：参与者唯一 ID。
- `DisplayName`：显示名。
- `Portrait`：头像。
- `DefaultStartNodeID`：这个参与者默认从哪个节点开始。
- `DialogueDefinition`：默认使用哪份对话资产。
- `DefaultInterruptPolicy`：参与者默认中断规则。
- `TriggerMode`：手动交互、自动触发、战斗前强制触发等。
- `bCanTalk`
- `bFaceInstigator`
- `bLockPlayer`
- `bCanSkip`
- `bAutoStartConversation`
- `bForceDialogueBeforeCombat`
- `bOnlyForceOnce`
- `bStartEncounterAfterDialogue`
- `LinkedEncounterTag`
- `ForcedDialoguePlayedFlag`

学习重点：

这是 UE 中常见的组件式拆分。Actor 不直接塞满所有对话字段，而是把“对话身份和触发配置”拆到 Component 中。

#### ADialogueNPC

`ADialogueNPC : AActor, IInteractableInterface` 是普通可交互 NPC 的包装 Actor。

它的职责很窄：

- 构造并持有 `DialogueParticipantComp`。
- 实现交互接口。
- 玩家交互时找到 `UDialogueQuestSubsystem`。
- 把 `InstigatorPawn` 和 `DialogueParticipantComp` 交给 Subsystem。

当前交互流程：

```text
玩家按交互键
  ↓
ADialogueNPC::Interact_Implementation
  ↓
检查 bCanInteract
  ↓
检查 InstigatorPawn
  ↓
检查 DialogueParticipantComp
  ↓
GetGameInstance()
  ↓
GetSubsystem<UDialogueQuestSubsystem>()
  ↓
StartConversation(InstigatorPawn, DialogueParticipantComp)
```

职责边界：

- `ADialogueNPC` 不自己管理对话状态。
- `ADialogueNPC` 不解析对话节点。
- `ADialogueNPC` 不执行任务或奖励逻辑。
- 它只是交互入口和转发者。

---

### 2.4 运行时管理层

文件：

```text
Source/EscapeGame/Dialogue/DialogueQuestSubsystem.h
Source/EscapeGame/Dialogue/DialogueQuestSubsystem.cpp
```

`UDialogueQuestSubsystem : UGameInstanceSubsystem` 是当前 Dialogue 系统的运行时中枢。

它负责：

- 开始对话。
- 结束对话。
- 保存当前会话。
- 管理对话运行进度。
- 管理任务运行进度。
- 管理全局旗标。
- 缓存已加载的对话和任务资产。
- 广播对话开始和结束事件。

关键字段：

- `ActiveSession`：当前正在进行的对话会话。
- `DialogueRuntimeMap`：每段对话的运行时进度。
- `QuestRuntimeMap`：任务运行时进度。
- `GlobalFlagMap`：全局剧情旗标状态。
- `LoadedDialogueDefinitions`：已加载对话资产缓存。
- `LoadedQuestDefinitions`：已加载任务资产缓存。
- `CurrentSaveGame`：当前存档对象。
- `SaveSlotName`
- `UserIndex`
- `bAutoSaveEnabled`
- `bConversationOpen`
- `OnConversationStarted`
- `OnConversationEnded`
- `PendingRewardQueue`

已经落地的核心函数：

- `StartConversation`
- `EndConversation`
- `IsConversationActive`
- `GetCurrentNode`
- `GetAvailableOptions`
- `SelectOption`
- `EvaluateCondition`（最小支持 `GlobalFlagIs` 与 `DialogueNodeSeen`）

尚未真正落地的核心函数：

- `ApplyEffect`

当前 `ApplyEffect` 仍然以日志提示形式占位；任务、奖励、存档和完整 UI 刷新仍未接入。

---

## 3. 当前已具备的最小运行闭环

当前系统已经具备的最小闭环已经从“开始对话 -> 获取当前节点 -> 结束对话”，推进到“开始对话 -> 获取当前节点 -> 筛选可用选项 -> 选择选项 -> 跳转或关闭对话”。

开始对话流程：

```text
玩家交互 NPC
  ↓
ADialogueNPC::Interact_Implementation
  ↓
UDialogueQuestSubsystem::StartConversation
  ↓
检查 Instigator / Participant / bCanTalk / 是否已有对话
  ↓
加载 Participant 上配置的 DialogueDefinition
  ↓
决定 StartNodeID
  ↓
写入 ActiveSession
  ↓
更新 DialogueRuntimeMap
  ↓
广播 OnConversationStarted
```

获取当前节点流程：

```text
GetCurrentNode
  ↓
检查当前是否有激活对话
  ↓
检查 DialogueID 和 CurrentNodeID
  ↓
从 LoadedDialogueDefinitions 找到对话资产
  ↓
在 Nodes 中查找 CurrentNodeID 对应的 FDialogueNode
  ↓
返回节点给 UI 或调用方
```

结束对话流程：

```text
EndConversation
  ↓
检查当前是否有激活对话
  ↓
把当前节点写回 DialogueRuntimeMap
  ↓
记录中断原因
  ↓
ActiveSession.bIsActive = false
  ↓
bConversationOpen = false
  ↓
广播 OnConversationEnded
```

获取可用选项流程：

```text
GetAvailableOptions
  ↓
检查当前是否有激活对话
  ↓
读取当前节点
  ↓
遍历 CurrentNode.Options
  ↓
对每个 Option 的 Conditions 逐条调用 EvaluateCondition
  ↓
全部条件通过的选项加入 AvailableOptions
  ↓
返回可显示或可点击的选项
```

当前 `EvaluateCondition` 已最小支持：

- `GlobalFlagIs`
- `DialogueNodeSeen`

选择选项流程：

```text
SelectOption
  ↓
检查当前是否有激活对话
  ↓
读取当前节点并查找 OptionID
  ↓
记录 ActiveSession.LastSelectedOptionID
  ↓
记录 DialogueRuntimeMap[DialogueID].VisitedOptionIDs
  ↓
如果 bCloseDialogueAfterSelected=true，复用 EndConversation(NormalEnd)
  ↓
否则如果 NextNodeID 有效，更新 ActiveSession.CurrentNodeID
  ↓
同步更新 DialogueRuntimeMap[DialogueID].CurrentNodeID 与 SeenNodeIDs
```

---

## 4. 存档层

文件：

```text
Source/EscapeGame/Dialogue/EscapeDialogueSaveGame.h
Source/EscapeGame/Dialogue/EscapeDialogueSaveGame.cpp
```

`UEscapeDialogueSaveGame : USaveGame` 是对话与任务系统的长期存档容器。

保存内容：

- `SaveVersion`
- `LastMapName`
- `SavedQuestStates`
- `SavedDialogueStates`
- `SavedGlobalFlags`
- `SavedWorldEvents`
- `SaveTimestamp`

职责边界：

- 它只保存数据。
- 它不负责把运行时 Map 转成数组。
- 它不负责真正调用 SaveGame API。
- 保存和读取流程应该由 `UDialogueQuestSubsystem` 负责。

---

## 5. 当前架构中的核心边界

### 5.1 静态配置 vs 运行时状态

静态配置：

- `UDialogueDefinition`
- `UQuestDefinition`
- `FDialogueNode`
- `FDialogueOption`
- `FQuestObjectiveDefinition`
- `FQuestRewardDefinition`

运行时状态：

- `FConversationSession`
- `FDialogueRuntimeState`
- `FQuestRuntimeState`
- `FObjectiveRuntimeState`
- `FGlobalFlagState`

存档快照：

- `FDialogueSaveRecord`
- `FQuestSaveRecord`
- `FObjectiveSaveRecord`
- `FGlobalFlagSaveRecord`
- `FWorldEventSaveRecord`

这是当前系统最重要的设计边界。

### 5.2 Actor vs Component vs Subsystem

`ADialogueNPC`：

- 负责可交互入口。
- 不负责对话规则。

`UDialogueParticipantComponent`：

- 负责“这个对象是谁、用哪份对话、怎么触发”。
- 不负责全局进度。

`UDialogueQuestSubsystem`：

- 负责当前对话会话、运行时状态、全局旗标、任务状态和事件广播。
- 不应该承担 UI 绘制职责。

---

## 6. 当前完成状态与未完成部分

当前已经最小落地的核心行为：

1. 选项点击后的跳转：`SelectOption` 已支持查找选项、记录选择、跳到 `NextNodeID`、关闭对话。
2. 条件筛选：`GetAvailableOptions` 已遍历选项条件，并通过 `EvaluateCondition` 做最小筛选。
3. 条件判断：`EvaluateCondition` 已支持 `GlobalFlagIs` 与 `DialogueNodeSeen`，其他条件类型默认不通过。

当前代码里已经明确存在但尚未落地的核心行为：

1. 效果执行：`ApplyEffect` 当前仍是占位日志。
2. 任务状态推进。
3. 存档读写。
4. 完整 UI 刷新与选项按钮点击。
5. 中断策略实际执行。
6. Boss / Encounter 相关触发。

其中最自然的下一步不是直接做完整任务系统，也不是先做存档，而是先补上 `ApplyEffect` 的最小效果执行，让选项能够改变全局旗标或关闭对话。

---

## 7. 与 Mission_And_dialogue 计划的对齐

`Mission_And_dialogue.md` 是 Dialogue 系统的预期发展计划。它的核心目标是：

```text
做出一套可复用、可扩展的对话与任务框架，
支持普通 NPC、Boss 开场对白、接任务、交任务、谜题推进、奖励发放和存档读档，
并且保持 Blueprint 可配置。
```

当前源码已经基本接受了这份计划的主要架构方向：

- `UDialogueQuestSubsystem` 作为运行时状态与规则执行中心。
- `UDialogueDefinition` 和 `UQuestDefinition` 作为 `UDataAsset` 内容配置。
- `UEscapeDialogueSaveGame` 作为持久化容器。
- `ADialogueNPC` 只作为普通 NPC 的 Actor 包装和手动交互入口。
- `UDialogueParticipantComponent` 作为普通 NPC、Boss、敌人、机关都能复用的对话参与者入口。
- 使用 `GameplayTag` 作为对话、任务、旗标、遭遇战、节点、选项等 ID。

需要注意的是，`Mission_And_dialogue.md` 中的“当前实现状态”是较早阶段的计划快照，不能直接当成现在的源码事实。例如它记录 `UDialogueQuestSubsystem`、`UQuestDefinition`、`UEscapeDialogueSaveGame` 尚未创建，但当前源码中这些类型已经存在。因此后续阅读时应该这样区分：

```text
Mission_And_dialogue.md：目标架构与路线图
dialogue_folder_architecture_overview.md：当前源码事实 + 与目标架构的差距
```

---

## 8. 计划中的 MVP 范围与当前差距

`Mission_And_dialogue.md` 中定义的 MVP 目标是：

- NPC 交互后打开对话 UI。
- `UDialogueDefinition` 支持单段对白、选项、选项跳转和关闭。
- `UDialogueParticipantComponent` 提供对话资产、起始节点、显示信息和触发策略。
- `UDialogueQuestSubsystem` 能创建会话、读取当前节点、筛选可见选项、响应选项点击。
- 支持 `SetGlobalFlag`、`CloseDialogue`、`StartQuest` 三类基础效果。
- 支持 `GlobalFlagIs`、`DialogueNodeSeen` 两类基础条件。
- MVP 优先做普通 NPC 手动对话。
- 战斗前强制对白先写入字段和流程规范，完整 Boss 接入放到后续阶段。
- 暂不做完整存档、完整 Boss AI 接入、奖励队列、复杂任务目标、谜题目标。

对照当前源码，状态可以整理为：

| 能力 | 当前状态 | 说明 |
|---|---|---|
| 公共类型与字段 | 已大体存在 | `EscapeDialogueTypes.h` 已包含大量枚举、条件、效果、运行时状态和存档记录 |
| 对话 DataAsset | 已存在 | `UDialogueDefinition` 已包含 `DialogueID`、`StartNodeID`、`Nodes` 等字段 |
| 任务 DataAsset | 已存在 | `UQuestDefinition` 已包含任务标题、目标、奖励、前置条件等字段 |
| 参与者组件 | 已存在 | `UDialogueParticipantComponent` 已承担身份、对话资产、触发模式和 Boss 强制对白配置 |
| 普通 NPC 包装 | 已存在 | `ADialogueNPC` 已实现 `IInteractableInterface` 并转发到 Subsystem |
| 开始对话 | 已部分落地 | `StartConversation` 可加载对话资产、创建会话、写入运行时状态并广播事件 |
| 获取当前节点 | 已部分落地 | `GetCurrentNode` 可从缓存资产中查找当前节点 |
| 获取选项 | 已最小落地 | `GetAvailableOptions` 已按选项 Conditions 做最小筛选 |
| 选择选项 | 已最小落地 | `SelectOption` 已支持选项查找、跳转、记录历史与关闭对话 |
| 条件判断 | 已最小落地 | `EvaluateCondition` 已支持 `GlobalFlagIs` 与 `DialogueNodeSeen`，其他条件默认不通过 |
| 效果执行 | 未落地 | `ApplyEffect` 当前只输出未落地日志 |
| UI 接入 | 最小 C++ Widget / WBP 已创建，完整刷新链路未落地 | `UDialogueWidget` 和 `WBP_Dialogue` 已能作为基础 UI 类型，后续仍需接选项按钮点击与刷新 |
| 存档读写 | 容器已存在，流程未落地 | `UEscapeDialogueSaveGame` 已有字段，但 Subsystem 尚未实现 Save/Load 映射 |
| Boss / Encounter 接入 | 字段已预留，流程未落地 | `UDialogueParticipantComponent` 有相关字段，实际触发链还未实现 |

所以当前最准确的判断是：

```text
系统骨架已从“计划阶段”推进到“会话骨架阶段”。
现在卡点不在类型定义，而在运行时行为闭环。
```

---

## 9. GameplayTag 命名规范

`Mission_And_dialogue.md` 建议使用稳定、可读、可跨资产引用的 `GameplayTag`，当前架构也已经围绕 `FGameplayTag` 展开。

推荐继续沿用以下命名模式：

| 类型 | 命名模式 | 示例 |
|---|---|---|
| 对话资产 | `Dialogue.<角色或地点>.<主题>` | `Dialogue.Gatekeeper.Intro` |
| 对话节点 | `Dialogue.Node.<角色>.<节点名>` | `Dialogue.Node.Gatekeeper.Root` |
| 对话选项 | `Dialogue.Option.<角色>.<动作>` | `Dialogue.Option.Gatekeeper.AcceptQuest` |
| NPC / 参与者 | `NPC.<区域>.<名称>` | `NPC.Village.Gatekeeper` |
| 任务 | `Quest.<类型>.<名称>` | `Quest.Main.FindGateKey` |
| 任务目标 | `Quest.Objective.<任务>.<目标>` | `Quest.Objective.FindGateKey.CollectKey` |
| 全局旗标 | `Flag.<系统>.<状态>` | `Flag.World.MainGateUnlocked` |
| 遭遇战 | `Encounter.<类型>.<名称>` | `Encounter.Boss.Gatekeeper` |
| 谜题 | `Puzzle.<区域>.<名称>` | `Puzzle.Courtyard.StatueOrder` |
| 奖励 | `Reward.<类型>.<名称>` | `Reward.Item.GateKey` |

边界规则：

- `DialogueID`、`QuestID`、`ParticipantID`、`EncounterID` 必须全项目唯一。
- 节点和选项最好也使用完整前缀，方便调试和存档排查。
- 存档主键应该保存稳定的 Tag，不依赖资产路径。
- Blueprint / DataAsset 中尽量避免裸字符串 ID。

---

## 10. 计划中的完整运行流程

根据 `Mission_And_dialogue.md`，完整系统最终应形成下面的流程：

```text
UInteractComponent 命中实现 IInteractableInterface 的对象
  ↓
目标对象把对话请求转给 UDialogueQuestSubsystem
  ↓
Subsystem 创建 FConversationSession
  ↓
Subsystem 加载对话树并检查 FDialogueCondition
  ↓
UDialogueWidget 显示节点和选项
  ↓
玩家选择选项
  ↓
Subsystem 执行 FDialogueEffect
  ↓
必要时执行 FRewardEffect
  ↓
Subsystem 更新任务、旗标和对话运行时状态
  ↓
如需持久化，Subsystem 写入 UEscapeDialogueSaveGame
```

Boss 开场对白也应复用同一路径，只是在触发入口和效果链末尾不同：

```text
敌人感知玩家 / 玩家进入战斗触发范围
  ↓
AI 或感知组件读取 UDialogueParticipantComponent
  ↓
TriggerMode == ForcedBeforeCombat 且 ForcedDialoguePlayedFlag=false
  ↓
暂停 AI 移动和攻击，锁定玩家输入
  ↓
UDialogueQuestSubsystem::StartConversation(Player, DialogueParticipantComp)
  ↓
对话结束后设置 ForcedDialoguePlayedFlag=true
  ↓
若 bStartEncounterAfterDialogue=true，则广播 Encounter 或写 Blackboard
  ↓
AI 进入战斗状态
```

当前源码已经完成了这条完整链路的前半段，并补上了选项跳转与条件筛选：

```text
交互入口
  ↓
StartConversation
  ↓
创建 ActiveSession
  ↓
GetCurrentNode / GetAvailableOptions
  ↓
SelectOption
  ↓
跳转节点 / EndConversation
```

---

## 11. 建议的下一个学习切片

建议下一个小切片：

```text
ApplyEffect 最小效果执行
```

目标：

```text
点击一个带 Effects 的选项
  ↓
SelectOption 找到选项并记录选择历史
  ↓
逐条执行 Option.Effects
  ↓
如果 EffectType == SetGlobalFlag，写入 GlobalFlagMap
  ↓
如果 EffectType == CloseDialogue，复用 EndConversation(NormalEnd)
```

本切片暂时不处理：

- 任务状态变化。
- 物品奖励。
- 自动存档。
- UI 动画。
- 延迟效果。
- 节点 OnEnterEffects。

学习目标：

- 理解 `FDialogueEffect` 是数据化的“动作请求”，真正执行由 Subsystem 负责。
- 区分“读状态的条件判断”和“写状态的效果执行”。
- 练习只让本切片修改 `GlobalFlagMap` 和会话关闭状态。
- 避免一上来把任务、奖励、存档、Boss 触发全部混在一起。

最小验证思路：

```text
1. 直接构造一个 SetGlobalFlag 的 FDialogueEffect。
2. 调用 ApplyEffect。
3. 检查 GlobalFlagMap[FlagID].bValue 是否符合 Effect.BoolValue。
4. 再构造一个 CloseDialogue 的 FDialogueEffect。
5. 在激活对话状态下调用 ApplyEffect。
6. 检查 IsConversationActive() 是否变为 false，InterruptReason 是否为 NormalEnd。
```

---

## 12. 后续切片顺序建议

为了对齐计划，又避免一次性把任务、存档、UI、Boss 全部混在一起，建议按下面顺序推进：

1. `SelectOption` 最小跳转闭环。（已完成）
2. `GetAvailableOptions` 加入最小条件筛选，只支持 `GlobalFlagIs` 和 `DialogueNodeSeen`。（已完成）
3. `ApplyEffect` 加入最小效果执行，只支持 `CloseDialogue` 和 `SetGlobalFlag`。
4. 接入一个最小 `UDialogueWidget`，只显示说话者、文本、选项按钮。
5. 加入 `StartQuest`，让对话能启动一个任务。
6. 加入 `AddObjectiveProgress` 或任务目标最小推进。
7. 做运行时状态到 `UEscapeDialogueSaveGame` 的保存映射。
8. 最后再接 Boss / Encounter 的强制对白链路。

这样推进的原因：

- 先让“对话节点能流动起来”。
- 再让“条件能控制选项”。
- 再让“选项能改变世界状态”。
- 最后再让任务、UI、存档和 Boss 接入这条管线。

---

## 13. 一句话总结

当前 Dialogue 文件夹已经搭好了“数据模型 + 对话资产 + 参与者组件 + NPC 交互入口 + Subsystem 会话管理 + 条件筛选 + 选项跳转 + 存档容器”的骨架，并且整体方向与 `Mission_And_dialogue.md` 的目标架构一致。但计划文档中的部分“当前状态”可能落后于源码，后续应以源码事实和开发日志为准，以计划文档作为目标路线图。下一步应优先补齐 `ApplyEffect` 的最小效果执行，而不是直接推进完整任务、奖励和存档系统。
