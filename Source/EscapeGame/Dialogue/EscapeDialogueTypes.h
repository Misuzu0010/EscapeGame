// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "EscapeDialogueTypes.generated.h"

UENUM(BlueprintType)
enum class EDialogueNodeType : uint8
{
	Line UMETA(DisplayName="普通对白节点"),
	Choice UMETA(DisplayName="玩家选项节点"),
	Reward UMETA(DisplayName="奖励发放节点"),
	Exit UMETA(DisplayName="结束对话节点"),
	Branch UMETA(DisplayName="条件分支节点"),
	BossIntro UMETA(DisplayName="Boss 开场对白节点")
};

UENUM(BlueprintType)
enum class EDialogueTriggerMode : uint8
{
	ManualInteract UMETA(DisplayName="手动交互触发"),
	AutoOnOverlap UMETA(DisplayName="进入范围自动触发"),
	ForcedBeforeCombat UMETA(DisplayName="战斗前强制触发"),
	BossIntro UMETA(DisplayName="Boss 开场触发"),
	ScriptedOnly UMETA(DisplayName="仅脚本触发")
};

UENUM(BlueprintType)
enum class EDialogueConditionType : uint8
{
	QuestStateIs UMETA(DisplayName="任务状态满足"),
	ObjectiveCompleted UMETA(DisplayName="任务目标已完成"),
	HasItem UMETA(DisplayName="持有指定物品"),
	GlobalFlagIs UMETA(DisplayName="全局旗标满足"),
	DialogueNodeSeen UMETA(DisplayName="已看过指定对白节点"),
	PuzzleSolved UMETA(DisplayName="谜题已解开"),
	BossStateIs UMETA(DisplayName="Boss 状态满足")
};

UENUM(BlueprintType)
enum class EConditionCompareMode : uint8
{
	Equal UMETA(DisplayName="等于"),
	NotEqual UMETA(DisplayName="不等于"),
	GreaterOrEqual UMETA(DisplayName="大于或等于"),
	LessOrEqual UMETA(DisplayName="小于或等于"),
	Contains UMETA(DisplayName="包含"),
	Exists UMETA(DisplayName="存在")
};

UENUM(BlueprintType)
enum class EDialogueEffectType : uint8
{
	StartQuest UMETA(DisplayName="开始任务"),
	SetQuestState UMETA(DisplayName="设置任务状态"),
	AddObjectiveProgress UMETA(DisplayName="增加目标进度"),
	SetGlobalFlag UMETA(DisplayName="设置全局旗标"),
	GiveItem UMETA(DisplayName="给予物品"),
	RemoveItem UMETA(DisplayName="移除物品"),
	GrantReward UMETA(DisplayName="发放奖励"),
	StartBossEncounter UMETA(DisplayName="启动 Boss 遭遇战"),
	BroadcastWorldEvent UMETA(DisplayName="广播世界事件"),
	CloseDialogue UMETA(DisplayName="关闭对话"),
	SaveGame UMETA(DisplayName="保存游戏")
};

UENUM(BlueprintType)
enum class EQuestState : uint8
{
	NotStarted UMETA(DisplayName="未开始"),
	Active UMETA(DisplayName="进行中"),
	ReadyToTurnIn UMETA(DisplayName="可交付"),
	Completed UMETA(DisplayName="已完成"),
	RewardClaimed UMETA(DisplayName="奖励已领取"),
	Failed UMETA(DisplayName="已失败")
};

UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
	TalkToNPC UMETA(DisplayName="与 NPC 对话"),
	CollectItem UMETA(DisplayName="收集物品"),
	UseItem UMETA(DisplayName="使用物品"),
	SolvePuzzle UMETA(DisplayName="解开谜题"),
	KillEnemy UMETA(DisplayName="击杀敌人"),
	ReachLocation UMETA(DisplayName="到达地点"),
	TriggerWorldEvent UMETA(DisplayName="触发世界事件")
};

UENUM(BlueprintType)
enum class ERewardType : uint8
{
	Message UMETA(DisplayName="提示消息"),
	Item UMETA(DisplayName="物品奖励"),
	Attribute UMETA(DisplayName="属性变化"),
	WorldEvent UMETA(DisplayName="世界事件"),
	Unlock UMETA(DisplayName="解锁内容")
};

