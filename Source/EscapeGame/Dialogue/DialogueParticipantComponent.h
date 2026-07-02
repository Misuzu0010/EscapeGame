// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Dialogue/EscapeDialogueTypes.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "DialogueParticipantComponent.generated.h"

class UDialogueDefinition;


// 可挂在任意 Actor 上的对话参与者组件。
// 普通 NPC、敌人、Boss、机关都可以用它来复用同一套对话身份和触发配置。
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESCAPEGAME_API UDialogueParticipantComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDialogueParticipantComponent();
	// 参与者唯一 ID。例：NPC.Village.OldMan、Enemy.Boss.Butcher。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	FGameplayTag ParticipantID;
	
	// 对外显示的名字。例：“老村民”“屠夫”。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	FText DisplayName;
	
	// 对话头像。用于 UI 左上角头像显示。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	TSoftObjectPtr<UTexture2D> Portrait;
	
	// 默认从哪个节点开始说。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	FGameplayTag DefaultStartNodeID;
	
	// 默认使用哪份对话资产。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	TSoftObjectPtr<UDialogueDefinition> DialogueDefinition;
	
	// 这个参与者默认遵守的中断规则。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	FInterruptPolicy DefaultInterruptPolicy;
	
	// 触发方式。手动交互、靠近自动触发、战斗前强制对白都在这里区分。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	EDialogueTriggerMode TriggerMode = EDialogueTriggerMode::ManualInteract;
	
	// true：当前这个参与者允许开始对话。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	bool bCanTalk;
	
	// true：对话开始时朝向触发者或玩家。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	bool bFaceInstigator;
	
	// true：对话期间锁住玩家输入或移动。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	bool bLockPlayer;
	
	// true：允许玩家跳过这段对话。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	bool bCanSkip;
	
	// true：进入触发范围后自动开对话。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	bool bAutoStartConversation;
	
	// true：Boss 或敌人进入战斗前必须先播这段对白。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	bool bForceDialogueBeforeCombat;
	
	// true：这段战斗前对白只播放一次。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	bool bOnlyForceOnce;
	
	// true：对话结束后自动进入遭遇战或战斗状态。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	bool bStartEncounterAfterDialogue;
	
	// 关联的 Boss 或遭遇战 ID。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	FGameplayTag LinkedEncounterTag;
	
	// 记录“战斗前强制对白是否已播过”的全局旗标。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	FGameplayTag ForcedDialoguePlayedFlag;
	
};
