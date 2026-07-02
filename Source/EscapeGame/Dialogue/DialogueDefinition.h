// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Dialogue/EscapeDialogueTypes.h"
#include "DialogueDefinition.generated.h"

// 对话内容资产。
// 一份 UDialogueDefinition 就是一棵对话树：从 StartNodeID 开始，根据节点和选项跳转。
UCLASS()
class ESCAPEGAME_API UDialogueDefinition : public UDataAsset
{
	GENERATED_BODY()
public:

	// 这份对话资产的唯一标识。例：Dialogue.NPC.OldMan.Intro。
	// 存档和运行时状态会用它来找到这段对话的进度。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag DialogueID;

	// 默认进入对话时从哪个节点开始。
	// 如果参与者组件没有指定起始节点，就使用这里。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag StartNodeID;

	// 这段对话包含的所有节点。
	// 每个节点需要有唯一 NodeID，选项通过 NextNodeID 跳到对应节点。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TArray<FDialogueNode> Nodes;

	// true：这段对话看完后允许重新从起点打开。
	// false：看过后通常改走运行时记录里的节点或后续分支。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bAllowRestart = true;

	// true：后续文本读取可以接入本地化表或文本 Key。
	// 当前阶段先保留开关，不强制实现本地化读取。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bUseLocalizedText = false;

	// 这段对话默认使用的中断规则。
	// 单个节点也可以配置自己的 InterruptPolicy 覆盖默认行为。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FInterruptPolicy DefaultInterruptPolicy;
};
