# Dialogue Prototype Verification Plan

最后更新：2026-07-02

本文档记录 Dialogue 原型从“能对话”推进到“能接任务、推进目标、发奖励、预留人物表现”的验证顺序。它不是完整实现方案，而是每个切片要验证什么、需要哪些配置数据、成功标准是什么。

---

## 1. 总原则

当前阶段只验证小闭环，不一次性实现完整任务系统。

推进顺序：

```text
最小 Dialogue UI
  -> SetGlobalFlag / CloseDialogue 真实资产验证
  -> StartQuest
  -> QuestStateIs
  -> AddObjectiveProgress
  -> GiveItem / GrantReward
  -> OnEnterEffects
  -> SaveGame 映射
  -> 人物表现层 Expression / Gesture / FacePlayer
```

每个切片只允许有一个核心成功标准。失败时先定位当前切片，不扩散到后续系统。

---

## 2. 已配置 Native GameplayTags

### 2.1 最小 NPC01 对话

```text
NPC.NPC01

Dialogue.NPC01.Intro
Dialogue.Node.NPC01.Root
Dialogue.Node.NPC01.Danger

Dialogue.Option.NPC01.AskDanger
Dialogue.Option.NPC01.Back
Dialogue.Option.NPC01.Leave
Dialogue.Option.NPC01.LeaveAfterHint
```

用途：
- 验证 NPC 交互打开 UI。
- 验证 Root -> Danger 节点跳转。
- 验证 Leave 关闭对话。

### 2.2 NPC01 任务测试

```text
Dialogue.NPC01.QuestTest
Dialogue.Node.NPC01.QuestOffer
Dialogue.Node.NPC01.QuestAccepted
Dialogue.Node.NPC01.QuestInProgress
Dialogue.Node.NPC01.QuestComplete

Dialogue.Option.NPC01.AcceptQuest
Dialogue.Option.NPC01.DeclineQuest
Dialogue.Option.NPC01.AskProgress
Dialogue.Option.NPC01.TurnInQuest

Quest.NPC01.FindTestKey
Quest.Objective.NPC01.CollectTestKey
Item.Key.NPC01.TestKey
Reward.Item.NPC01.TestKey
Reward.Message.NPC01.Thanks

Flag.Dialogue.NPC01.IntroSeen
Flag.Quest.NPC01.Accepted
```

用途：
- 验证 `StartQuest` effect。
- 验证 `QuestStateIs` condition。
- 验证 `AddObjectiveProgress` effect。
- 验证 `GiveItem` 或 `GrantReward` 最小奖励路径。

### 2.3 人物表现预留

```text
Character.Expression.Neutral
Character.Expression.Happy
Character.Expression.Worried

Character.Gesture.Idle
Character.Gesture.Talk
Character.Gesture.Point

Character.Focus.Player
```

用途：
- 后续节点级表情、手势、朝向玩家验证。
- 当前不接入运行时逻辑，只作为后续配置 ID。

---

## 3. 验证切片 A：最小 Dialogue UI

### 目标

地图中的 `BP_NPC01` 可以被玩家交互，打开 `WBP_Dialogue`，显示节点文本和选项，点击后跳转或关闭。

### 推荐配置

`BP_NPC01.DialogueParticipantComp`：

```text
ParticipantID = NPC.NPC01
DialogueDefinition = DA_Dialogue_NPC01_Intro
DefaultStartNodeID = 留空
bCanTalk = true
TriggerMode = ManualInteract
bLockPlayer = true
bCanSkip = true
```

`DA_Dialogue_NPC01_Intro`：

```text
DialogueID = Dialogue.NPC01.Intro
StartNodeID = Dialogue.Node.NPC01.Root
```

Root Node：

```text
NodeID = Dialogue.Node.NPC01.Root
SpeakerID = NPC.NPC01
SpeakerText = 你好，旅行者。这里现在还不太安全。

Option 1:
  OptionID = Dialogue.Option.NPC01.AskDanger
  OptionText = 这里发生了什么？
  NextNodeID = Dialogue.Node.NPC01.Danger

Option 2:
  OptionID = Dialogue.Option.NPC01.Leave
  OptionText = 我先走了。
  bCloseDialogueAfterSelected = true
```

Danger Node：

```text
NodeID = Dialogue.Node.NPC01.Danger
SpeakerID = NPC.NPC01
SpeakerText = 前面的门被锁住了，也许你需要先找到钥匙。

Option 1:
  OptionID = Dialogue.Option.NPC01.Back
  OptionText = 我明白了。
  NextNodeID = Dialogue.Node.NPC01.Root

Option 2:
  OptionID = Dialogue.Option.NPC01.LeaveAfterHint
  OptionText = 谢谢，我去看看。
  bCloseDialogueAfterSelected = true
```

### 成功标准

```text
按交互键 -> UI 打开
点击“这里发生了什么？” -> 文本切到 Danger Node
点击“谢谢，我去看看。” -> UI 关闭，输入恢复
```

