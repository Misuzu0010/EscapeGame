# Mission_And_dialogue 实现计划

> **给执行型代理的说明：** 需要配合 `superpowers:subagent-driven-development` 或 `superpowers:executing-plans`，按任务逐步推进。进度用复选框 `- [ ]` 标记。

**目标：** 做出一套可复用、可扩展的对话与任务框架，支持普通 NPC、Boss 开场对白、接任务、交任务、谜题推进、奖励发放和存档读档，并且保持 Blueprint 可配置。

**架构：** 由 `UDialogueQuestSubsystem` 负责运行时状态与规则执行，`UDataAsset` 类定义负责内容数据，`USaveGame` 负责持久化。`ADialogueNPC` 只负责普通非战斗 NPC 的 Actor 包装和手动交互入口；敌人、Boss、机关等非普通 NPC 不继承 `ADialogueNPC`，而是挂载 `UDialogueParticipantComponent` 复用同一套对话身份、触发策略和战斗前强制对白配置。

**技术栈：** Unreal Engine 5 C++、Blueprint、UMG、`UGameInstanceSubsystem`、`USaveGame`、`GameplayTags`、`UDataAsset`、`TMap` / `TArray`、现有 `UInteractComponent` + `IInteractableInterface`。

---

## 1. 设计边界

- `UDialogueDefinition` 和 `UQuestDefinition` 只描述内容，不保存运行时进度。
- `UDialogueQuestSubsystem` 负责运行时状态、条件判断、效果执行、存档读档。
- `UEscapeDialogueSaveGame` 只保存需要跨关卡保留的快照。
- `UDialogueWidget` 只负责显示，不直接改任务数据。
- `ADialogueNPC` 是普通非战斗 NPC 的蓝图友好包装，负责地图摆放、交互文本和默认组件创建。
- `UDialogueParticipantComponent` 是真正可复用的对话参与者入口，普通 NPC、敌人、Boss、雕像、门、机关都可以挂。
- 敌人或 Boss 的战斗前强制对白不走 `ADialogueNPC` 继承链，而由自身 Actor 上的 `UDialogueParticipantComponent` 配置和触发。

---

## 2. 文件边界计划

- `Source/EscapeGame/Dialogue/EscapeDialogueTypes.h`
  - 对话、任务、奖励、旗标、中断规则的公共枚举与小结构体。
- `Source/EscapeGame/Dialogue/DialogueDefinition.h/.cpp`
  - `UDialogueDefinition`，`FDialogueNode`，`FDialogueOption`，`FDialogueCondition`，`FDialogueEffect`。
- `Source/EscapeGame/Dialogue/QuestDefinition.h/.cpp`
  - `UQuestDefinition`，`FQuestObjectiveDefinition`，`FQuestRewardDefinition`。
- `Source/EscapeGame/Dialogue/DialogueParticipantComponent.h/.cpp`
  - `UDialogueParticipantComponent`，复用型对话入口。
- `Source/EscapeGame/Dialogue/DialogueNPC.h/.cpp`
  - `ADialogueNPC`，普通 NPC 包装类。
- `Source/EscapeGame/Dialogue/DialogueQuestSubsystem.h/.cpp`
  - 运行时状态、条件/效果执行、会话管理、存档读档。
- `Source/EscapeGame/Dialogue/EscapeDialogueSaveGame.h/.cpp`
  - 持久化任务、对话、旗标、世界事件记录。
- `Source/EscapeGame/UI/DialogueWidget.h/.cpp`
  - 对话 UI、选项列表、提示文本、跳过与关闭控制。

---

## 3. 推荐枚举

| 枚举 | 建议值 | 用途 |
|---|---|---|
| `EDialogueNodeType` | `Line`、`Choice`、`Reward`、`Exit`、`Branch`、`BossIntro` | 区分节点类型 |
| `EDialogueTriggerMode` | `ManualInteract`、`AutoOnOverlap`、`ForcedBeforeCombat`、`BossIntro`、`ScriptedOnly` | 定义对话由普通交互、重叠、战斗前强制、Boss 开场或脚本触发 |
| `EDialogueConditionType` | `QuestStateIs`、`ObjectiveCompleted`、`HasItem`、`GlobalFlagIs`、`DialogueNodeSeen`、`PuzzleSolved`、`BossStateIs` | 控制节点/选项是否可见 |
| `EConditionCompareMode` | `Equal`、`NotEqual`、`GreaterOrEqual`、`LessOrEqual`、`Contains`、`Exists` | 统一比较方式 |
| `EDialogueEffectType` | `StartQuest`、`SetQuestState`、`AddObjectiveProgress`、`SetGlobalFlag`、`GiveItem`、`RemoveItem`、`GrantReward`、`StartBossEncounter`、`BroadcastWorldEvent`、`CloseDialogue`、`SaveGame` | 选项点击后执行什么 |
| `EQuestState` | `NotStarted`、`Active`、`ReadyToTurnIn`、`Completed`、`RewardClaimed`、`Failed` | 任务生命周期 |
| `EObjectiveType` | `TalkToNPC`、`CollectItem`、`UseItem`、`SolvePuzzle`、`KillEnemy`、`ReachLocation`、`TriggerWorldEvent` | 任务目标类型 |
| `ERewardType` | `Message`、`Item`、`Attribute`、`WorldEvent`、`Unlock` | 奖励/反馈通道 |
| `EInterruptReason` | `LeaveRange`、`OpenMenu`、`Death`、`MapChange`、`CombatStart`、`HigherPriorityCutscene` | 对话中断原因 |

