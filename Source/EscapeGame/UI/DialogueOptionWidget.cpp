// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/DialogueOptionWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UDialogueOptionWidget::InitializeOption(const FGameplayTag& InOptionID, const FText& InOptionText)
{
	OptionID = InOptionID;

	if (OptionTextBlock)
	{
		OptionTextBlock->SetText(InOptionText);
	}

	if (OptionButton && !OptionButton->OnClicked.IsAlreadyBound(this, &UDialogueOptionWidget::HandleButtonClicked))
	{
		OptionButton->OnClicked.AddDynamic(this, &UDialogueOptionWidget::HandleButtonClicked);
	}
}

void UDialogueOptionWidget::HandleButtonClicked()
{
	OnOptionClicked.Broadcast(OptionID);
}