UENUM(BlueprintType)
enum class EInterruptReason : uint8
{
	LeaveRange UMETA(DisplayName="离开对话范围"),
	OpenMenu UMETA(DisplayName="打开菜单"),
	Death UMETA(DisplayName="角色死亡"),
	MapChange UMETA(DisplayName="切换地图"),
	CombatStart UMETA(DisplayName="进入战斗"),
	HigherPriorityCutscene UMETA(DisplayName="更高优先级演出"),
	NormalEnd UMETA(DisplayName="正常结束对话")
};

// 一段对话遇到外部事件时该怎么处理。
// 例：玩家走远、打开菜单、死亡、切图、进入战斗时，是继续对话，还是立刻结束/暂停。
USTRUCT(BlueprintType)
struct ESCAPEGAME_API FInterruptPolicy
{
	GENERATED_BODY()

	// true：玩家走出可交互范围后结束或暂停对话；false：即使走远也继续播放。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bAllowLeaveRangeInterrupt;

	// true：打开背包、暂停菜单等界面时中断对话；false：菜单打开后对话状态仍保留。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bAllowMenuInterrupt;

	// true：玩家、NPC 或关键敌人死亡时中断对话；false：死亡事件不主动打断对话。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bAllowDeathInterrupt;

	// true：切换地图、传送或卸载关卡时中断对话；通常建议保持 true。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bAllowMapChangeInterrupt;

	// true：进入战斗时中断普通对话；战斗前强制对白通常会在结束后再进入战斗。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bAllowCombatInterrupt;

	// true：中断后下次可以从 CurrentNodeID 附近继续；false：下次重新从起点或默认节点开始。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bCanResume;

	// true：中断时记录当前节点；false：中断时不保留当前位置。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bPreserveNodeProgress;
};

// 一条“是否满足”的判断规则。
// 可挂在节点、选项、任务前置条件上，例如“任务 A 已完成”“有钥匙”“Boss 没死”。
USTRUCT(BlueprintType)
struct ESCAPEGAME_API FDialogueCondition
{
	GENERATED_BODY()

	// 要检查哪一类数据。选了 HasItem 就看 ItemID，选了 QuestStateIs 就看 QuestID 和 ExpectedQuestState。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	EDialogueConditionType ConditionType = EDialogueConditionType::QuestStateIs;

	// 怎么比较。常用 Equal；检查数量时可用 GreaterOrEqual；检查列表/集合时可用 Contains 或 Exists。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	EConditionCompareMode ComparisonMode = EConditionCompareMode::Equal;

	// 要检查的任务。例：Quest.Main.FindKey。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag QuestID;

	// 要检查的任务目标。例：Objective.FindKey.CollectKey。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag ObjectiveID;

	// 要检查的物品。例：Item.Key.Basement。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag ItemID;

	// 要检查的全局开关。例：Flag.Door.BasementOpened。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag FlagID;

	// 要检查的战斗、Boss 或遭遇事件。例：Encounter.Boss.Butcher。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag EncounterID;

	// 当 ConditionType 是 QuestStateIs 时，任务需要处于这个状态才算满足。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	EQuestState ExpectedQuestState = EQuestState::NotStarted;

	// 当检查开关类条件时填这里。例：Flag.Door.Opened 必须是 true 才显示某个选项。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool ExpectedBoolValue = false;

	// 当检查数字类条件时填这里。例：警戒等级 >= 2、谜题阶段 == 3。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	int32 ExpectedIntValue = 0;

	// 当某个状态本身也是 GameplayTag 时，用这个值做比较。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag ExpectedTagValue;

	// 需要达到的数量。例：至少收集 3 个碎片时填 3。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	int32 RequiredCount = 0;

	// true：把判断结果反过来。例：“没有钥匙”可以写成 HasItem + bInvert=true。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bInvert = false;

	// 条件不满足时显示给玩家或写入日志的解释。例：“你还没有地下室钥匙”。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FText FailureHint;
};

// 一条“要做什么”的规则。
// 可挂在进入节点时，或玩家点击选项后，例如开始任务、给物品、设置旗标、开启 Boss 战。
USTRUCT(BlueprintType)
struct ESCAPEGAME_API FDialogueEffect
{
	GENERATED_BODY()

	// 要执行的动作类型。选了 GiveItem 就主要看 ItemID 和 Count；选了 SetGlobalFlag 就看 FlagID 和 BoolValue。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	EDialogueEffectType EffectType = EDialogueEffectType::CloseDialogue;

	// 要开始、修改或完成的任务。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag QuestID;

	// 要推进的任务目标。例：给 Objective.FindKey.CollectKey 增加进度。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag ObjectiveID;

	// 要给予或移除的物品。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag ItemID;