---

## 4. 字段表

### 4.1 `ADialogueNPC`

`ADialogueNPC` 只服务普通非战斗 NPC。它不保存完整对话身份，不负责敌人 / Boss 战斗前对白。普通 NPC 的显示名、头像、对话资产和起始节点由自带的 `UDialogueParticipantComponent` 持有。

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `DialogueParticipantComp` | `TObjectPtr<UDialogueParticipantComponent>` | 普通 NPC 默认创建的对话参与者组件 | 否 |
| `InteractionText` | `FText` | 交互提示文本，如“交谈” | 否 |
| `bCanInteract` | `bool` | 普通交互是否启用 | 否 |
| `bAutoStartDialogueOnInteract` | `bool` | 玩家交互后是否立即调用组件开始对话 | 否 |
| `bShowInteractionPrompt` | `bool` | 是否允许 UI 显示交互提示 | 否 |

约束：

- `ADialogueNPC` 应实现 `IInteractableInterface`，在 `Interact(APawn* InstigatorPawn)` 中把请求转给 `DialogueParticipantComp` 或 `UDialogueQuestSubsystem`。
- `ADialogueNPC` 不保存 `NPCDisplayName`、`NPCPortrait`、`DialogueAsset`、`DefaultStartNodeID` 的第二份副本，避免和组件配置不一致。
- 若需要普通 NPC 面向玩家或锁玩家输入，应读取 `DialogueParticipantComp` 上的 `bFaceInstigator` / `bLockPlayer`。

### 4.2 `UDialogueParticipantComponent`

`UDialogueParticipantComponent` 是普通 NPC、敌人、Boss 和机关共享的对话身份与触发策略载体。战斗前强制对白必须配置在组件上，而不是写进 `ADialogueNPC`。

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `ParticipantID` | `FGameplayTag` | 对话参与者标识，如 `NPC.Village.Gatekeeper` 或 `Enemy.Boss.Gatekeeper` | 是 |
| `DisplayName` | `FText` | 当前说话者名称 | 否 |
| `Portrait` | `TSoftObjectPtr<UTexture2D>` | 头像资源 | 否 |
| `DialogueAsset` | `TSoftObjectPtr<UDialogueDefinition>` | 默认对话树 | 否 |
| `DefaultStartNodeID` | `FGameplayTag` | 起始节点 | 否 |
| `DefaultInterruptPolicy` | `FInterruptPolicy` | 默认中断规则 | 否 |
| `TriggerMode` | `EDialogueTriggerMode` | 对话触发方式：手动交互、重叠、战斗前强制、Boss 开场或脚本触发 | 否 |
| `bCanTalk` | `bool` | 当前是否可对话 | 否 |
| `bFaceInstigator` | `bool` | 对话开始时是否朝向触发者 | 否 |
| `bLockPlayer` | `bool` | 对话期间是否锁玩家输入 / 移动 | 否 |
| `bCanSkip` | `bool` | 是否允许跳过 | 否 |
| `bAutoStartConversation` | `bool` | 重叠或脚本触发后是否自动开始会话 | 否 |
| `bForceDialogueBeforeCombat` | `bool` | 敌人 / Boss 进入战斗前是否必须先播放对白 | 是 |
| `bOnlyForceOnce` | `bool` | 强制对白是否只播放一次 | 是 |
| `bStartEncounterAfterDialogue` | `bool` | 对话结束后是否自动启动遭遇战 / 战斗状态 | 否 |
| `LinkedEncounterID` | `FGameplayTag` | 关联 Boss / 遭遇战标识 | 是 |
| `ForcedDialoguePlayedFlag` | `FGameplayTag` | 记录战斗前强制对白是否已播放的全局旗标 | 是 |

约束：

- 普通 NPC 通常使用 `ManualInteract`。
- 敌人战斗前对白使用 `ForcedBeforeCombat` 或 `BossIntro`。
- `bOnlyForceOnce=true` 时，Subsystem 应用 `ForcedDialoguePlayedFlag` 或对话运行时状态阻止重复强制播放。
- 对话结束后是否进入战斗由 `bStartEncounterAfterDialogue` 和 `LinkedEncounterID` 决定，不由 UI 决定。

### 4.3 `UDialogueDefinition`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `DialogueID` | `FGameplayTag` | 对话资产标识 | 是 |
| `StartNodeID` | `FGameplayTag` | 起始节点 | 是 |
| `Nodes` | `TArray<FDialogueNode>` | 节点列表 | 是 |
| `bAllowRestart` | `bool` | 是否允许重看 | 是 |
| `bUseLocalizedText` | `bool` | 是否保留本地化入口 | 是 |
| `DefaultInterruptPolicy` | `FInterruptPolicy` | 默认中断规则 | 是 |

### 4.4 `FDialogueNode`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `NodeID` | `FGameplayTag` | 节点唯一标识 | 是 |
| `SpeakerID` | `FGameplayTag` | 说话者标识 | 是 |
| `SpeakerText` | `FText` | 本节点对白 | 是 |
| `NodeType` | `EDialogueNodeType` | 节点类型 | 是 |
| `Conditions` | `TArray<FDialogueCondition>` | 节点显示条件 | 是 |
| `Options` | `TArray<FDialogueOption>` | 选项列表 | 是 |
| `OnEnterEffects` | `TArray<FDialogueEffect>` | 进入节点时触发的效果 | 是 |
| `InterruptPolicy` | `FInterruptPolicy` | 节点级中断规则 | 是 |
| `bAllowSkip` | `bool` | 是否允许跳过该节点 | 是 |

