#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "DialogueWidget.generated.h"

class UDialogueQuestSubsystem;
class UDialogueOptionWidget;
class UTextBlock;
class UVerticalBox;


UCLASS()
class ESCAPEGAME_API UDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void InitializeDialogueWidget(UDialogueQuestSubsystem* InDialogueSubsystem);
	
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void RefreshFromDialogue();
	
	UFUNCTION()
	void HandleDialogueOptionClicked(FGameplayTag OptionID);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	TObjectPtr<UDialogueQuestSubsystem> DialogueSubsystem;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> OptionsBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SpeakerNameTextBlock;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SpeakerContextTextBlock;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue")
	TSubclassOf<UDialogueOptionWidget> OptionWidgetClass;
	
	
};