	// 要设置的全局开关。例：Flag.NPC.OldManTalked。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag FlagID;

	// 要启动或通知的战斗/Boss/遭遇事件。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag EncounterID;

	// 数量。给物品时表示数量；推进目标时表示增加多少进度。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	int32 Count = 0;

	// true/false 参数。设置全局旗标时通常写入这里。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool BoolValue = false;

	// 文本参数。可用作提示文案、任务变化原因或调试说明。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FText TextValue;

	// 小数参数。用于属性增减、持续时间、倍率等需要 float 的效果。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	float FloatValue = 0.0f;

	// 延迟多少秒后执行。0 表示立刻执行。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	float DelaySeconds = 0.0f;

	// true：执行完这个效果马上写存档。适合不可反悔的选择、发奖励、Boss 战前对白已播放等关键状态。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bCommitSaveImmediately = false;
};

// 玩家能点的一条对话选项。
// 它可以有显示条件、点击后效果，以及跳到哪个下一个节点。
USTRUCT(BlueprintType)
struct ESCAPEGAME_API FDialogueOption
{
	GENERATED_BODY()

	// 选项 ID。用于记录玩家是否点过这个选项，建议在同一段对话内唯一。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag OptionID;

	// 显示在 UI 上的选项文字。例：“我接受这个任务”。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FText OptionText;

	// 选项出现或可点击前必须满足的条件。例：有钥匙才出现“交出钥匙”。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TArray<FDialogueCondition> Conditions;

	// 点击选项后执行的动作。例：开始任务、设置旗标、给奖励。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TArray<FDialogueEffect> Effects;

	// 点击后进入的下一个节点。为空时通常表示不跳转，由效果或关闭逻辑决定。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag NextNodeID;

	// true：点完这个选项后关闭对话窗口。适合“再见”“离开”等选项。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bCloseDialogueAfterSelected = false;

	// true：条件不满足就完全不显示；false：可显示为灰色或显示失败提示。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bHideWhenUnavailable = false;

	// UI 排序。数值小的排在前面；相同值时按数组顺序处理。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	int32 SortOrder = 0;
};

// 对话树里的一个节点。
// 可以是一句台词、一个选项页、一个分支点、一个奖励点，或结束点。
USTRUCT(BlueprintType)
struct ESCAPEGAME_API FDialogueNode
{
	GENERATED_BODY()

	// 节点 ID。其他节点或选项会通过这个 ID 跳转到这里。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag NodeID;

	// 谁在说这句话。填对应参与者的 ParticipantID；旁白也可以约定一个 Narrator.* 标签。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag SpeakerID;

	// 当前节点显示的正文台词。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FText SpeakerText;

	// 节点用途。普通对白用 Line；只有选项可选时用 Choice；结束对话用 Exit。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	EDialogueNodeType NodeType = EDialogueNodeType::Line;

	// 进入这个节点前要满足的条件。条件不满足时，Subsystem 应跳过、走备用分支或结束。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TArray<FDialogueCondition> Conditions;

	// 这个节点上显示的玩家选项。普通台词节点可以为空。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TArray<FDialogueOption> Options;

	// 一进入这个节点就执行的效果。例：标记“已听过警告”、播放后开启 Boss 战。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TArray<FDialogueEffect> OnEnterEffects;

	// 这个节点自己的中断规则。用于让关键台词不可被菜单/战斗打断。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FInterruptPolicy InterruptPolicy;

	// true：玩家可以跳过这句台词；false：必须完整播放或等待脚本推进。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bAllowSkip = true;
};

// 任务里的一个目标配置。
// 它描述“玩家要做什么、做几次、做到哪个对象身上”。
USTRUCT(BlueprintType)
struct ESCAPEGAME_API FQuestObjectiveDefinition
{
	GENERATED_BODY()

	// 这个目标自己的名字。例：Objective.FindKey.TalkToOldMan；同一任务内不要重复。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag ObjectiveID;

	// 目标类型。决定进度从哪里来：对话、收集、使用、解谜、击杀、到达地点等。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	EObjectiveType ObjectiveType = EObjectiveType::TalkToNPC;

	// 目标对象 ID。例：要杀的敌人类型、要收集的物品、要对话的 NPC。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag TargetID;

	// 需要完成几次。例：击杀 3 个敌人填 3；和某个 NPC 对话一次填 1。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	int32 RequiredCount = 1;

	// 任务列表里显示的目标文字。例：“找到地下室钥匙”。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FText Description;