### 4.5 `FDialogueOption`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `OptionID` | `FGameplayTag` | 选项唯一标识 | 是 |
| `OptionText` | `FText` | 选项文本 | 是 |
| `Conditions` | `TArray<FDialogueCondition>` | 选项显示/可用条件 | 是 |
| `Effects` | `TArray<FDialogueEffect>` | 点击后执行的效果 | 是 |
| `NextNodeID` | `FGameplayTag` | 跳转到的下一节点 | 是 |
| `bCloseDialogueAfterSelected` | `bool` | 选择后是否关闭对话 | 是 |
| `bHideWhenUnavailable` | `bool` | 不可用时是否隐藏 | 是 |
| `SortOrder` | `int32` | 选项排序 | 是 |

### 4.6 `FDialogueCondition`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `ConditionType` | `EDialogueConditionType` | 条件类型 | 是 |
| `ComparisonMode` | `EConditionCompareMode` | 比较方式 | 是 |
| `QuestID` | `FGameplayTag` | 目标任务 | 是 |
| `ObjectiveID` | `FGameplayTag` | 目标任务目标 | 是 |
| `ItemID` | `FGameplayTag` | 目标物品 | 是 |
| `FlagID` | `FGameplayTag` | 目标全局旗标 | 是 |
| `EncounterID` | `FGameplayTag` | 目标遭遇战 / Boss | 是 |
| `ExpectedQuestState` | `EQuestState` | 任务状态比较值 | 是 |
| `ExpectedBoolValue` | `bool` | 布尔比较值 | 是 |
| `ExpectedIntValue` | `int32` | 数值比较值 | 是 |
| `ExpectedTagValue` | `FGameplayTag` | Tag 比较值 | 是 |
| `RequiredCount` | `int32` | 数量阈值 | 是 |
| `bInvert` | `bool` | 是否取反 | 是 |
| `FailureHint` | `FText` | 不满足时的提示 | 是 |

### 4.7 `FDialogueEffect`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `EffectType` | `EDialogueEffectType` | 效果类型 | 是 |
| `QuestID` | `FGameplayTag` | 目标任务 | 是 |
| `ObjectiveID` | `FGameplayTag` | 目标目标 | 是 |
| `ItemID` | `FGameplayTag` | 目标物品 | 是 |
| `FlagID` | `FGameplayTag` | 目标旗标 | 是 |
| `EncounterID` | `FGameplayTag` | 目标遭遇战 / Boss | 是 |
| `Count` | `int32` | 数量参数 | 是 |
| `BoolValue` | `bool` | 布尔参数 | 是 |
| `TextValue` | `FText` | 文本参数 | 是 |
| `FloatValue` | `float` | 浮点参数 | 是 |
| `DelaySeconds` | `float` | 延迟执行时间 | 是 |
| `bCommitSaveImmediately` | `bool` | 是否立刻保存 | 是 |

### 4.8 `UQuestDefinition`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `QuestID` | `FGameplayTag` | 任务标识 | 是 |
| `QuestTitle` | `FText` | 任务标题 | 是 |
| `QuestDescription` | `FText` | 任务描述 | 是 |
| `QuestGiverNPC_ID` | `FGameplayTag` | 接任务 NPC | 是 |
| `TurnInNPC_ID` | `FGameplayTag` | 交任务 NPC | 是 |
| `Prerequisites` | `TArray<FDialogueCondition>` | 接任务前置条件 | 是 |
| `Objectives` | `TArray<FQuestObjectiveDefinition>` | 任务目标列表 | 是 |
| `Rewards` | `TArray<FQuestRewardDefinition>` | 任务奖励列表 | 是 |
| `bCanRepeat` | `bool` | 是否可重复完成 | 是 |
| `bAutoComplete` | `bool` | 是否在最后一个目标完成后自动完成 | 是 |

### 4.9 `FQuestObjectiveDefinition`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `ObjectiveID` | `FGameplayTag` | 目标标识 | 是 |
| `ObjectiveType` | `EObjectiveType` | 目标类型 | 是 |
| `TargetID` | `FGameplayTag` | 目标对象标识 | 是 |
| `RequiredCount` | `int32` | 完成数量要求 | 是 |
| `Description` | `FText` | 目标说明 | 是 |
| `HintText` | `FText` | 辅助提示 | 是 |
| `Conditions` | `TArray<FDialogueCondition>` | 额外完成条件 | 是 |
| `bOptional` | `bool` | 是否为可选目标 | 是 |

### 4.10 `FQuestRewardDefinition`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `RewardType` | `ERewardType` | 奖励类型 | 是 |
| `TargetID` | `FGameplayTag` | 奖励目标 | 是 |
| `Count` | `int32` | 数量 | 是 |
| `AttributeDelta` | `float` | 属性变化值 | 是 |
| `UnlockID` | `FGameplayTag` | 解锁内容标识 | 是 |
| `MessageText` | `FText` | 奖励提示文本 | 是 |
| `bGrantImmediately` | `bool` | 是否立即发放 | 是 |

