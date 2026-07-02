// Fill out your copyright notice in the Description page of Project Settings.

#include "Dialogue/DialogueQuestSubsystem.h"

#include "Dialogue/DialogueDefinition.h"
#include "Dialogue/DialogueParticipantComponent.h"

bool UDialogueQuestSubsystem::StartConversation(AActor* Instigator, UDialogueParticipantComponent* Participant)
{
	if (!IsValid(Instigator))
	{
		UE_LOG(LogTemp, Error, TEXT("StartConversation 失败：Instigator 无效。"));
		return false;
	}

	if (!IsValid(Participant))
	{
		UE_LOG(LogTemp, Error, TEXT("StartConversation 失败：Participant 无效。"));
		return false;
	}

	if (!Participant->bCanTalk)
	{
		UE_LOG(LogTemp, Error, TEXT("StartConversation 失败：Participant 当前不允许对话。Owner=%s"), *GetNameSafe(Participant->GetOwner()));
		return false;
	}

	if (bConversationOpen)
	{
		UE_LOG(LogTemp, Error, TEXT("StartConversation 失败：当前已有对话正在进行。"));
		return false;
	}

	if (Participant->DialogueDefinition.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("StartConversation 失败：Participant 未配置 DialogueDefinition。Owner=%s"), *GetNameSafe(Participant->GetOwner()));
		return false;
	}

	UDialogueDefinition* DialogueAsset = Participant->DialogueDefinition.LoadSynchronous();
	if (!IsValid(DialogueAsset))
	{
		UE_LOG(LogTemp, Error, TEXT("StartConversation 失败：DialogueDefinition 加载失败。Owner=%s"), *GetNameSafe(Participant->GetOwner()));
		return false;
	}

	const FGameplayTag StartNodeID = Participant->DefaultStartNodeID.IsValid()
		? Participant->DefaultStartNodeID
		: DialogueAsset->StartNodeID;

	if (!StartNodeID.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("StartConversation 失败：未配置有效起始节点。DialogueID=%s"), *DialogueAsset->DialogueID.ToString());
		return false;
	}

	LoadedDialogueDefinitions.Add(DialogueAsset->DialogueID, DialogueAsset);

	ActiveSession = FConversationSession();
	ActiveSession.SessionId = FGuid::NewGuid();
	ActiveSession.NPC_ID = Participant->ParticipantID;
	ActiveSession.DialogueID = DialogueAsset->DialogueID;
	ActiveSession.CurrentNodeID = StartNodeID;
	ActiveSession.bIsActive = true;
	ActiveSession.bLockPlayer = Participant->bLockPlayer;
	ActiveSession.bCanSkip = Participant->bCanSkip;
	ActiveSession.InterruptPolicy = Participant->DefaultInterruptPolicy;
	ActiveSession.StartTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	bConversationOpen = true;

	FDialogueRuntimeState& State = DialogueRuntimeMap.FindOrAdd(ActiveSession.DialogueID);
	State.DialogueID = ActiveSession.DialogueID;
	State.bHasSeenDialogue = true;
	State.CurrentNodeID = StartNodeID;
	State.LastTalkPartnerID = Participant->ParticipantID;
	State.SeenNodeIDs.Add(StartNodeID);
	
	ActiveParticipantComponent = Participant;

	OnConversationStarted.Broadcast(ActiveSession);
	
	
	return true;
}

