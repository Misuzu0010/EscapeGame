// Copyright Epic Games, Inc. All Rights Reserved.


#include "Character/EscapeGamePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Dialogue/DialogueQuestSubsystem.h"
#include "Inventory/InventoryComponent.h"
#include "Core/EscapeGame.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "Inventory/UI/InventoryMenuWidget.h"
#include "UI/DialogueWidget.h"

void AEscapeGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		DialogueSubsystem = GameInstance->GetSubsystem<UDialogueQuestSubsystem>();
		if (DialogueSubsystem)
		{
			DialogueSubsystem->OnConversationStarted.AddDynamic(this, &AEscapeGamePlayerController::HandleDialogueConversationStarted);
			DialogueSubsystem->OnConversationEnded.AddDynamic(this, &AEscapeGamePlayerController::HandleDialogueConversationEnded);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("对话 UI 绑定失败：找不到 UDialogueQuestSubsystem。"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("对话 UI 绑定失败：GameInstance 为空。"));
	}

	// only spawn touch controls on local player controllers
	if (SVirtualJoystick::ShouldDisplayTouchInterface() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogEscapeGame, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AEscapeGamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}
void AEscapeGamePlayerController::ToggleInventoryUI()
{
    // 1. 如果 UI 还没创建，先创建

    UE_LOG(LogTemp, Warning, TEXT("控制器收到开包指令！")); // 第一步检查

    if (!InventoryMenuClass)
    {
        UE_LOG(LogTemp, Error, TEXT("笨蛋！你忘了在蓝图里设置 InventoryMenuClass！"));
        return;
    }

    if (!InventoryMenuInstance && InventoryMenuClass)
    {
        InventoryMenuInstance = CreateWidget<UUserWidget>(this, InventoryMenuClass);

        // --- 初始化 UI 数据 ---
        // 控制器要去问控制的 Pawn：“你身上有背包组件吗？”
        APawn* ControlledPawn = GetPawn();
        if (ControlledPawn && InventoryMenuInstance)
        {
            UInventoryComponent* InvComp = ControlledPawn->FindComponentByClass<UInventoryComponent>();
            UInventoryMenuWidget* MenuWidget = Cast<UInventoryMenuWidget>(InventoryMenuInstance);
            if (MenuWidget && InvComp)
            {
                MenuWidget->InitializeInventory(InvComp);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("背包 UI 初始化警告：MenuWidget=%s, InventoryComponent=%s, Pawn=%s。"),
                    *GetNameSafe(MenuWidget),
                    *GetNameSafe(InvComp),
                    *GetNameSafe(ControlledPawn));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("背包 UI 初始化警告：ControlledPawn=%s, InventoryMenuInstance=%s。"),
                *GetNameSafe(ControlledPawn),
                *GetNameSafe(InventoryMenuInstance));
        }
    }

    if (!InventoryMenuInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("背包 UI 切换失败：InventoryMenuInstance 创建失败，Class=%s。"),
            InventoryMenuClass ? *InventoryMenuClass->GetName() : TEXT("None"));
        return;
    }

    // 2. 根据当前状态切换
    if (InventoryMenuInstance->IsInViewport())
    {
        // 如果开着，就关掉
        SetInventoryVisibility(false);
    }
    else
    {
        // 如果关着，就打开
        SetInventoryVisibility(true);
    }
}

void AEscapeGamePlayerController::SetInventoryVisibility(bool bVisible)
{
    if (!InventoryMenuInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("设置背包可见性失败：InventoryMenuInstance 为空，bVisible=%s。"),
            bVisible ? TEXT("true") : TEXT("false"));
        return;
    }

    if (bVisible)
    {
        InventoryMenuInstance->AddToViewport();

        // === 这里的代码完全是你原来的逻辑，只是换了个地方 ===
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(InventoryMenuInstance->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

        SetInputMode(InputMode);
        bShowMouseCursor = true;

        // 既然是 Controller，直接设置自己就行，不用 GetController() 了
        SetIgnoreMoveInput(true);
        SetIgnoreLookInput(true);
    }
    else
    {
        InventoryMenuInstance->RemoveFromParent();

        FInputModeGameOnly InputMode;
        SetInputMode(InputMode);
        bShowMouseCursor = false;

        SetIgnoreMoveInput(false);
        SetIgnoreLookInput(false);
    }
}

void AEscapeGamePlayerController::HandleDialogueConversationStarted(const FConversationSession& Session)
{
	if (!Session.bIsActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("对话 UI 打开失败：收到的 Session 不是激活状态。DialogueID=%s。"),
			*Session.DialogueID.ToString());
		return;
	}

	ShowDialogueUI();
}

void AEscapeGamePlayerController::HandleDialogueConversationEnded(EInterruptReason Reason)
{
	HideDialogueUI();
}

void AEscapeGamePlayerController::ShowDialogueUI()
{
	if (!DialogueWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("对话 UI 打开失败：请在 PlayerController 蓝图里设置 DialogueWidgetClass。"));
		return;
	}

	if (!DialogueSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("对话 UI 打开失败：DialogueSubsystem 为空。"));
		return;
	}

	if (!DialogueWidgetInstance)
	{
		DialogueWidgetInstance = CreateWidget<UDialogueWidget>(this, DialogueWidgetClass);
		if (!DialogueWidgetInstance)
		{
			UE_LOG(LogTemp, Error, TEXT("对话 UI 打开失败：CreateWidget 返回空，Class=%s。"), *GetNameSafe(DialogueWidgetClass));
			return;
		}
	}

	DialogueWidgetInstance->InitializeDialogueWidget(DialogueSubsystem);

	if (!DialogueWidgetInstance->IsInViewport())
	{
		DialogueWidgetInstance->AddToViewport();
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(DialogueWidgetInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;

	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
}

void AEscapeGamePlayerController::HideDialogueUI()
{
	if (DialogueWidgetInstance && DialogueWidgetInstance->IsInViewport())
	{
		DialogueWidgetInstance->RemoveFromParent();
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;

	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);
}