### 4.11 `FConversationSession`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `SessionId` | `FGuid` | 本次会话唯一编号 | 否 |
| `NPC_ID` | `FGameplayTag` | 当前对话对象 | 否 |
| `DialogueID` | `FGameplayTag` | 当前对话树 | 否 |
| `CurrentNodeID` | `FGameplayTag` | 当前节点 | 否 |
| `LastSelectedOptionID` | `FGameplayTag` | 最近选择的选项 | 否 |
| `bIsActive` | `bool` | 是否正在对话 | 否 |
| `bLockPlayer` | `bool` | 是否锁玩家输入 | 否 |
| `bCanSkip` | `bool` | 是否允许跳过 | 否 |
| `InterruptPolicy` | `FInterruptPolicy` | 本次会话中断规则 | 否 |
| `StartTimeSeconds` | `float` | 调试或超时用 | 否 |
| `InterruptReason` | `EInterruptReason` | 中断原因 | 否 |

### 4.12 `FDialogueRuntimeState`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `DialogueID` | `FGameplayTag` | 对话标识 | 是 |
| `CurrentNodeID` | `FGameplayTag` | 最近节点 | 是 |
| `bHasSeenDialogue` | `bool` | 是否曾打开过这段对话 | 是 |
| `SeenNodeIDs` | `TSet<FGameplayTag>` | 访问过的节点 | 是 |
| `VisitedOptionIDs` | `TSet<FGameplayTag>` | 选择过的选项 | 是 |
| `BranchFlags` | `TSet<FGameplayTag>` | 该对话专属分支标记 | 是 |
| `LastTalkPartnerID` | `FGameplayTag` | 最近说话对象 | 是 |

### 4.13 `FObjectiveRuntimeState`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `ObjectiveID` | `FGameplayTag` | 目标标识 | 是 |
| `CurrentCount` | `int32` | 当前进度 | 是 |
| `TargetCount` | `int32` | 完成阈值 | 是 |
| `bCompleted` | `bool` | 是否完成 | 是 |
| `SourceIDs` | `TSet<FGameplayTag>` | 进度来源 | 是 |
| `LastProgressTimeSeconds` | `float` | 调试用时间戳 | 是 |

### 4.14 `FQuestRuntimeState`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `QuestID` | `FGameplayTag` | 任务标识 | 是 |
| `QuestState` | `EQuestState` | 当前任务状态 | 是 |
| `ObjectiveStates` | `TMap<FGameplayTag, FObjectiveRuntimeState>` | 目标进度表 | 是 |
| `bRewardClaimed` | `bool` | 是否领过奖励 | 是 |
| `QuestGiverNPC_ID` | `FGameplayTag` | 接任务 NPC | 是 |
| `TurnInNPC_ID` | `FGameplayTag` | 交任务 NPC | 是 |
| `StartedTimeSeconds` | `float` | 调试 / 统计用 | 是 |
| `LastUpdateReason` | `FText` | 最近一次变化原因 | 是 |

### 4.15 `FGlobalFlagState`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `FlagID` | `FGameplayTag` | 全局旗标标识 | 是 |
| `bValue` | `bool` | 布尔值 | 是 |
| `NumericValue` | `int32` | 数值值 | 是 |
| `TextValue` | `FText` | 文本值 | 是 |
| `LastChangedTimeSeconds` | `float` | 调试用时间戳 | 是 |

### 4.16 `FRewardEffect`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `RewardType` | `ERewardType` | 奖励通道 | 是 |
| `TargetID` | `FGameplayTag` | 奖励目标 | 是 |
| `Amount` | `int32` | 数量或强度 | 是 |
| `AttributeDelta` | `float` | 属性变化值 | 是 |
| `MessageText` | `FText` | 提示文本 | 是 |
| `UnlockID` | `FGameplayTag` | 解锁标识 | 是 |
| `bAutoApply` | `bool` | 是否立即执行 | 是 |
| `DelaySeconds` | `float` | 延迟时间 | 是 |

### 4.17 `FInterruptPolicy`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `bAllowLeaveRangeInterrupt` | `bool` | 离开范围时是否中断 | 是 |
| `bAllowMenuInterrupt` | `bool` | 打开菜单时是否中断 | 是 |
| `bAllowDeathInterrupt` | `bool` | 死亡时是否中断 | 是 |
| `bAllowMapChangeInterrupt` | `bool` | 切场景时是否中断 | 是 |
| `bAllowCombatInterrupt` | `bool` | 进入战斗时是否中断 | 是 |
| `bCanResume` | `bool` | 中断后是否允许恢复 | 是 |
| `bPreserveNodeProgress` | `bool` | 中断后是否保留当前节点进度 | 是 |

### 4.18 `UDialogueQuestSubsystem`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `ActiveSession` | `FConversationSession` | 当前会话 | 否 |
| `DialogueRuntimeMap` | `TMap<FGameplayTag, FDialogueRuntimeState>` | 对话运行时缓存 | 是 |
| `QuestRuntimeMap` | `TMap<FGameplayTag, FQuestRuntimeState>` | 任务运行时缓存 | 是 |
| `GlobalFlagMap` | `TMap<FGameplayTag, FGlobalFlagState>` | 全局旗标缓存 | 是 |
| `LoadedDialogueDefinitions` | `TMap<FGameplayTag, TObjectPtr<UDialogueDefinition>>` | 已加载对话资产缓存 | 否 |
| `LoadedQuestDefinitions` | `TMap<FGameplayTag, TObjectPtr<UQuestDefinition>>` | 已加载任务资产缓存 | 否 |
| `CurrentSaveGame` | `TObjectPtr<UEscapeDialogueSaveGame>` | 当前存档对象 | 否 |
| `SaveSlotName` | `FString` | 存档槽名 | 是 |
| `UserIndex` | `int32` | 存档用户索引 | 是 |
| `bAutoSaveEnabled` | `bool` | 是否自动存档 | 是 |
| `bConversationOpen` | `bool` | 当前是否正在对话 | 否 |
| `PendingRewardQueue` | `TArray<FRewardEffect>` | 延迟奖励队列 | 否 |