	// 玩家卡住时显示的提示。例：“钥匙可能在厨房附近”。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FText HintText;

	// 额外完成条件。例：收集钥匙后，还要求某个谜题已经解开。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TArray<FDialogueCondition> Conditions;

	// true：这个目标不影响主任务完成，但可用于额外奖励或分支。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bOptional = false;
};

// 任务完成后能给玩家的奖励配置。
USTRUCT(BlueprintType)
struct ESCAPEGAME_API FQuestRewardDefinition
{
	GENERATED_BODY()

	// 奖励类型。决定 TargetID、Count、AttributeDelta 等字段怎么解释。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	ERewardType RewardType = ERewardType::Message;

	// 奖励作用到谁身上。给物品时填 Item.*；改属性时填 Attribute.*；触发事件时填 WorldEvent.*。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag TargetID;

	// 奖励数量。例：给 3 个草药就填 3；不是数量型奖励时可以保持 0。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	int32 Count = 0;

	// 属性变化量。例：生命上限 +10，填 10。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	float AttributeDelta = 0.0f;

	// 要解锁的内容 ID。例：Unlock.Door.Basement、Unlock.Shop.Blacksmith。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag UnlockID;

	// 发奖励时显示给玩家的文字。例：“获得地下室钥匙”。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FText MessageText;

	// true：任务完成时直接给；false：等交任务、结算界面或脚本手动发。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bGrantImmediately = true;
};

// 当前正在进行的一次对话。
// 它只存在于运行时，用来告诉系统“现在谁在和谁聊、聊到哪一句、能不能跳过”。
USTRUCT(BlueprintType)
struct ESCAPEGAME_API FConversationSession
{
	GENERATED_BODY()

	// 本次对话的临时 ID。每次开始对话都可以生成一个新的，方便调试和区分多次会话。
	UPROPERTY(Transient)
	FGuid SessionId;

	// 当前交谈对象的 ID。普通 NPC、Boss、机关都可以作为对话对象。
	UPROPERTY(Transient)
	FGameplayTag NPC_ID;

	// 当前正在播放哪段对话。读它可以知道 UI 现在属于哪个 UDialogueDefinition。
	UPROPERTY(Transient)
	FGameplayTag DialogueID;

	// 当前显示到哪个节点。UI 刷新、跳转、恢复都看它。
	UPROPERTY(Transient)
	FGameplayTag CurrentNodeID;

	// 玩家刚刚点过的选项 ID。可用于调试，也可用于后续分支判断。
	UPROPERTY(Transient)
	FGameplayTag LastSelectedOptionID;

	// true：现在有一段对话正在打开；false：没有正在进行的对话。
	UPROPERTY(Transient)
	bool bIsActive = false;

	// true：对话期间锁住玩家移动/输入；false：玩家可以边走边听。
	UPROPERTY(Transient)
	bool bLockPlayer = false;

	// true：当前这段对话允许玩家跳过文本或演出。
	UPROPERTY(Transient)
	bool bCanSkip = false;

	// 本次对话采用的中断规则，通常来自对话资产、节点或参与者组件。
	UPROPERTY(Transient)
	FInterruptPolicy InterruptPolicy;

	// 对话开始的世界时间，主要用于调试、统计时长或处理超时。
	UPROPERTY(Transient)
	float StartTimeSeconds = 0.0f;

	// 如果对话被中断，这里记录原因，方便恢复、日志和 UI 提示。
	UPROPERTY(Transient)
	EInterruptReason InterruptReason = EInterruptReason::LeaveRange;
};

// 某段对话的长期进度。
// 它回答“这段对话玩家看过没有、看过哪些节点、点过哪些选项”。
USTRUCT(BlueprintType)
struct ESCAPEGAME_API FDialogueRuntimeState
{
	GENERATED_BODY()

	// 对话资产 ID，用来把这份进度对应回具体的 UDialogueDefinition。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag DialogueID;

	// 最近到达的节点。需要恢复对话时可以从这里继续。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag CurrentNodeID;

	// true：玩家至少进入过这段对话一次。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bHasSeenDialogue = false;

	// 玩家已经看过的节点集合。可用于隐藏重复说明或解锁“你之前说过...”分支。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TSet<FGameplayTag> SeenNodeIDs;

	// 玩家已经点过的选项集合。例：点过“撒谎”后，之后 NPC 可以追问这次选择。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TSet<FGameplayTag> VisitedOptionIDs;

	// 只属于这段对话内部的小标记。全局影响更大的状态应放到 FGlobalFlagState。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TSet<FGameplayTag> BranchFlags;

