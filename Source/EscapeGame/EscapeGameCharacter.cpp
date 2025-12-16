// Copyright Epic Games, Inc. All Rights Reserved.

#include "EscapeGameCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include"InventoryMenuWidget.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "EscapeGame.h"
#include"Blueprint/UserWidget.h"
#include "SprintComponent.h"
#include "GameHUDWidget.h" 
#include "statemachine/StateMachineComponent.h"

AEscapeGameCharacter::AEscapeGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	StateMachineComp = CreateDefaultSubobject<UStateMachineComponent>(TEXT("StateMachineComp"));

	SprintComp = CreateDefaultSubobject<USprintComponent>(TEXT("SprintComp"));

	AttributeComp = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComp"));

	InventoryComp = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComp"));

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	//SprintComp = CreateDefaultSubobject<USprintComponent>(TEXT("SprintComp"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	//  // 初始化状态
	//if (StateMachineComp) // 加个判断是个好习惯，虽然在构造函数里通常都有
	//{
	//	StateMachineComp->CurrentState = ECharacterState::Idle;
	//	StateMachineComp->bCanMove = true;
	//	StateMachineComp->bCanAttack = true;
	//}
}

void AEscapeGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AEscapeGameCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AEscapeGameCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AEscapeGameCharacter::Look);

		if (SprintAction) 
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, SprintComp, &USprintComponent::StartSprinting);

			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, SprintComp, &USprintComponent::StopSprinting);
		}
		if (CrouchAction) 
		{
			// 在 SetupPlayerInputComponent 里绑定
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AEscapeGameCharacter::StartCrouch);
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AEscapeGameCharacter::StopCrouch);
		}
		if (InventoryAction)
		{
			// 意思是：当按下 I 键，调用 "this" (我自己/角色) 身上的 ToggleInventory 函数
			EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &AEscapeGameCharacter::ToggleInventory);
		}




		// === 你需要在这里绑定冲刺和攻击 ===
	    //假设你有 SprintAction 和 AttackAction
	    //EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, SprintComp, &USprintComponent::StartSprinting);
	    //EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, SprintComp, &USprintComponent::StopSprinting);
	    //EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ECharacterState::);
	
	}
	else
	{
		UE_LOG(LogEscapeGame, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AEscapeGameCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 只有本地玩家才创建UI
	if (IsLocallyControlled() && HUDWidgetClass)
	{
		UGameHUDWidget* HUD = CreateWidget<UGameHUDWidget>(GetWorld(), HUDWidgetClass);
		if (HUD)
		{
			HUD->AddToViewport(); // 【关键】这句没写就是隐形的！
			HUD->InitializeWidget(AttributeComp,SprintComp,InventoryComp);
		}
	}
}
void AEscapeGameCharacter::Move(const FInputActionValue& Value)
{
	//if (!StateMachineComp->bCanMove)return;
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AEscapeGameCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AEscapeGameCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
		GEngine->AddOnScreenDebugMessage(7, 0.f, FColor::White,
			FString::Printf(TEXT("Input - Right: %.2f | Forward: %.2f"), Right, Forward));
	}
}

void AEscapeGameCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AEscapeGameCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AEscapeGameCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AEscapeGameCharacter::StartCrouch()
{
	if (GetCharacterMovement()->IsFalling())return;
	if (SprintComp->bSprintRequested)return;
	Crouch();
}

void AEscapeGameCharacter::StopCrouch()
{
	UnCrouch();
}

void AEscapeGameCharacter::ToggleInventory()
{
	APlayerController* PC = Cast<APlayerController>(GetController());

	//防止没有设置UI类或者PC为空时崩溃
	if (!PC || !InventoryMenuClass) return;

	// 如果窗口不存在，就创建它
	if (!InventoryMenuInstance)
	{
		// 1. 创建 Widget
		InventoryMenuInstance = CreateWidget<UUserWidget>(PC, InventoryMenuClass);

		// 2. 强转并初始化 (解除封印！)
		if (InventoryMenuInstance)
		{
			// 因为我们引用了头文件，所以可以用 UInventoryMenuWidget
			UInventoryMenuWidget* MenuWidget = Cast<UInventoryMenuWidget>(InventoryMenuInstance);
			if (MenuWidget)
			{
				// 把身上的背包组件传给 UI
				MenuWidget->InitializeInventory(InventoryComp);
			}
		}
	}

// --- 下面是必须补上的逻辑 ---

	if (InventoryMenuInstance) // 再次确认一下有东西
	{
		// 判断当前是在屏幕上显示着，还是藏着
		if (InventoryMenuInstance->IsInViewport())
		{
			// === 如果开着，就关掉 ===
			InventoryMenuInstance->RemoveFromParent(); // 从屏幕移除

			// 把鼠标藏起来，控制权还给游戏角色
			FInputModeGameOnly GameMode;
			PC->SetInputMode(GameMode);
			PC->bShowMouseCursor = false;
		}
		else
		{
			// === 如果关着，就打开 ===
			InventoryMenuInstance->AddToViewport(); // 贴到屏幕上

			// 把鼠标显示出来，控制权交给 UI
			FInputModeGameAndUI UIMode;
			UIMode.SetWidgetToFocus(InventoryMenuInstance->TakeWidget()); // 让UI获得焦点
			UIMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(UIMode);
			PC->bShowMouseCursor = true;
		}
	}

	
}