### 4.19 `UEscapeDialogueSaveGame`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `SaveVersion` | `int32` | 存档版本号 | 是 |
| `LastMapName` | `FName` | 上次地图名 | 是 |
| `SavedQuestStates` | `TArray<FQuestRuntimeState>` | 保存的任务进度 | 是 |
| `SavedDialogueStates` | `TArray<FDialogueRuntimeState>` | 保存的对话进度 | 是 |
| `SavedGlobalFlags` | `TArray<FGlobalFlagState>` | 保存的全局旗标 | 是 |
| `SavedWorldEvents` | `TArray<FWorldEventSaveRecord>` | 保存的世界事件 | 是 |
| `SaveTimestamp` | `FDateTime` | 保存时间 | 是 |

### 4.20 `UDialogueWidget`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `SpeakerNameText` | `UTextBlock*` | 说话者名字文本 | 否 |
| `DialogueTextBlock` | `UTextBlock*` | 对白文本 | 否 |
| `PortraitImage` | `UImage*` | 头像控件 | 否 |
| `OptionContainer` | `UVerticalBox*` / `UScrollBox*` | 选项容器 | 否 |
| `QuestHintText` | `UTextBlock*` | 任务提示文本 | 否 |
| `SkipButton` | `UButton*` | 跳过按钮 | 否 |
| `CloseButton` | `UButton*` | 关闭按钮 | 否 |
| `OptionWidgetClass` | `TSubclassOf<UUserWidget>` | 选项行 Widget 类 | 否 |
| `CurrentDialogueID` | `FGameplayTag` | 当前显示的对话 | 否 |
| `CurrentNodeID` | `FGameplayTag` | 当前节点 | 否 |
| `bCanSkip` | `bool` | UI 层是否允许跳过 | 否 |

### 4.21 `FDialogueSaveRecord`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `DialogueID` | `FGameplayTag` | 对话标识 | 是 |
| `CurrentNodeID` | `FGameplayTag` | 当前节点 | 是 |
| `SeenNodeIDs` | `TArray<FGameplayTag>` | 已访问节点 | 是 |
| `VisitedOptionIDs` | `TArray<FGameplayTag>` | 已选择选项 | 是 |
| `bHasSeenDialogue` | `bool` | 是否看过 | 是 |

### 4.22 `FQuestSaveRecord`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `QuestID` | `FGameplayTag` | 任务标识 | 是 |
| `QuestState` | `EQuestState` | 任务状态 | 是 |
| `ObjectiveRecords` | `TArray<FObjectiveSaveRecord>` | 目标进度 | 是 |
| `bRewardClaimed` | `bool` | 是否领过奖励 | 是 |
| `QuestGiverNPC_ID` | `FGameplayTag` | 接任务 NPC | 是 |
| `TurnInNPC_ID` | `FGameplayTag` | 交任务 NPC | 是 |

### 4.23 `FObjectiveSaveRecord`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `ObjectiveID` | `FGameplayTag` | 目标标识 | 是 |
| `CurrentCount` | `int32` | 当前进度 | 是 |
| `TargetCount` | `int32` | 完成数量要求 | 是 |
| `bCompleted` | `bool` | 是否完成 | 是 |
| `SourceIDs` | `TArray<FGameplayTag>` | 进度来源 | 是 |

### 4.24 `FGlobalFlagSaveRecord`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `FlagID` | `FGameplayTag` | 旗标标识 | 是 |
| `bValue` | `bool` | 布尔值 | 是 |
| `NumericValue` | `int32` | 数值值 | 是 |
| `TextValue` | `FText` | 文本值 | 是 |

### 4.25 `FWorldEventSaveRecord`

| 字段 | 类型 | 作用 | 是否进入运行时存档 |
|---|---|---|---|
| `EventID` | `FGameplayTag` | 世界事件标识 | 是 |
| `bTriggered` | `bool` | 是否触发过 | 是 |
| `TriggerCount` | `int32` | 触发次数 | 是 |
| `LastTriggerTimeSeconds` | `float` | 最近触发时间 | 是 |

---

## 5. 运行流程

1. `UInteractComponent` 命中实现 `IInteractableInterface` 的对象。
2. 目标对象把对话请求转给 `UDialogueQuestSubsystem`。
3. 子系统创建 `FConversationSession`。
4. 子系统加载对话树并检查 `FDialogueCondition`。
5. `UDialogueWidget` 显示节点和选项。
6. 玩家选择选项。
7. 子系统执行 `FDialogueEffect`，必要时再执行 `FRewardEffect`。
8. 子系统更新任务、旗标和对话运行时状态。
9. 如需持久化，子系统写入 `UEscapeDialogueSaveGame`。