bool UDialogueQuestSubsystem::SelectOption(FGameplayTag OptionID)
{
	if (!IsConversationActive())
	{
		UE_LOG(LogTemp, Error, TEXT("SelectOption 失败：当前没有激活的对话。"));
		return false;
	}

	if (!OptionID.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("SelectOption 失败：OptionID 无效。"));
		return false;
	}
	FDialogueNode CurrentNode = GetCurrentNode();
	
	if (!CurrentNode.NodeID.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("SelectOption 失败：CurrentNode.OptionID 无效。"));
		return false;	
	}
	
	const FDialogueOption* SelectedOption = CurrentNode.Options.FindByPredicate(
	[OptionID](const FDialogueOption& Option)
	{
		return Option.OptionID == OptionID;
	});
	
	if (!SelectedOption)
	{
		UE_LOG(LogTemp, Error, TEXT("SelectOption 失败：无效选项。"));
		return false;
	}
	
	ActiveSession.LastSelectedOptionID = OptionID;
	
	
	FDialogueRuntimeState& State = DialogueRuntimeMap.FindOrAdd(ActiveSession.DialogueID);
	State.DialogueID = ActiveSession.DialogueID;
	State.bHasSeenDialogue = true;
	State.CurrentNodeID = ActiveSession.CurrentNodeID;
	State.LastTalkPartnerID = ActiveSession.NPC_ID;
	State.VisitedOptionIDs.Add(OptionID);

	for (const auto&Effect: SelectedOption->Effects)
	{
		ApplyEffect(Effect);
		
		if (!IsConversationActive())
		{
			return true;
		}
	}
	
	
	
	
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
}

bool UDialogueQuestSubsystem::EndConversation(EInterruptReason Reason)
{
	if (!IsConversationActive())
	{
		UE_LOG(LogTemp, Error, TEXT("EndConversation 失败：当前没有激活的对话。"));
		return false;
	}

	if (ActiveSession.DialogueID.IsValid())
	{
		FDialogueRuntimeState& State = DialogueRuntimeMap.FindOrAdd(ActiveSession.DialogueID);
		State.DialogueID = ActiveSession.DialogueID;
		State.LastTalkPartnerID = ActiveSession.NPC_ID;
		State.bHasSeenDialogue = true;

		if (ActiveSession.CurrentNodeID.IsValid())
		{
			State.CurrentNodeID = ActiveSession.CurrentNodeID;
			State.SeenNodeIDs.Add(ActiveSession.CurrentNodeID);
		}
	}

	ActiveSession.InterruptReason = Reason;
	ActiveSession.bIsActive = false;
	bConversationOpen = false;

	OnConversationEnded.Broadcast(Reason);
	ActiveParticipantComponent = nullptr;
	return true;
}

bool UDialogueQuestSubsystem::IsConversationActive() const
{
	return bConversationOpen && ActiveSession.bIsActive;
}

FDialogueNode UDialogueQuestSubsystem::GetCurrentNode() const
{
	if (!IsConversationActive())
	{
		UE_LOG(LogTemp, Error, TEXT("GetCurrentNode 失败：当前没有激活的对话。"));
		return FDialogueNode();
	}

	if (!ActiveSession.DialogueID.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("GetCurrentNode 失败：ActiveSession.DialogueID 无效。"));
		return FDialogueNode();
	}

	if (!ActiveSession.CurrentNodeID.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("GetCurrentNode 失败：ActiveSession.CurrentNodeID 无效。"));
		return FDialogueNode();
	}

	const TObjectPtr<UDialogueDefinition>* DialogueDefinitionPtr = LoadedDialogueDefinitions.Find(ActiveSession.DialogueID);
	if (!DialogueDefinitionPtr || !IsValid(DialogueDefinitionPtr->Get()))
	{
		UE_LOG(LogTemp, Error, TEXT("GetCurrentNode 失败：未找到已加载的对话资产。DialogueID=%s"), *ActiveSession.DialogueID.ToString());
		return FDialogueNode();
	}

	const FDialogueNode* FoundNode = DialogueDefinitionPtr->Get()->Nodes.FindByPredicate(
		[this](const FDialogueNode& Node)
		{
			return Node.NodeID == ActiveSession.CurrentNodeID;
		});

	if (!FoundNode)
	{
		UE_LOG(LogTemp, Error, TEXT("GetCurrentNode 失败：对话资产中找不到节点。NodeID=%s"), *ActiveSession.CurrentNodeID.ToString());
		return FDialogueNode();
	}

	return *FoundNode;
}