	// 最近一次与这段对话关联的说话对象。多 NPC 共用同一段对话时有用。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag LastTalkPartnerID;
};

// 某个任务目标的当前进度。
// 例：击杀 3 个敌人，目前杀了 1 个；收集 5 个碎片，目前收了 4 个。
USTRUCT(BlueprintType)
struct ESCAPEGAME_API FObjectiveRuntimeState
{
	GENERATED_BODY()

	// 对应任务定义里的 ObjectiveID，用它把运行时进度和静态目标配置对上。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag ObjectiveID;

	// 当前完成了多少。击杀、收集、触发次数都会累加到这里。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	int32 CurrentCount = 0;

	// 完成目标需要多少。通常来自任务目标定义里的 RequiredCount。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	int32 TargetCount = 0;

	// true：该目标已经完成。通常 CurrentCount >= TargetCount 后设为 true。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bCompleted = false;

	// 已经给这个目标加过进度的来源。可防止同一个物品、同一个机关重复刷进度。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TSet<FGameplayTag> SourceIDs;

	// 最近一次进度变化的世界时间，主要用于调试和日志。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	float LastProgressTimeSeconds = 0.0f;
};

// 某个任务的当前状态。
// 它不是任务配置，而是玩家当前这份存档里的任务进度。
USTRUCT(BlueprintType)
struct ESCAPEGAME_API FQuestRuntimeState
{
	GENERATED_BODY()

	// 对应任务资产里的 QuestID，用它把这份进度归到哪一个任务。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag QuestID;

	// 当前任务处于未开始、进行中、可交付、已完成等哪个阶段。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	EQuestState QuestState = EQuestState::NotStarted;

	// 这个任务下每个目标的进度表。Key 是 ObjectiveID，Value 是该目标当前做到多少。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TMap<FGameplayTag, FObjectiveRuntimeState> ObjectiveStates;

	// true：奖励已经发过，避免读档或重复交任务时再次发奖励。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bRewardClaimed = false;

	// 当时从哪个 NPC 接到任务。可用于日志、回访或任务 UI 显示。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag QuestGiverNPC_ID;

	// 任务应该交给哪个 NPC。为空时可由任务定义或脚本决定。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag TurnInNPC_ID;

	// 任务开始的世界时间。主要用于统计、调试或限时任务扩展。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	float StartedTimeSeconds = 0.0f;

	// 最近一次改变任务状态/进度的原因。例：“选择了接受任务选项”。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FText LastUpdateReason;
};

// 全局旗标。
// 用来记录跨对话、跨任务都会关心的世界状态，例如门开了、Boss 死了、某个机关启动了。
USTRUCT(BlueprintType)
struct ESCAPEGAME_API FGlobalFlagState
{
	GENERATED_BODY()

	// 旗标 ID。例：Flag.Door.BasementOpened。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag FlagID;

	// 最常用的开关值。例：门是否打开、是否见过某个 NPC。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bValue = false;

	// 数值状态。例：机关阶段、警戒等级、谜题进度。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	int32 NumericValue = 0;

	// 文本状态。少用，适合记录玩家输入、调查结果名称等。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FText TextValue;

	// 最近一次改动这个旗标的世界时间，主要用于调试和存档排查。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	float LastChangedTimeSeconds = 0.0f;
};

// 运行时奖励。
// 和任务奖励定义不同，它表示“已经准备要发出去的一份奖励”，可进入延迟奖励队列。
USTRUCT(BlueprintType)
struct ESCAPEGAME_API FRewardEffect
{
	GENERATED_BODY()

	// 这份奖励要走哪条发放逻辑。Item 看 TargetID/Amount，Attribute 看 TargetID/AttributeDelta，Unlock 看 UnlockID。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	ERewardType RewardType = ERewardType::Message;

	// 奖励作用对象。例：Item.Key.Basement、Attribute.MaxHealth、WorldEvent.OpenGate。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag TargetID;

	// 奖励数量。给物品时表示给几个；触发世界事件或消息奖励时通常保持 0。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	int32 Amount = 0;

	// 属性变化量。例：生命上限 +10、理智值 -5。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	float AttributeDelta = 0.0f;

	// 奖励发放后给玩家看的提示。例：“获得地下室钥匙 x1”。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FText MessageText;

	// 要解锁的内容。例：Unlock.Area.Basement、Unlock.Dialogue.OldMan.Secret。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag UnlockID;

