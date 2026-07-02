// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dialogue/EscapeDialogueTypes.h"
#include "GameFramework/PlayerController.h"
#include "EscapeGamePlayerController.generated.h"

class UInventoryComponent;
class UInputMappingContext;
class UUserWidget;
class UDialogueQuestSubsystem;
class UDialogueWidget;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class AEscapeGamePlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	TObjectPtr<UUserWidget> MobileControlsWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> InventoryMenuClass;

	// 运行时保存的实例
	UPROPERTY()
	UUserWidget* InventoryMenuInstance;

	// 对话 UI 蓝图类。需要在 PlayerController 蓝图里设置为继承 UDialogueWidget 的 WBP。
	UPROPERTY(EditDefaultsOnly, Category = "UI|Dialogue")
	TSubclassOf<UDialogueWidget> DialogueWidgetClass;

	// 当前正在显示的对话 UI 实例。
	UPROPERTY()
	TObjectPtr<UDialogueWidget> DialogueWidgetInstance;

	// 运行时对话子系统引用，用来绑定开始/结束会话事件。
	UPROPERTY()
	TObjectPtr<UDialogueQuestSubsystem> DialogueSubsystem;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	// 辅助函数：专门用来开关
	void SetInventoryVisibility(bool bVisible);

	UFUNCTION()
	void HandleDialogueConversationStarted(const FConversationSession& Session);

	UFUNCTION()
	void HandleDialogueConversationEnded(EInterruptReason Reason);

	void ShowDialogueUI();
	void HideDialogueUI();
public:
	UFUNCTION()
	void ToggleInventoryUI();


};