---

## 4. 验证切片 B：SetGlobalFlag / CloseDialogue 真实资产验证

### 目标

不用 Automation Test，直接用真实 Dialogue DataAsset 验证 effect 链路。

### 推荐配置

在某个 Option 上添加：

```text
EffectType = SetGlobalFlag
FlagID = Flag.Dialogue.NPC01.IntroSeen
BoolValue = true
```

关闭对话可继续使用：

```text
bCloseDialogueAfterSelected = true
```

或单独验证：

```text
EffectType = CloseDialogue
```

### 成功标准

```text
点击选项后对话仍按预期跳转或关闭
后续带 GlobalFlagIs 条件的选项能按 Flag 状态显示
```

---

## 5. 验证切片 C：StartQuest

### 目标

点击接受任务选项后，`QuestRuntimeMap[Quest.NPC01.FindTestKey]` 进入 `Active`。

### 推荐配置

`DA_Dialogue_NPC01_QuestTest`：

```text
DialogueID = Dialogue.NPC01.QuestTest
StartNodeID = Dialogue.Node.NPC01.QuestOffer
```

Accept Option：

```text
OptionID = Dialogue.Option.NPC01.AcceptQuest
OptionText = 我可以帮你找钥匙。

Effect:
  EffectType = StartQuest
  QuestID = Quest.NPC01.FindTestKey

NextNodeID = Dialogue.Node.NPC01.QuestAccepted
```

### 成功标准

```text
点击接受任务 -> SelectOption 返回 true
QuestRuntimeMap 中出现 Quest.NPC01.FindTestKey
QuestState = Active
```

暂时不要求任务 UI 显示。

---

## 6. 验证切片 D：QuestStateIs

### 目标

让选项根据任务状态出现或隐藏。

### 推荐配置

`AskProgress` 选项：

```text
OptionID = Dialogue.Option.NPC01.AskProgress
OptionText = 钥匙的事我还在找。

Condition:
  ConditionType = QuestStateIs
  QuestID = Quest.NPC01.FindTestKey
  ExpectedQuestState = Active
```

### 成功标准

```text
任务未开始 -> AskProgress 不显示
任务 Active -> AskProgress 显示
```

---

## 7. 验证切片 E：AddObjectiveProgress

### 目标

通过对话或临时交互推进 `Quest.Objective.NPC01.CollectTestKey`。

### 推荐配置

临时 Option：

```text
OptionID = Dialogue.Option.NPC01.AskProgress
OptionText = 我找到了测试钥匙。

Effect:
  EffectType = AddObjectiveProgress
  QuestID = Quest.NPC01.FindTestKey
  ObjectiveID = Quest.Objective.NPC01.CollectTestKey
  Count = 1
```

### 成功标准

```text
ObjectiveRuntimeState.CurrentCount 增加
达到 TargetCount 后 bCompleted = true
必要时 QuestState 进入 ReadyToTurnIn
```

---

## 8. 验证切片 F：GiveItem / GrantReward

### 目标

完成任务后通过对话发放最小奖励。

### 推荐配置

TurnIn Option：

```text
OptionID = Dialogue.Option.NPC01.TurnInQuest
OptionText = 我找到了钥匙。

Condition:
  ConditionType = QuestStateIs
  QuestID = Quest.NPC01.FindTestKey
  ExpectedQuestState = ReadyToTurnIn

Effect:
  EffectType = GiveItem 或 GrantReward
  ItemID / TargetID = Item.Key.NPC01.TestKey 或 Reward.Item.NPC01.TestKey
  Count = 1
```

### 成功标准

```text
任务可交付时显示 TurnInQuest
点击后玩家获得奖励，或 PendingRewardQueue 出现奖励记录
任务状态进入 Completed / RewardClaimed
```

---

## 9. 验证切片 G：人物表现预留

### 目标

后续让对话节点能够驱动人物表现，但当前不实现。

### 可能的数据设计方向

方案 A：节点字段

```text
FDialogueNode.ExpressionTag = Character.Expression.Worried
FDialogueNode.GestureTag = Character.Gesture.Talk
```

方案 B：Effect

```text
EffectType = SetExpression
ExpectedTagValue = Character.Expression.Happy
```

当前建议：

```text
先不加字段，不实现逻辑。
等最小任务奖励闭环通过后，再设计人物表现层。
```

---

## 10. 当前停止线

当前只允许推进到真实地图里的最小 Dialogue UI 验证。

不要提前实现：

```text
完整任务 UI
完整背包奖励
复杂 SaveGame
表情系统
语音系统
对话镜头系统
AI 移动 / 寻路
```

下一步实际动作：

```text
1. 编译 Native GameplayTags
2. 重启 Editor
3. 创建 DA_Dialogue_NPC01_Intro
4. 配置 BP_NPC01.DialogueParticipantComp
5. 在地图中按交互键验证最小 UI 闭环
```