TArray<FDialogueOption> UDialogueQuestSubsystem::GetAvailableOptions() const
{
	if (!IsConversationActive())
	{
		UE_LOG(LogTemp, Error, TEXT("GetAvailableOptions 失败：当前没有激活的对话。"));
		return TArray<FDialogueOption>();
	}

	const FDialogueNode CurrentNode = GetCurrentNode();
	if (!CurrentNode.NodeID.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("GetAvailableOptions 失败：当前节点无效。"));
		return TArray<FDialogueOption>();
	}

	TArray<FDialogueOption> AvailableOptions;
	for (const FDialogueOption& Option : CurrentNode.Options)
	{
		bool bAllConditionsPassed = true;
		for (const FDialogueCondition& Condition : Option.Conditions)
		{
			if (!EvaluateCondition(Condition))
			{
				bAllConditionsPassed = false;
				break;
			}
		}
		if (bAllConditionsPassed)
		{
			AvailableOptions.Add(Option);
		}
	}
	return AvailableOptions;
}


bool UDialogueQuestSubsystem::EvaluateCondition(const FDialogueCondition& Condition) const
{
	bool bResult = false;

	switch (Condition.ConditionType)
	{
	case EDialogueConditionType::GlobalFlagIs:
		{
			const FGlobalFlagState* FlagState = GlobalFlagMap.Find(Condition.FlagID);
			if (!FlagState)
			{
				UE_LOG(LogTemp, Warning, TEXT("EvaluateCondition GlobalFlagIs 失败：找不到 FlagID=%s。"),
					*Condition.FlagID.ToString());
				bResult = false;
				break;
			}

			bResult = FlagState->bValue == Condition.ExpectedBoolValue;
			break;
		}

	case EDialogueConditionType::DialogueNodeSeen:
		{
			const FDialogueRuntimeState* DialogueState = DialogueRuntimeMap.Find(ActiveSession.DialogueID);
			if (!DialogueState)
			{
				UE_LOG(LogTemp, Warning, TEXT("EvaluateCondition DialogueNodeSeen 失败：找不到 DialogueID=%s 的运行时状态。"),
					*ActiveSession.DialogueID.ToString());
				bResult = false;
				break;
			}

			const bool bHasSeenNode = DialogueState->SeenNodeIDs.Contains(Condition.ExpectedTagValue);
			bResult = bHasSeenNode == Condition.ExpectedBoolValue;
			break;
		}

	default:
		UE_LOG(LogTemp, Warning, TEXT("EvaluateCondition 不支持的条件类型：ConditionType=%d。"),
			static_cast<int32>(Condition.ConditionType));
		bResult = false;
		break;
	}

	return Condition.bInvert ? !bResult : bResult;
}

void UDialogueQuestSubsystem::ApplyEffect(const FDialogueEffect& Effect)
{
	//根据当前的Effect.EffectType来选实际的效果
	switch (Effect.EffectType)
	{
	case EDialogueEffectType::SetGlobalFlag:
		{
			if (!Effect.FlagID.IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("ApplyEffect SetGlobalFlag 失败：FlagID 无效。"));
				return;
			}
			FGlobalFlagState& FlagState = GlobalFlagMap.FindOrAdd(Effect.FlagID);
			FlagState.FlagID = Effect.FlagID;
			FlagState.bValue = Effect.BoolValue;
			FlagState.LastChangedTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
			return;
		}
	case EDialogueEffectType::CloseDialogue:
		{
			if (!IsConversationActive())
			{
				UE_LOG(LogTemp, Warning, TEXT("ApplyEffect CloseDialogue 跳过：当前没有激活的对话。"));
				return;
			}

			EndConversation(EInterruptReason::NormalEnd);
			return;
		}
	default:
		UE_LOG(LogTemp, Warning, TEXT("ApplyEffect 不支持的效果类型：EffectType=%d。"),
		static_cast<int32>(Effect.EffectType));
		return;
		
	}
}
UDialogueParticipantComponent* UDialogueQuestSubsystem::GetActiveParticipantComponent() const
{
	return ActiveParticipantComponent;
}