	// true：进入奖励队列后自动发放；false：等待 UI、剧情或脚本确认后再发。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bAutoApply = true;

	// 延迟多少秒发放。适合等对白播完、动画结束后再给奖励。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	float DelaySeconds = 0.0f;
};

// 对话存档记录。
// 它是 FDialogueRuntimeState 的存档版：运行时可以用 Set/Map，真正写入 USaveGame 时转成数组更稳。
USTRUCT(BlueprintType)
struct ESCAPEGAME_API FDialogueSaveRecord
{
	GENERATED_BODY()

	// 这条记录属于哪一段对话。读档时用它找到对应的对话进度。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag DialogueID;

	// 保存时停在哪个节点。读档恢复时可以用它定位。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag CurrentNodeID;

	// 已经看过的节点列表。读档后转回 SeenNodeIDs，避免重复播放“一次性说明”。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TArray<FGameplayTag> SeenNodeIDs;

	// 已经点过的选项列表。读档后可继续隐藏一次性选项，或开启“你刚才选择了...”分支。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TArray<FGameplayTag> VisitedOptionIDs;

	// true：这段对话至少被打开过一次。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bHasSeenDialogue = false;
};

// 单个任务目标的存档记录。
// 读档后用它还原“这个目标做到多少了、哪些来源已经计数过”。
USTRUCT(BlueprintType)
struct ESCAPEGAME_API FObjectiveSaveRecord
{
	GENERATED_BODY()

	// 这条记录属于哪个目标。读档时用它匹配任务定义里的 ObjectiveID。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag ObjectiveID;

	// 存档时已经完成的数量。例：杀了 2/3 个敌人，这里就是 2。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	int32 CurrentCount = 0;

	// 完成该目标需要的数量。保存它可以避免任务定义变动后旧存档完全丢失上下文。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	int32 TargetCount = 0;

	// true：目标已经完成。读档后不应再因为同一事件重复完成一次。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bCompleted = false;

	// 已经计入过进度的来源列表。例：宝箱 A 已经算过一次，读档后再打开不应重复加进度。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TArray<FGameplayTag> SourceIDs;
};

// 单个任务的存档记录。
// 读档后用它恢复任务状态、目标进度、接任务/交任务对象和奖励领取状态。
USTRUCT(BlueprintType)
struct ESCAPEGAME_API FQuestSaveRecord
{
	GENERATED_BODY()

	// 这条记录属于哪个任务。读档时用它匹配 UQuestDefinition 的 QuestID。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag QuestID;

	// 存档时任务处于哪个阶段。例：Active 表示进行中，ReadyToTurnIn 表示可以交任务。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	EQuestState QuestState = EQuestState::NotStarted;

	// 任务下每个目标的进度快照。读档后恢复到 ObjectiveStates。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TArray<FObjectiveSaveRecord> ObjectiveRecords;

	// true：奖励已经领过，读档后不能重复发。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bRewardClaimed = false;

	// 当时从哪个 NPC 接到任务。任务 UI 或回访逻辑可以用它显示来源。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag QuestGiverNPC_ID;

	// 当前任务应该交给哪个 NPC。读档后交任务检测会用到它。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag TurnInNPC_ID;
};

// 单个全局旗标的存档记录。
// 读档后用它恢复门、机关、剧情分支、Boss 状态等世界状态。
USTRUCT(BlueprintType)
struct ESCAPEGAME_API FGlobalFlagSaveRecord
{
	GENERATED_BODY()

	// 这条记录属于哪个旗标。例：Flag.Door.BasementOpened。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag FlagID;

	// 旗标的开关值。例：门是否已打开、Boss 是否已出现。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bValue = false;

	// 旗标的数值状态。例：机关当前阶段、警戒等级、谜题计数。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	int32 NumericValue = 0;

	// 旗标的文本状态。仅在确实需要保存文字结果时使用。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FText TextValue;
};

// 世界事件存档记录。
// 用来保存“某个一次性/可重复事件是否发生过、发生过几次”。
USTRUCT(BlueprintType)
struct ESCAPEGAME_API FWorldEventSaveRecord
{
	GENERATED_BODY()

	// 这条记录属于哪个世界事件。例：WorldEvent.BridgeCollapsed。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag EventID;

	// true：这个事件至少触发过一次。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bTriggered = false;

	// 总共触发过几次。一次性事件通常是 0 或 1。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	int32 TriggerCount = 0;

	// 最近一次触发的世界时间，主要用于调试或限时逻辑。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	float LastTriggerTimeSeconds = 0.0f;
};
