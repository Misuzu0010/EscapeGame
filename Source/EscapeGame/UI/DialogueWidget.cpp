#include "UI/DialogueWidget.h"

#include "DialogueParticipantComponent.h"
#include "Components/PanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Dialogue/DialogueQuestSubsystem.h"
#include "UI/DialogueOptionWidget.h"

void UDialogueWidget::InitializeDialogueWidget(UDialogueQuestSubsystem* InDialogueSubsystem)
{
	DialogueSubsystem = InDialogueSubsystem;
	RefreshFromDialogue();
}

void UDialogueWidget::RefreshFromDialogue()
{
	if (!DialogueSubsystem)
	{
		UE_LOG(LogTemp,Error, TEXT("对话子系统未能正常载入"));
		return ;
	}
	
	if (!DialogueSubsystem->IsConversationActive())
	{
		UE_LOG(LogTemp, Error, TEXT("对话未被打开"));
		return;
	}
	FDialogueNode CurrentNode = DialogueSubsystem->GetCurrentNode();
	
	if (!CurrentNode.NodeID.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("当前的节点ID不支持或者非法"));
		return;
	}
	if (SpeakerNameTextBlock)
	{
		const UDialogueParticipantComponent* Participant = DialogueSubsystem->GetActiveParticipantComponent();

		if (Participant && Participant->ParticipantID == CurrentNode.SpeakerID && !Participant->DisplayName.IsEmpty())
		{
			SpeakerNameTextBlock->SetText(Participant->DisplayName);
		}
		else
		{
			SpeakerNameTextBlock->SetText(FText::FromString(CurrentNode.SpeakerID.ToString()));
		}
	}

	if (SpeakerContextTextBlock)
	{
		SpeakerContextTextBlock->SetText(CurrentNode.SpeakerText);
	}

	if (!OptionsBox)
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueWidget 刷新失败：OptionsBox 未绑定。"));
		return;
	}
	OptionsBox->ClearChildren();
	
	if (!OptionWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("选项控件暂未绑定"));
		return;
	}
	
	TArray<FDialogueOption> AvailableOptions = DialogueSubsystem->GetAvailableOptions();
	UE_LOG(LogTemp, Warning, TEXT("DialogueWidget 选项诊断：CurrentNode.Options=%d, AvailableOptions=%d"),
		CurrentNode.Options.Num(),
		AvailableOptions.Num());
	
	for (const FDialogueOption& Option : AvailableOptions)
	{
		UDialogueOptionWidget* OptionWidget = CreateWidget<UDialogueOptionWidget>(this, OptionWidgetClass);
		if (!OptionWidget)
		{
			UE_LOG(LogTemp, Error, TEXT("DialogueWidget 创建选项失败：OptionID=%s。"), *Option.OptionID.ToString());
			continue;
		}
		OptionWidget->InitializeOption(Option.OptionID, Option.OptionText);
		UPanelSlot* AddedSlot = OptionsBox->AddChild(OptionWidget);
		UE_LOG(LogTemp, Warning, TEXT("DialogueWidget 添加选项：OptionID=%s, Text=%s, Children=%d, AddedChild=%s"),
			*Option.OptionID.ToString(),
			*Option.OptionText.ToString(),
			OptionsBox->GetChildrenCount(),
			*GetNameSafe(AddedSlot));

		OptionWidget->OnOptionClicked.AddDynamic(this, &UDialogueWidget::HandleDialogueOptionClicked);
	}
}

void UDialogueWidget::HandleDialogueOptionClicked(FGameplayTag OptionID)
{
	if (!DialogueSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("子系统无效"));
		return;
	}
	if (!DialogueSubsystem->SelectOption(OptionID))
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueWidget 选择选项失败：SelectOption 返回 false。OptionID=%s"), *OptionID.ToString());
		return;
	}
	if (DialogueSubsystem->IsConversationActive())
	{
		RefreshFromDialogue();
	}
	
}
