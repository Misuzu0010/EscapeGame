// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Dialogue/EscapeDialogueTypes.h"
#include "DialogueQuestSubsystem.generated.h"

class UDialogueDefinition;
class UQuestDefinition;
class UDialogueParticipantComponent;
class UEscapeDialogueSaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueConversationStarted, const FConversationSession&, Session);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueConversationEnded, EInterruptReason, Reason);

// 对话与任务运行时管理器。
// 它负责开始/结束对话、保存当前会话、管理任务进度、全局旗标和存档对象。
UCLASS()
class ESCAPEGAME_API UDialogueQuestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 当前正在进行的对话会话。
	// 没有打开对话时应保持非激活状态。
	UPROPERTY(Transient)
	FConversationSession ActiveSession;
	
	// 对话运行时进度缓存。
	// Key 是 DialogueID，Value 记录已看节点、已选选项、当前节点等。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TMap<FGameplayTag, FDialogueRuntimeState> DialogueRuntimeMap;

	// 任务运行时进度缓存。
	// Key 是 QuestID，Value 记录任务状态、目标进度和奖励领取情况。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TMap<FGameplayTag, FQuestRuntimeState> QuestRuntimeMap;

	// 全局旗标缓存。
	// 用于记录门、机关、剧情分支、Boss 状态等跨系统共享状态。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TMap<FGameplayTag, FGlobalFlagState> GlobalFlagMap;

	// 已加载的对话资产缓存。
	// 避免同一段对话反复查找或加载。
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UDialogueDefinition>> LoadedDialogueDefinitions;

	// 已加载的任务资产缓存。
	// 后续任务查询、条件判断和奖励发放会从这里取配置。
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UQuestDefinition>> LoadedQuestDefinitions;

	// 当前正在使用的存档对象。
	UPROPERTY(Transient)
	TObjectPtr<UEscapeDialogueSaveGame> CurrentSaveGame;

	// 存档槽名。例："DialogueQuestSlot"。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	FString SaveSlotName;

	// UE SaveGame 使用的用户索引。单机项目通常保持 0。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	int32 UserIndex = 0;

	// true：关键对话效果执行后允许自动保存。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	bool bAutoSaveEnabled = true;

	// true：当前有对话 UI 或对话流程处于打开状态。
	UPROPERTY(Transient)
	bool bConversationOpen = false;

	// 对话成功开始时广播。Controller 用它来创建并显示对话 UI。
	UPROPERTY(BlueprintAssignable, Category="Dialogue")
	FOnDialogueConversationStarted OnConversationStarted;

	// 对话结束时广播。Controller 用它来关闭 UI 并恢复输入模式。
	UPROPERTY(BlueprintAssignable, Category="Dialogue")
	FOnDialogueConversationEnded OnConversationEnded;

	// 等待发放的奖励队列。
	// 用于处理延迟奖励、对白结束后奖励、结算界面确认后奖励。
	UPROPERTY(Transient)
	TArray<FRewardEffect> PendingRewardQueue;
	
	//对话组件
	UPROPERTY(Transient)
	TObjectPtr<UDialogueParticipantComponent> ActiveParticipantComponent;
	
	// 尝试开始一段对话。
	// Instigator 是发起者，通常是玩家；Participant 是被交互对象上的对话组件。
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool StartConversation(AActor* Instigator,UDialogueParticipantComponent *Participant);
	
	// 选择当前节点中的一个选项。
	// OptionID 必须来自 GetAvailableOptions 返回的选项。
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	bool SelectOption(FGameplayTag OptionID);
	
	// 结束当前对话，并记录结束或中断原因。
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	bool EndConversation(EInterruptReason Reason);
	
	// 当前是否存在激活中的对话。
	UFUNCTION(BlueprintPure, Category="Dialogue")
	bool IsConversationActive() const;

	// 获取当前正在显示的对话节点。
	UFUNCTION(BlueprintPure, Category="Dialogue")
	FDialogueNode GetCurrentNode() const;
	
	// 获取当前节点下满足条件、可显示或可点击的选项。
	UFUNCTION(BlueprintPure, Category="Dialogue")
	TArray<FDialogueOption> GetAvailableOptions() const;
	
	//评估当前状态是否可以继续对话
	UFUNCTION(BlueprintPure, Category="Dialogue")
	bool EvaluateCondition(const FDialogueCondition& Condition) const;
	
	// 执行一个对话效果。
	// 这个函数会修改任务、旗标或会话状态，不能标记为 BlueprintPure。
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void ApplyEffect(const FDialogueEffect& Effect);
	
	UFUNCTION(BlueprintPure, Category="Dialogue")
	UDialogueParticipantComponent* GetActiveParticipantComponent() const;

	
	
};
