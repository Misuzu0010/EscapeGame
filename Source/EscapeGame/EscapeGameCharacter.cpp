// Copyright Epic Games, Inc. All Rights Reserved.

#include "EscapeGameCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include"InventoryMenuWidget.h"
#include"EscapeCombatComponent.h"
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
#include"EscapeGameplayTags.h"
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
	
	EscapeCombatComp=CreateDefaultSubobject<UEscapeCombatComponent>(TEXT("EscapeCombatComp"));

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
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AEscapeGameCharacter::StartCrouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AEscapeGameCharacter::StopCrouch);
		
		
		// 注意第三个参数是 InterectComp，第四个参数是组件的函数地址			
		EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, InteractComp, &UInterectComponent::RequestToggleInventory);

		// 交互键也是同理
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, InteractComp, &UInterectComponent::OnInteract);

		//切换视角
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Started, this, &AEscapeGameCharacter::ToggleCameraMode);
		//使用物品
		EnhancedInputComponent->BindAction(UseItemAction, ETriggerEvent::Started, this, &AEscapeGameCharacter::Input_UseItem);
		//连击函数
		EnhancedInputComponent->BindAction(AttackAction,ETriggerEvent::Started,EscapeCombatComp,&UEscapeCombatComponent::Input_AttackStarted);
		//EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, EscapeCombatComp, &UEscapeCombatComponent::BeginOrUpdateChargedAttack);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, EscapeCombatComp, &UEscapeCombatComponent::Input_AttackCompleted);
	    
	}
	else
	{
		UE_LOG(LogEscapeGame, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AEscapeGameCharacter::BeginPlay()
{
    Super::BeginPlay();

    // ==========================================
    // 🛠️ 【基础调试】核心挂载组件状态大排查
    // ==========================================
    UE_LOG(LogTemp, Warning, TEXT("=========== 香子兰组件体检开始 ==========="));

    if (StateMachineComp)
    {
        UE_LOG(LogTemp, Log, TEXT("✅ 状态机组件挂载成功！类名: %s"), *StateMachineComp->GetClass()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ 警报！StateMachineComp 是空指针(nullptr)喵！"));
    }

    if (SprintComp)
    {
        UE_LOG(LogTemp, Log, TEXT("✅ 冲刺组件挂载成功！类名: %s"), *SprintComp->GetClass()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ 警报！SprintComp 是空指针(nullptr)喵！"));
    }

    if (AttributeComp)
    {
        UE_LOG(LogTemp, Log, TEXT("✅ 属性组件挂载成功！类名: %s"), *AttributeComp->GetClass()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ 警报！AttributeComp 是空指针(nullptr)喵！"));
    }

    if (InventoryComp)
    {
        UE_LOG(LogTemp, Log, TEXT("✅ 背包组件挂载成功！类名: %s"), *InventoryComp->GetClass()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ 警报！InventoryComp 是空指针(nullptr)喵！"));
    }

    UE_LOG(LogTemp, Warning, TEXT("========================================"));


    // ==========================================
    // 基础业务逻辑层：本地玩家创建 UI
    // ==========================================
    UE_LOG(LogTemp, Warning, TEXT("HUD诊断：IsLocallyControlled=%s, HUDWidgetClass=%s, Controller=%s"),
        IsLocallyControlled() ? TEXT("true") : TEXT("false"),
        HUDWidgetClass ? *HUDWidgetClass->GetName() : TEXT("None"),
        GetController() ? *GetController()->GetName() : TEXT("None"));

    if (IsLocallyControlled() && HUDWidgetClass)
    {
        AEscapeGamePlayerController* PC = Cast<AEscapeGamePlayerController>(GetController());
        UGameHUDWidget* HUD = CreateWidget<UGameHUDWidget>(GetWorld(), HUDWidgetClass);
        if (HUD)
        {
            UE_LOG(LogTemp, Warning, TEXT("HUD诊断：CreateWidget 成功，准备 AddToViewport。Widget=%s"), *HUD->GetName());
            HUD->AddToViewport(); //
            UE_LOG(LogTemp, Warning, TEXT("HUD诊断：AddToViewport 已执行，准备 InitializeWidget。"));
            HUD->InitializeWidget(AttributeComp, SprintComp, InventoryComp); //
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("HUD诊断：CreateWidget 失败。请检查 HUDWidgetClass 是否继承自 GameHUDWidget。"));
        }

        if (PC && InteractComp)
        {
            // 当组件喊话时 -> 自动调用控制器的 ToggleInventoryUI
            InteractComp->OnRequestToggleInventory.AddDynamic(PC, &AEscapeGamePlayerController::ToggleInventoryUI); //
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HUD诊断：跳过 HUD 创建。原因可能是非本地控制角色，或 HUDWidgetClass 未配置。"));
    }
}
void AEscapeGameCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	// 【核心修复】：只要轴向有输入，说明玩家正在尝试位移
	if (!MovementVector.IsZero() && StateMachineComp)
	{
		ECharacterState CurrentState = StateMachineComp->GetCurrentState();
		// 只有当前是 Idle 时才切为 Moving，绝对不能打断 Attacking、Sprinting、Stunned 等高优先级状态
		if (CurrentState == ECharacterState::Idle)
		{
			StateMachineComp->SetState(ECharacterState::Moving);
		}
	}

	// 保持原有的底层移动路由不变
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
	
	if (EscapeCombatComp)
	{
		// 【正确的做法】：通过组件的接口去查 Tag！
		if (EscapeCombatComp->HasCombatTag(EscapeGameplayTags::Action_State_Attacking) || 
			EscapeCombatComp->HasCombatTag(EscapeGameplayTags::Action_Combat_Heavy_Charge) ||
			EscapeCombatComp->HasCombatTag(EscapeGameplayTags::Action_ChargedAttack_Release))
		{
			UE_LOG(LogTemp, Warning, TEXT("香子兰嘲讽：正在挥刀呢，跳什么跳！"));
			return;
		}
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
	// 【安全检查1】获取 CharacterMovement，必须判空
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (!MovementComp) return;

	// 【安全检查2】只有在非下落时才能蹲伏
	if (MovementComp->IsFalling()) return;

	// 【安全检查3】检查 SprintComp 是否有效
	if (SprintComp && SprintComp->bSprintRequested) return;

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

void AEscapeGameCharacter::Input_UseItem(const FInputActionValue& Value)
{
	// 1. 先判断布尔值，确保按键确实触发了 (虽然 Started 肯定是触发了)
	const bool bIsPressed = Value.Get<bool>();

	if (bIsPressed && InventoryComp) // 别忘了判空！
	{
		// 2. 这里就是“桥梁”！
		// 角色知道 CurrentSelectedSlotIndex 是多少，把它传给组件
		InventoryComp->UseItem(CurrentSelectedSlotIndex);
	}
}

void AEscapeGameCharacter::ApplyDamage_Implementation(float DamageValue, AActor* InstigatorActor, const FVector& HitLocation, const FVector& HitImpulse)
{
	AttributeComp->ApplyHealthChange(-FMath::Max(0.f,DamageValue));
	if (AttributeComp->CurrentHealth<=0)
	{
		StateMachineComp->ApplyDeath();
	}
	GetCharacterMovement()->AddImpulse(HitImpulse);


}