Boss 开场对白复用同一路径，只是在效果链末尾通常接 `StartBossEncounter`，然后让 Boss AI 切到战斗状态。

---

## 6. 分阶段推进

- [ ] **阶段 1：公共类型与 ID 规范**
- 创建 `EscapeDialogueTypes.h` 中的公共枚举与结构体。
  - 定义最终使用的 `GameplayTag` 命名空间：`Quest.*`、`Dialogue.*`、`Flag.*`、`Encounter.*`、`Reward.*`、`Puzzle.*`。
- [ ] **阶段 2：数据资产**
  - 创建 `UDialogueDefinition` 与 `UQuestDefinition`。
  - 保持节点、选项、目标、奖励都能在 Blueprint / Data Asset 流程中编辑。
- [ ] **阶段 3：运行时主控**
  - 创建 `UDialogueQuestSubsystem`。
  - 实现会话、条件判断、效果执行、运行时 Map。
- [ ] **阶段 4：持久化**
  - 创建 `UEscapeDialogueSaveGame`。
  - 完成运行时记录与存档记录之间的映射。
- [ ] **阶段 5：宿主对象**
  - 创建 `UDialogueParticipantComponent` 和 `ADialogueNPC`。
  - `ADialogueNPC` 只做普通 NPC 包装，Boss 复用组件，不做专门继承。
- [ ] **阶段 6：UI 与输入接线**
  - 创建 `UDialogueWidget`。
  - 复用现有 `UInteractComponent` / `PlayerController` 的开关模式来打开和关闭对话 UI。
- [ ] **阶段 7：Boss 和谜题适配**
  - Boss / 敌人战斗前强制对白走同一条对话管线，由敌人自身的 `UDialogueParticipantComponent` 触发，不继承 `ADialogueNPC`。
  - 谜题完成只作为目标进度变化，不单独开一套系统。

---

## 7. 当前实现状态

当前源码已存在以下对话相关类型，但还没有形成完整运行链路：

| 类型 | 当前状态 | 后续处理 |
|---|---|---|
| `UDialogueDefinition` | 已创建空 `UDataAsset` 类 | 需要补 `DialogueID`、`StartNodeID`、`Nodes` 等配置字段 |
| `ADialogueNPC` | 已创建 Actor，但当前仍混有 NPC 身份、头像、对话资产等字段 | 需要收敛为普通 NPC 包装：默认创建 `UDialogueParticipantComponent`，自身只保留交互文本和普通交互开关，并实现 `IInteractableInterface` |
| `UDialogueParticipantComponent` | 已创建空组件，当前 Tick 开启 | 应关闭 Tick，补参与者 ID、对话资产、起始节点、中断策略、触发模式和战斗前强制对白字段 |
| `UDialogueQuestSubsystem` | 未创建 | 第一版主控逻辑入口 |
| `UQuestDefinition` | 未创建 | 第一版任务配置资产 |
| `UEscapeDialogueSaveGame` | 未创建 | MVP 后再接入 |
| `UDialogueWidget` | 未创建 | 第一版 UI 显示入口 |
| `UInteractComponent` | 已由旧拼写类名重命名 | 对话入口应通过 `IInteractableInterface` 进入，不直接耦合 NPC 类 |

---

## 8. MVP 范围

第一版只做最小闭环，不一次性实现完整任务系统：

- NPC 交互后打开对话 UI。
- `UDialogueDefinition` 支持单段对白、选项、选项跳转和关闭。
- `UDialogueParticipantComponent` 提供对话资产、起始节点、显示信息和触发策略。
- `UDialogueQuestSubsystem` 能创建会话、读取当前节点、筛选可见选项、响应选项点击。
- 支持 `SetGlobalFlag`、`CloseDialogue`、`StartQuest` 三类基础效果。
- 支持 `GlobalFlagIs`、`DialogueNodeSeen` 两类基础条件。
- MVP 优先做普通 NPC 手动对话；战斗前强制对白先写入字段和流程规范，完整 Boss 接入放到后续阶段。
- 暂不做存档、完整 Boss AI 接入、奖励队列、复杂任务目标、谜题目标。

MVP 结束标准：地图中放一个 `ADialogueNPC`，玩家按交互键后能看到对白，选择一个选项后能跳到下一节点或关闭对话，并能设置一个全局 Flag。

---

## 9. GameplayTag 命名规范

Tag 必须稳定、可读、可跨资产引用。推荐命名：

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

规则：

- `DialogueID`、`QuestID`、`NPC_ID`、`EncounterID` 必须全项目唯一。
- 节点和选项 Tag 允许只在所属对话内语义唯一，但仍建议使用完整前缀，方便调试。
- 存档只保存运行时状态 Tag，不保存资产路径作为主键。
- Blueprint / DataAsset 中不使用裸字符串 ID。

---

## 10. Subsystem API 草案

`UDialogueQuestSubsystem` 对外暴露的最小接口：

