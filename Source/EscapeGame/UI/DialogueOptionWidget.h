// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"

#include "DialogueOptionWidget.generated.h"

class UButton;
class UTextBlock;
class UDialogueParticipantComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueOptionClicked, FGameplayTag, OptionID);

UCLASS()
class ESCAPEGAME_API UDialogueOptionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void InitializeOption(const FGameplayTag& InOptionID, const FText& InOptionText);

	UPROPERTY(BlueprintAssignable, Category="Dialogue")
	FOnDialogueOptionClicked OnOptionClicked;

protected:
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> OptionButton;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> OptionTextBlock;

private:
	UPROPERTY()
	FGameplayTag OptionID;

	UFUNCTION()
	void HandleButtonClicked();
};
