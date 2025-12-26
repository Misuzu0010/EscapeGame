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
#include"EscapeGamePlayerController.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include"Interface/PickupInterface.h"
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

	// Create first person camera
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FIrstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetMesh(),FName("head"));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->SetRelativeLocation(FVector(10.0f,0.0f,0.0f));
	FirstPersonCameraComponent->SetActive(false);
	bIsFirstPerson = false;
	// 创建状态机组件
	StateMachineComp = CreateDefaultSubobject<UStateMachineComponent>(TEXT("StateMachineComp"));
	// 创建冲刺组件
	SprintComp = CreateDefaultSubobject<USprintComponent>(TEXT("SprintComp"));
	// 创建属性组件
	AttributeComp = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComp"));
	// 创建背包组件
	InventoryComp = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComp"));

	InteractComp = CreateDefaultSubobject<UInterectComponent>(TEXT("InteractComp"));

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
}

void AEscapeGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AEscapeGameCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AEscapeGameCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AEscapeGameCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AEscapeGameCharacter::Look);

		
		
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, SprintComp, &USprintComponent::StartSprinting);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, SprintComp, &USprintComponent::StopSprinting);
		
		
		
			// 在 SetupPlayerInputComponent 里绑定
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AEscapeGameCharacter::StartCrouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AEscapeGameCharacter::StopCrouch);
		
		
			// 注意第三个参数是 InterectComp，第四个参数是组件的函数地址			
		EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, InteractComp, &UInterectComponent::RequestToggleInventory);

			// 交互键也是同理
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, InteractComp, &UInterectComponent::OnInteract);
		

		//捡起物品

		//切换视角
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Started, this, &AEscapeGameCharacter::ToggleCameraMode);
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
		AEscapeGamePlayerController* PC = Cast<AEscapeGamePlayerController>(GetController());
		UGameHUDWidget* HUD = CreateWidget<UGameHUDWidget>(GetWorld(), HUDWidgetClass);
		if (HUD)
		{
			HUD->AddToViewport(); // 【关键】这句没写就是隐形的！
			HUD->InitializeWidget(AttributeComp,SprintComp,InventoryComp);
		}
		if (PC && InteractComp)
		{
			// 【关键连线】
			// 当组件喊话时 -> 自动调用控制器的 ToggleInventoryUI
			InteractComp->OnRequestToggleInventory.AddDynamic(PC, &AEscapeGamePlayerController::ToggleInventoryUI);
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
	// 1. 查岗：如果被定身（打开了背包），直接无视跳跃请求
	if (Controller && Controller->IsMoveInputIgnored())
	{
		return;
	}
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
void AEscapeGameCharacter::ToggleCameraMode()
{
	bIsFirstPerson = !bIsFirstPerson;
	if (bIsFirstPerson)
	{	
		FollowCamera->SetActive(false);
		FirstPersonCameraComponent->SetActive(true);
		bUseControllerRotationYaw = true;
		
	}
	else
	{
		FollowCamera->SetActive(true);
		FirstPersonCameraComponent->SetActive(false);
		bUseControllerRotationYaw = false;
	}
}