```cpp
UFUNCTION(BlueprintCallable, Category="Dialogue")
bool StartConversation(AActor* Instigator, UDialogueParticipantComponent* Participant);

UFUNCTION(BlueprintCallable, Category="Dialogue")
bool SelectOption(FGameplayTag OptionID);

UFUNCTION(BlueprintCallable, Category="Dialogue")
bool EndConversation(EInterruptReason Reason);

UFUNCTION(BlueprintPure, Category="Dialogue")
bool IsConversationActive() const;

UFUNCTION(BlueprintPure, Category="Dialogue")
FDialogueNode GetCurrentNode() const;

UFUNCTION(BlueprintPure, Category="Dialogue")
TArray<FDialogueOption> GetAvailableOptions() const;

bool EvaluateCondition(const FDialogueCondition& Condition) const;
void ApplyEffect(const FDialogueEffect& Effect);
void MarkNodeSeen(FGameplayTag DialogueID, FGameplayTag NodeID);
void SetGlobalFlag(FGameplayTag FlagID, bool bValue);
bool GetGlobalFlag(FGameplayTag FlagID) const;
bool StartQuest(FGameplayTag QuestID);
void AddObjectiveProgress(FGameplayTag QuestID, FGameplayTag ObjectiveID, int32 Delta);
bool SaveDialogueQuestState();
bool LoadDialogueQuestState();
```

约束：

- UI 只调用 `SelectOption` / `EndConversation`，不直接修改运行时 Map。
- 交互对象只调用 `StartConversation`，不自己解析对话树。
- `ApplyEffect` 内部必须先验证参数，再修改任务、旗标、背包或遭遇战状态。
- 所有失败分支都要输出 `UE_LOG`，防止 DataAsset 配置错误被静默吞掉。

---

## 11. 现有系统接线点

| 系统 | 接线方式 | 注意事项 |
|---|---|---|
| `UInteractComponent` | Sphere Sweep 命中实现 `IInteractableInterface` 的 Actor 后调用 `Interact(Pawn)` | 只负责发现对象，不解析对话 |
| `ADialogueNPC` | 普通 NPC 实现 `IInteractableInterface`，在 `Interact` 中把请求交给自身 `UDialogueParticipantComponent` 或 Subsystem | 不保存任务进度，不用于敌人 / Boss 继承 |
| `UDialogueParticipantComponent` | 持有 `DialogueAsset`、`DefaultStartNodeID`、显示名、头像、中断策略和触发策略 | 普通 NPC、敌人、Boss、雕像、机关复用该组件 |
| `AEscapeGamePlayerController` | 负责创建/显示 `UDialogueWidget`，切换输入模式 | 可复用背包 UI 的 Game/UI 输入切换习惯 |
| `UInventoryComponent` | 被 `HasItem`、`GiveItem`、`RemoveItem` 条件/效果读取或调用 | Subsystem 不直接操作 UI，只操作组件数据 |
| `UStateMachineComponent` | 对话可按配置锁定玩家移动或拒绝战斗中对话 | 死亡/眩晕状态应拒绝普通对话 |
| Boss / Enemy AI / Blackboard | AI 在进入战斗前检查自身 `UDialogueParticipantComponent`，若 `TriggerMode=ForcedBeforeCombat` 且未播放过，则先请求 Subsystem 开对话 | 对话结束后再通过 `StartBossEncounter` 或 Blackboard Key 切入战斗 |

---

## 12. DataAsset 配置示例

示例：守门人要求玩家找到钥匙并打开主门。

### 12.1 普通 NPC

```text
ADialogueNPC:
  InteractionText: 交谈
  bCanInteract: true
  bAutoStartDialogueOnInteract: true

DialogueParticipantComp:
  ParticipantID: NPC.Village.Gatekeeper
  DisplayName: 守门人
  DialogueAsset: DA_Dialogue_Gatekeeper_Intro
  DefaultStartNodeID: Dialogue.Node.Gatekeeper.Root
  TriggerMode: ManualInteract
  bFaceInstigator: true
  bLockPlayer: true
```

### 12.2 对话节点

```text
NodeID: Dialogue.Node.Gatekeeper.Root
SpeakerID: NPC.Village.Gatekeeper
SpeakerText: 大门被锁住了。没有钥匙，谁也不能过去。
Options:
  - OptionID: Dialogue.Option.Gatekeeper.AskKey
    OptionText: 钥匙在哪里？
    NextNodeID: Dialogue.Node.Gatekeeper.KeyHint
  - OptionID: Dialogue.Option.Gatekeeper.TurnInKey
    OptionText: 我拿到钥匙了。
    Conditions:
      HasItem(Item.Key.MainGate, RequiredCount=1)
    Effects:
      RemoveItem(Item.Key.MainGate, Count=1)
      SetGlobalFlag(Flag.World.MainGateUnlocked, true)
      SetQuestState(Quest.Main.FindGateKey, Completed)
    NextNodeID: Dialogue.Node.Gatekeeper.OpenGate
  - OptionID: Dialogue.Option.Gatekeeper.Leave
    OptionText: 先离开。
    Effects:
      CloseDialogue
```

```text
NodeID: Dialogue.Node.Gatekeeper.KeyHint
SpeakerText: 去旧仓库看看。上一任守卫总把钥匙藏在那里。
OnEnterEffects:
  StartQuest(Quest.Main.FindGateKey)
Options:
  - OptionID: Dialogue.Option.Gatekeeper.Back
    OptionText: 我知道了。
    Effects:
      CloseDialogue
```

### 12.3 任务

```text
QuestID: Quest.Main.FindGateKey
QuestTitle: 找到主门钥匙
QuestDescription: 守门人说主门钥匙可能在旧仓库。
QuestGiverNPC_ID: NPC.Village.Gatekeeper
TurnInNPC_ID: NPC.Village.Gatekeeper
Objectives:
  - ObjectiveID: Quest.Objective.FindGateKey.CollectKey
    ObjectiveType: CollectItem
    TargetID: Item.Key.MainGate
    RequiredCount: 1
Rewards:
  - RewardType: WorldEvent
    TargetID: Flag.World.MainGateUnlocked
    bGrantImmediately: true
```

