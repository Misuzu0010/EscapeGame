// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Dialogue/EscapeDialogueTypes.h"
#include "QuestDefinition.generated.h"

// 任务内容资产。
// 它只描述任务“是什么”：标题、目标、奖励、前置条件；不保存玩家当前做到哪一步。
UCLASS()
class ESCAPEGAME_API UQuestDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	// 任务唯一标识。例：Quest.Main.FindBasementKey。
	// 运行时进度和存档都会通过这个 ID 对应到本任务。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
	FGameplayTag QuestID;

	// 任务在 UI 中显示的标题。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
	FText QuestTitle;

	// 任务详情文本，用于任务日志或接任务界面。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
	FText QuestDescription;

	// 默认接任务 NPC。为空时可以由对话效果或脚本决定。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
	FGameplayTag QuestGiverNPC_ID;

	// 默认交任务 NPC。为空时可以表示自动完成或不需要交付。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
	FGameplayTag TurnInNPC_ID;

	// 接任务前必须满足的条件。
	// 例：某段对话已看过、某个旗标为 true、前置任务已完成。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
	TArray<FDialogueCondition> Prerequisites;

	// 任务目标列表。每个目标负责描述一个可推进的任务要求。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
	TArray<FQuestObjectiveDefinition> Objectives;

	// 任务奖励列表。可配置物品、属性变化、解锁项或提示消息。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
	TArray<FQuestRewardDefinition> Rewards;

	// true：任务完成并领取奖励后，允许再次接取。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
	bool bCanRepeat = false;

	// true：所有必要目标完成后自动把任务标记为完成。
	// false：完成目标后进入 ReadyToTurnIn，等待玩家交任务。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
	bool bAutoComplete = false;
};
