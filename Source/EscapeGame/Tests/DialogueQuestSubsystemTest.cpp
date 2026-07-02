#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Dialogue/DialogueDefinition.h"
#include "Core/EscapeGameplayTags.h"
#include "Dialogue/DialogueQuestSubsystem.h"
#include "Engine/GameInstance.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueQuestSubsystemSelectOptionChangesNodeTest,
	"EscapeGame.Dialogue.SelectOption.ChangesCurrentNode",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FDialogueQuestSubsystemSelectOptionChangesNodeTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	TestNotNull(TEXT("GameInstance can be created"), GameInstance);

	UDialogueQuestSubsystem* Subsystem = NewObject<UDialogueQuestSubsystem>(GameInstance);
	TestNotNull(TEXT("Subsystem can be created"), Subsystem);

	UDialogueDefinition* Dialogue = NewObject<UDialogueDefinition>();
	TestNotNull(TEXT("Dialogue asset can be created"), Dialogue);

	const FGameplayTag DialogueID = EscapeGameplayTags::Dialogue_Gatekeeper_Intro;
	const FGameplayTag RootNodeID = EscapeGameplayTags::Dialogue_Node_Gatekeeper_Root;
	const FGameplayTag NextNodeID = EscapeGameplayTags::Dialogue_Node_Gatekeeper_KeyHint;
	const FGameplayTag OptionID = EscapeGameplayTags::Dialogue_Option_Gatekeeper_AskKey;
	const FGameplayTag NPCID = EscapeGameplayTags::NPC_Village_Gatekeeper;

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueQuestSubsystemSelectOptionClosesConversationTest,
	"EscapeGame.Dialogue.SelectOption.ClosesConversation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FDialogueQuestSubsystemSelectOptionClosesConversationTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	TestNotNull(TEXT("GameInstance can be created"), GameInstance);

	UDialogueQuestSubsystem* Subsystem = NewObject<UDialogueQuestSubsystem>(GameInstance);
	TestNotNull(TEXT("Subsystem can be created"), Subsystem);

	UDialogueDefinition* Dialogue = NewObject<UDialogueDefinition>();
	TestNotNull(TEXT("Dialogue asset can be created"), Dialogue);

	const FGameplayTag DialogueID = EscapeGameplayTags::Dialogue_Gatekeeper_Intro;
	const FGameplayTag RootNodeID = EscapeGameplayTags::Dialogue_Node_Gatekeeper_Root;
	const FGameplayTag OptionID = EscapeGameplayTags::Dialogue_Option_Gatekeeper_Leave;
	const FGameplayTag NPCID = EscapeGameplayTags::NPC_Village_Gatekeeper;

	Dialogue->DialogueID = DialogueID;

	FDialogueOption CloseOption;
	CloseOption.OptionID = OptionID;
	CloseOption.OptionText = FText::FromString(TEXT("Leave"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueQuestSubsystemGetAvailableOptionsGlobalFlagConditionShowsMatchingOptionTest,
	"EscapeGame.Dialogue.GetAvailableOptions.GlobalFlagConditionShowsMatchingOption",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FDialogueQuestSubsystemGetAvailableOptionsGlobalFlagConditionShowsMatchingOptionTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	TestNotNull(TEXT("GameInstance can be created"), GameInstance);

	UDialogueQuestSubsystem* Subsystem = NewObject<UDialogueQuestSubsystem>(GameInstance);
	TestNotNull(TEXT("Subsystem can be created"), Subsystem);

	UDialogueDefinition* Dialogue = NewObject<UDialogueDefinition>();
	TestNotNull(TEXT("Dialogue asset can be created"), Dialogue);
	
	const FGameplayTag DialogueID = EscapeGameplayTags::Dialogue_Gatekeeper_Intro;
	const FGameplayTag RootNodeID = EscapeGameplayTags::Dialogue_Node_Gatekeeper_Root;
	const FGameplayTag OptionID = EscapeGameplayTags::Dialogue_Option_Gatekeeper_AskKey;
	const FGameplayTag NPCID = EscapeGameplayTags::NPC_Village_Gatekeeper;
	const FGameplayTag FlagID = EscapeGameplayTags::Flag_World_MainGateUnlocked;
	
	FDialogueCondition FlagCondition;
	FlagCondition.ConditionType = EDialogueConditionType::GlobalFlagIs;
	FlagCondition.FlagID = FlagID;
	FlagCondition.ExpectedBoolValue = true;
	
	
	FDialogueOption AskKeyOption;
	AskKeyOption.OptionID = OptionID;
	AskKeyOption.OptionText = FText::FromString(TEXT("Ask key"));
	AskKeyOption.Conditions.Add(FlagCondition);
	
	FDialogueNode RootNode;
	RootNode.NodeID = RootNodeID;
	RootNode.SpeakerID = NPCID;
	RootNode.SpeakerText = FText::FromString(TEXT("Root"));
	RootNode.Options.Add(AskKeyOption);
	
	Dialogue->DialogueID = DialogueID;
	Dialogue->Nodes.Add(RootNode);
	
	Subsystem->LoadedDialogueDefinitions.Add(DialogueID, Dialogue);
	Subsystem->bConversationOpen = true;
	Subsystem->ActiveSession.DialogueID = DialogueID;
	Subsystem->ActiveSession.CurrentNodeID = RootNodeID;
	Subsystem->ActiveSession.bIsActive = true;
	Subsystem->ActiveSession.NPC_ID = NPCID;
	
	FDialogueRuntimeState& RuntimeState = Subsystem->DialogueRuntimeMap.FindOrAdd(DialogueID);
	RuntimeState.DialogueID = DialogueID;
	RuntimeState.CurrentNodeID = RootNodeID;
	RuntimeState.bHasSeenDialogue = true;
	RuntimeState.SeenNodeIDs.Add(RootNodeID);
	
	FGlobalFlagState& FlagState = Subsystem->GlobalFlagMap.FindOrAdd(FlagID);
	FlagState.FlagID = FlagID;
	FlagState.bValue = true;
	
	const TArray<FDialogueOption> AvailableOptions = Subsystem->GetAvailableOptions();
	
	TestEqual(TEXT("One option is available"), AvailableOptions.Num(), 1);

	if (AvailableOptions.Num() == 1)
	{
		TestEqual(TEXT("Available option is AskKey"), AvailableOptions[0].OptionID, OptionID);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueQuestSubsystemGetAvailableOptionsGlobalFlagConditionHidesNonMatchingOptionTest,
	"EscapeGame.Dialogue.GetAvailableOptions.GlobalFlagConditionHidesNonMatchingOption",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FDialogueQuestSubsystemGetAvailableOptionsGlobalFlagConditionHidesNonMatchingOptionTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	TestNotNull(TEXT("GameInstance can be created"), GameInstance);

	UDialogueQuestSubsystem* Subsystem = NewObject<UDialogueQuestSubsystem>(GameInstance);
	TestNotNull(TEXT("Subsystem can be created"), Subsystem);

	UDialogueDefinition* Dialogue = NewObject<UDialogueDefinition>();
	TestNotNull(TEXT("Dialogue asset can be created"), Dialogue);

	const FGameplayTag DialogueID = EscapeGameplayTags::Dialogue_Gatekeeper_Intro;
	const FGameplayTag RootNodeID = EscapeGameplayTags::Dialogue_Node_Gatekeeper_Root;
	const FGameplayTag OptionID = EscapeGameplayTags::Dialogue_Option_Gatekeeper_AskKey;
	const FGameplayTag NPCID = EscapeGameplayTags::NPC_Village_Gatekeeper;
	const FGameplayTag FlagID = EscapeGameplayTags::Flag_World_MainGateUnlocked;

	FDialogueCondition FlagCondition;
	FlagCondition.ConditionType = EDialogueConditionType::GlobalFlagIs;
	FlagCondition.FlagID = FlagID;
	FlagCondition.ExpectedBoolValue = true;

	FDialogueOption AskKeyOption;
	AskKeyOption.OptionID = OptionID;
	AskKeyOption.OptionText = FText::FromString(TEXT("Ask key"));
	AskKeyOption.Conditions.Add(FlagCondition);

	FDialogueNode RootNode;
	RootNode.NodeID = RootNodeID;
	RootNode.SpeakerID = NPCID;
	RootNode.SpeakerText = FText::FromString(TEXT("Root"));
	RootNode.Options.Add(AskKeyOption);

	Dialogue->DialogueID = DialogueID;
	Dialogue->Nodes.Add(RootNode);

	Subsystem->LoadedDialogueDefinitions.Add(DialogueID, Dialogue);
	Subsystem->bConversationOpen = true;
	Subsystem->ActiveSession.DialogueID = DialogueID;
	Subsystem->ActiveSession.CurrentNodeID = RootNodeID;
	Subsystem->ActiveSession.bIsActive = true;
	Subsystem->ActiveSession.NPC_ID = NPCID;

	FDialogueRuntimeState& RuntimeState = Subsystem->DialogueRuntimeMap.FindOrAdd(DialogueID);
	RuntimeState.DialogueID = DialogueID;
	RuntimeState.CurrentNodeID = RootNodeID;
	RuntimeState.bHasSeenDialogue = true;
	RuntimeState.SeenNodeIDs.Add(RootNodeID);

	FGlobalFlagState& FlagState = Subsystem->GlobalFlagMap.FindOrAdd(FlagID);
	FlagState.FlagID = FlagID;
	FlagState.bValue = false;

	const TArray<FDialogueOption> AvailableOptions = Subsystem->GetAvailableOptions();

	TestEqual(TEXT("No option is available when global flag does not match"), AvailableOptions.Num(), 0);
	

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueQuestSubsystemGetAvailableOptionsDialogueNodeSeenConditionHidesUnseenOptionTest,
	"EscapeGame.Dialogue.GetAvailableOptions.DialogueNodeSeenConditionHidesUnseenOption",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FDialogueQuestSubsystemGetAvailableOptionsDialogueNodeSeenConditionHidesUnseenOptionTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	TestNotNull(TEXT("GameInstance can be created"), GameInstance);

	UDialogueQuestSubsystem* Subsystem = NewObject<UDialogueQuestSubsystem>(GameInstance);
	TestNotNull(TEXT("Subsystem can be created"), Subsystem);

	UDialogueDefinition* Dialogue = NewObject<UDialogueDefinition>();
	TestNotNull(TEXT("Dialogue asset can be created"), Dialogue);

	const FGameplayTag DialogueID = EscapeGameplayTags::Dialogue_Gatekeeper_Intro;
	const FGameplayTag RootNodeID = EscapeGameplayTags::Dialogue_Node_Gatekeeper_Root;
	const FGameplayTag RequiredSeenNodeID = EscapeGameplayTags::Dialogue_Node_Gatekeeper_KeyHint;
	const FGameplayTag OptionID = EscapeGameplayTags::Dialogue_Option_Gatekeeper_AskKey;
	const FGameplayTag NPCID = EscapeGameplayTags::NPC_Village_Gatekeeper;
	
	FDialogueCondition SeenCondition;
	SeenCondition.ConditionType = EDialogueConditionType::DialogueNodeSeen;
	SeenCondition.ExpectedTagValue = RequiredSeenNodeID;
	SeenCondition.ExpectedBoolValue = true;

	FDialogueOption AskKeyOption;
	AskKeyOption.OptionID = OptionID;
	AskKeyOption.OptionText = FText::FromString(TEXT("Ask key"));
	AskKeyOption.Conditions.Add(SeenCondition);

	FDialogueNode RootNode;
	RootNode.NodeID = RootNodeID;
	RootNode.SpeakerID = NPCID;
	RootNode.SpeakerText = FText::FromString(TEXT("Root"));
	RootNode.Options.Add(AskKeyOption);

	Dialogue->DialogueID = DialogueID;
	Dialogue->Nodes.Add(RootNode);

	Subsystem->LoadedDialogueDefinitions.Add(DialogueID, Dialogue);
	Subsystem->bConversationOpen = true;
	Subsystem->ActiveSession.DialogueID = DialogueID;
	Subsystem->ActiveSession.CurrentNodeID = RootNodeID;
	Subsystem->ActiveSession.bIsActive = true;
	Subsystem->ActiveSession.NPC_ID = NPCID;

	FDialogueRuntimeState& RuntimeState = Subsystem->DialogueRuntimeMap.FindOrAdd(DialogueID);
	RuntimeState.DialogueID = DialogueID;
	RuntimeState.CurrentNodeID = RootNodeID;
	RuntimeState.bHasSeenDialogue = true;
	RuntimeState.SeenNodeIDs.Add(RootNodeID);

	const TArray<FDialogueOption> AvailableOptions = Subsystem->GetAvailableOptions();

	TestEqual(TEXT("No option is available when required dialogue node has not been seen"), AvailableOptions.Num(), 0);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueQuestSubsystemGetAvailableOptionsDialogueNodeSeenConditionShowsSeenOptionTest,
	"EscapeGame.Dialogue.GetAvailableOptions.DialogueNodeSeenConditionShowsSeenOption",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FDialogueQuestSubsystemGetAvailableOptionsDialogueNodeSeenConditionShowsSeenOptionTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	TestNotNull(TEXT("GameInstance can be created"), GameInstance);

	UDialogueQuestSubsystem* Subsystem = NewObject<UDialogueQuestSubsystem>(GameInstance);
	TestNotNull(TEXT("Subsystem can be created"), Subsystem);

	UDialogueDefinition* Dialogue = NewObject<UDialogueDefinition>();
	TestNotNull(TEXT("Dialogue asset can be created"), Dialogue);

	const FGameplayTag DialogueID = EscapeGameplayTags::Dialogue_Gatekeeper_Intro;
	const FGameplayTag RootNodeID = EscapeGameplayTags::Dialogue_Node_Gatekeeper_Root;
	const FGameplayTag RequiredSeenNodeID = EscapeGameplayTags::Dialogue_Node_Gatekeeper_KeyHint;
	const FGameplayTag OptionID = EscapeGameplayTags::Dialogue_Option_Gatekeeper_AskKey;
	const FGameplayTag NPCID = EscapeGameplayTags::NPC_Village_Gatekeeper;

	FDialogueCondition SeenCondition;
	SeenCondition.ConditionType = EDialogueConditionType::DialogueNodeSeen;
	SeenCondition.ExpectedTagValue = RequiredSeenNodeID;
	SeenCondition.ExpectedBoolValue = true;

	FDialogueOption AskKeyOption;
	AskKeyOption.OptionID = OptionID;
	AskKeyOption.OptionText = FText::FromString(TEXT("Ask key"));
	AskKeyOption.Conditions.Add(SeenCondition);

	FDialogueNode RootNode;
	RootNode.NodeID = RootNodeID;
	RootNode.SpeakerID = NPCID;
	RootNode.SpeakerText = FText::FromString(TEXT("Root"));
	RootNode.Options.Add(AskKeyOption);

	Dialogue->DialogueID = DialogueID;
	Dialogue->Nodes.Add(RootNode);

	Subsystem->LoadedDialogueDefinitions.Add(DialogueID, Dialogue);
	Subsystem->bConversationOpen = true;
	Subsystem->ActiveSession.DialogueID = DialogueID;
	Subsystem->ActiveSession.CurrentNodeID = RootNodeID;
	Subsystem->ActiveSession.bIsActive = true;
	Subsystem->ActiveSession.NPC_ID = NPCID;

	FDialogueRuntimeState& RuntimeState = Subsystem->DialogueRuntimeMap.FindOrAdd(DialogueID);
	RuntimeState.DialogueID = DialogueID;
	RuntimeState.CurrentNodeID = RootNodeID;
	RuntimeState.bHasSeenDialogue = true;
	RuntimeState.SeenNodeIDs.Add(RootNodeID);
	RuntimeState.SeenNodeIDs.Add(RequiredSeenNodeID);

	const TArray<FDialogueOption> AvailableOptions = Subsystem->GetAvailableOptions();

	TestEqual(TEXT("One option is available when required dialogue node has been seen"), AvailableOptions.Num(), 1);

	if (AvailableOptions.Num() == 1)
	{
		TestEqual(TEXT("Available option is AskKey"), AvailableOptions[0].OptionID, OptionID);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueQuestSubsystemApplyEffectSetGlobalFlagTest,
	"EscapeGame.Dialogue.ApplyEffect.SetGlobalFlag",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FDialogueQuestSubsystemApplyEffectSetGlobalFlagTest::RunTest(const FString& Parameters)
{
	// Arrange: 创建 GameInstance、Subsystem，准备 SetGlobalFlag Effect
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	TestNotNull(TEXT("GameInstance can be created"), GameInstance);

	UDialogueQuestSubsystem* Subsystem = NewObject<UDialogueQuestSubsystem>(GameInstance);
	TestNotNull(TEXT("Subsystem can be created"), Subsystem);

	UDialogueDefinition* Dialogue = NewObject<UDialogueDefinition>();
	TestNotNull(TEXT("Dialogue asset can be created"), Dialogue);
	FDialogueEffect Effect;
	Effect.EffectType = EDialogueEffectType::SetGlobalFlag;
	Effect.FlagID = EscapeGameplayTags::Flag_World_MainGateUnlocked;
	Effect.BoolValue = true;
	Subsystem->ApplyEffect(Effect);
	

	// Act: 调用 Subsystem->ApplyEffect(Effect)

	// Assert: 检查 GlobalFlagMap 是否写入目标 Flag
	const FGlobalFlagState *FlagState =Subsystem->GlobalFlagMap.Find(EscapeGameplayTags::Flag_World_MainGateUnlocked);
	TestNotNull(TEXT("Global flag should be created"), FlagState);

	if (FlagState)
	{
		TestTrue(TEXT("Global flag value should be true"), FlagState->bValue);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueQuestSubsystemApplyEffectCloseDialogueTest,
	"EscapeGame.Dialogue.ApplyEffect.CloseDialogue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FDialogueQuestSubsystemApplyEffectCloseDialogueTest::RunTest(const FString& Parameters)
{
	// Arrange: 创建 GameInstance、Subsystem，并手动构造激活对话状态
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	TestNotNull(TEXT("GameInstance can be created"), GameInstance);

	UDialogueQuestSubsystem* Subsystem = NewObject<UDialogueQuestSubsystem>(GameInstance);
	TestNotNull(TEXT("Subsystem can be created"), Subsystem);

	UDialogueDefinition* Dialogue = NewObject<UDialogueDefinition>();
	TestNotNull(TEXT("Dialogue asset can be created"), Dialogue);

	// Act: 调用 Subsystem->ApplyEffect(CloseDialogueEffect)
	FDialogueEffect Effect;
	Effect.EffectType = EDialogueEffectType::CloseDialogue;
	
	Subsystem->bConversationOpen = true;
	Subsystem->ActiveSession.bIsActive = true;
	Subsystem->ActiveSession.DialogueID = EscapeGameplayTags::Dialogue_Gatekeeper_Intro;
	Subsystem->ActiveSession.CurrentNodeID = EscapeGameplayTags::Dialogue_Node_Gatekeeper_Root;
	Subsystem->ActiveSession.NPC_ID = EscapeGameplayTags::NPC_Village_Gatekeeper;
	Subsystem->ApplyEffect(Effect);
	

	// Assert: 检查 IsConversationActive()、bConversationOpen、InterruptReason
	TestFalse(TEXT("Conversation should be closed"), Subsystem->IsConversationActive());
	TestFalse(TEXT("Conversation open flag should be false"), Subsystem->bConversationOpen);
	TestEqual(
		TEXT("Interrupt reason should be normal end"),
		Subsystem->ActiveSession.InterruptReason,
		EInterruptReason::NormalEnd
	);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueQuestSubsystemSelectOptionAppliesSetGlobalFlagEffectTest,
	"EscapeGame.Dialogue.SelectOption.AppliesSetGlobalFlagEffect",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FDialogueQuestSubsystemSelectOptionAppliesSetGlobalFlagEffectTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	TestNotNull(TEXT("GameInstance can be created"), GameInstance);

	UDialogueQuestSubsystem* Subsystem = NewObject<UDialogueQuestSubsystem>(GameInstance);
	TestNotNull(TEXT("Subsystem can be created"), Subsystem);

	UDialogueDefinition* Dialogue = NewObject<UDialogueDefinition>();
	TestNotNull(TEXT("Dialogue asset can be created"), Dialogue);

	const FGameplayTag DialogueID = EscapeGameplayTags::Dialogue_Gatekeeper_Intro;
	const FGameplayTag RootNodeID = EscapeGameplayTags::Dialogue_Node_Gatekeeper_Root;
	const FGameplayTag OptionID = EscapeGameplayTags::Dialogue_Option_Gatekeeper_TurnInKey;
	const FGameplayTag NPCID = EscapeGameplayTags::NPC_Village_Gatekeeper;
	const FGameplayTag FlagID = EscapeGameplayTags::Flag_World_MainGateUnlocked;

	FDialogueEffect SetFlagEffect;
	SetFlagEffect.EffectType = EDialogueEffectType::SetGlobalFlag;
	SetFlagEffect.FlagID = FlagID;
	SetFlagEffect.BoolValue = true;

	FDialogueOption TurnInKeyOption;
	TurnInKeyOption.OptionID = OptionID;
	TurnInKeyOption.OptionText = FText::FromString(TEXT("Open the gate"));
	TurnInKeyOption.Effects.Add(SetFlagEffect);

	FDialogueNode RootNode;
	RootNode.NodeID = RootNodeID;
	RootNode.SpeakerID = NPCID;
	RootNode.SpeakerText = FText::FromString(TEXT("Do you have the key?"));
	RootNode.Options.Add(TurnInKeyOption);

	Dialogue->DialogueID = DialogueID;
	Dialogue->Nodes.Add(RootNode);

	Subsystem->LoadedDialogueDefinitions.Add(DialogueID, Dialogue);
	Subsystem->bConversationOpen = true;
	Subsystem->ActiveSession.DialogueID = DialogueID;
	Subsystem->ActiveSession.CurrentNodeID = RootNodeID;
	Subsystem->ActiveSession.bIsActive = true;
	Subsystem->ActiveSession.NPC_ID = NPCID;

	FDialogueRuntimeState& RuntimeState = Subsystem->DialogueRuntimeMap.FindOrAdd(DialogueID);
	RuntimeState.DialogueID = DialogueID;
	RuntimeState.CurrentNodeID = RootNodeID;
	RuntimeState.bHasSeenDialogue = true;
	RuntimeState.SeenNodeIDs.Add(RootNodeID);

	const bool bSelected = Subsystem->SelectOption(OptionID);

	TestTrue(TEXT("SelectOption should succeed"), bSelected);

	const FGlobalFlagState* FlagState = Subsystem->GlobalFlagMap.Find(FlagID);
	TestNotNull(TEXT("SelectOption should apply SetGlobalFlag effect"), FlagState);

	if (FlagState)
	{
		TestTrue(TEXT("Global flag value should be true"), FlagState->bValue);
		TestEqual(TEXT("Global flag ID should be recorded"), FlagState->FlagID, FlagID);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueQuestSubsystemSelectOptionAppliesCloseDialogueEffectTest,
	"EscapeGame.Dialogue.SelectOption.AppliesCloseDialogueEffect",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FDialogueQuestSubsystemSelectOptionAppliesCloseDialogueEffectTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	TestNotNull(TEXT("GameInstance can be created"), GameInstance);

	UDialogueQuestSubsystem* Subsystem = NewObject<UDialogueQuestSubsystem>(GameInstance);
	TestNotNull(TEXT("Subsystem can be created"), Subsystem);

	UDialogueDefinition* Dialogue = NewObject<UDialogueDefinition>();
	TestNotNull(TEXT("Dialogue asset can be created"), Dialogue);

	const FGameplayTag DialogueID = EscapeGameplayTags::Dialogue_Gatekeeper_Intro;
	const FGameplayTag RootNodeID = EscapeGameplayTags::Dialogue_Node_Gatekeeper_Root;
	const FGameplayTag OptionID = EscapeGameplayTags::Dialogue_Option_Gatekeeper_Leave;
	const FGameplayTag NPCID = EscapeGameplayTags::NPC_Village_Gatekeeper;

	FDialogueEffect CloseDialogueEffect;
	CloseDialogueEffect.EffectType = EDialogueEffectType::CloseDialogue;

	FDialogueOption LeaveOption;
	LeaveOption.OptionID = OptionID;
	LeaveOption.OptionText = FText::FromString(TEXT("Leave"));
	LeaveOption.Effects.Add(CloseDialogueEffect);

	FDialogueNode RootNode;
	RootNode.NodeID = RootNodeID;
	RootNode.SpeakerID = NPCID;
	RootNode.SpeakerText = FText::FromString(TEXT("Goodbye?"));
	RootNode.Options.Add(LeaveOption);

	Dialogue->DialogueID = DialogueID;
	Dialogue->Nodes.Add(RootNode);

	Subsystem->LoadedDialogueDefinitions.Add(DialogueID, Dialogue);
	Subsystem->bConversationOpen = true;
	Subsystem->ActiveSession.DialogueID = DialogueID;
	Subsystem->ActiveSession.CurrentNodeID = RootNodeID;
	Subsystem->ActiveSession.bIsActive = true;
	Subsystem->ActiveSession.NPC_ID = NPCID;

	FDialogueRuntimeState& RuntimeState = Subsystem->DialogueRuntimeMap.FindOrAdd(DialogueID);
	RuntimeState.DialogueID = DialogueID;
	RuntimeState.CurrentNodeID = RootNodeID;
	RuntimeState.bHasSeenDialogue = true;
	RuntimeState.SeenNodeIDs.Add(RootNodeID);

	const bool bSelected = Subsystem->SelectOption(OptionID);

	TestTrue(TEXT("SelectOption should succeed"), bSelected);
	TestFalse(TEXT("CloseDialogue effect should close conversation"), Subsystem->IsConversationActive());
	TestFalse(TEXT("Conversation open flag should be false"), Subsystem->bConversationOpen);
	TestEqual(
		TEXT("Interrupt reason should be normal end"),
		Subsystem->ActiveSession.InterruptReason,
		EInterruptReason::NormalEnd
	);

	const FDialogueRuntimeState* UpdatedState = Subsystem->DialogueRuntimeMap.Find(DialogueID);
	TestNotNull(TEXT("Runtime state should still exist"), UpdatedState);

	if (UpdatedState)
	{
		TestTrue(TEXT("Visited option should still be recorded before closing"), UpdatedState->VisitedOptionIDs.Contains(OptionID));
	}

	return true;
}
#endif