### 12.4 敌人战斗前强制对白

敌人或 Boss 不继承 `ADialogueNPC`，而是在自身蓝图或 C++ Actor 上挂 `UDialogueParticipantComponent`。

```text
Enemy Actor / Boss Actor:
  DialogueParticipantComp:
    ParticipantID: Enemy.Boss.Gatekeeper
    DisplayName: 门卫队长
    DialogueAsset: DA_Dialogue_Boss_Gatekeeper_Intro
    DefaultStartNodeID: Dialogue.Node.BossGatekeeper.Intro
    TriggerMode: ForcedBeforeCombat
    bForceDialogueBeforeCombat: true
    bOnlyForceOnce: true
    bStartEncounterAfterDialogue: true
    LinkedEncounterID: Encounter.Boss.Gatekeeper
    ForcedDialoguePlayedFlag: Flag.Dialogue.BossGatekeeper.IntroPlayed
    bLockPlayer: true
    bCanSkip: false
```

推荐触发流程：

```text
敌人感知玩家 / 玩家进入战斗触发范围
  -> AI 或感知组件读取 UDialogueParticipantComponent
  -> TriggerMode == ForcedBeforeCombat 且 ForcedDialoguePlayedFlag=false
  -> 暂停 AI 移动和攻击，锁定玩家输入
  -> UDialogueQuestSubsystem::StartConversation(Player, DialogueParticipantComp)
  -> 对话结束后设置 ForcedDialoguePlayedFlag=true
  -> 若 bStartEncounterAfterDialogue=true，则广播 Encounter.Boss.Gatekeeper 或写 Blackboard
  -> AI 进入战斗状态
```

---

## 13. 错误处理与边界规则

- `DialogueAsset` 为空：拒绝开始对话，`UE_LOG(Error)` 输出参与者名称和 ID。
- `DefaultStartNodeID` 不存在：回退到 `UDialogueDefinition::StartNodeID`；仍不存在则结束会话并输出错误。
- 当前节点不存在：结束会话，保留已写入的运行时状态，不执行节点效果。
- 选项不可见但被 UI 点击：拒绝执行，输出 Warning，不改变节点。
- 条件类型未实现：默认返回 false，输出 Warning，避免误放行。
- 效果类型未实现：不执行，输出 Warning，但不中断后续效果链；关键效果可后续增加 `bStopOnFailure`。
- 任务重复领取：如果任务是 `Active` / `ReadyToTurnIn` / `Completed` 且 `bCanRepeat=false`，拒绝 `StartQuest`。
- 背包空间不足：`GiveItem` 失败时不推进任务状态，并把失败原因传给 UI 提示。
- 玩家死亡、切图、进入战斗：根据 `FInterruptPolicy` 调用 `EndConversation`，必要时保留 `CurrentNodeID`。
- 存档版本不一致：低版本尝试兼容读取，高版本拒绝读取并输出 Error。
- 对话过程中对象销毁：Subsystem 使用 `TWeakObjectPtr` 或 `IsValid` 检查参与者，失效则中断会话。

---

## 14. 测试与验收用例

| 用例 | 前置条件 | 操作 | 期望结果 |
|---|---|---|---|
| 普通对话打开关闭 | NPC 配置有效 `DialogueAsset` | 玩家按交互键 | 创建会话并显示 Root 节点 |
| 选项跳转 | Root 节点存在 `NextNodeID` | 点击选项 | 当前节点切到目标节点 |
| 关闭对话 | 选项包含 `CloseDialogue` 效果 | 点击离开选项 | 会话结束，UI 关闭 |
| 全局旗标条件 | `Flag.World.MainGateUnlocked=false` | 查看需要 true 的选项 | 选项隐藏或禁用 |
| 设置全局旗标 | 选项包含 `SetGlobalFlag` | 点击选项 | `GlobalFlagMap` 中对应值变为 true |
| 接任务 | 选项包含 `StartQuest` | 点击选项 | `QuestRuntimeMap[QuestID].QuestState=Active` |
| 任务重复领取 | 任务已 Active 且不可重复 | 再次执行 `StartQuest` | 返回 false 并输出 Warning |
| 物品不足交任务 | 背包没有目标物品 | 查看交任务选项 | 选项不可用，并显示 `FailureHint` |
| 对话资产为空 | Participant 没有资产 | 按交互键 | 不打开 UI，输出 Error |
| 中断策略 | 对话中进入战斗 | 触发 CombatStart | 按策略关闭或保留当前节点 |
| 读档恢复 | 保存后重启关卡 | 调用 Load | 对话节点、任务状态、全局旗标恢复 |
| Boss 开场 | Boss 组件配置 `LinkedEncounterID` | 对话结束执行 `StartBossEncounter` | Boss AI 切入战斗状态 |

---

## 15. 验收标准

- 普通 NPC 可以打开对话、显示选项、接任务和交任务。
- Boss 可以播放开场对白，并通过对话效果切入战斗。
- 任务进度能够跨关卡并且可以读档恢复。
- 对话与任务状态不会存放在 NPC Actor 本体上。
- UI 只负责显示，不直接承担业务逻辑。
