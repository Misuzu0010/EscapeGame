// Fill out your copyright notice in the Description page of Project Settings.


#include "SprintComponent.h"
#include "GameFramework/Character.h" 
#include "GameFramework/CharacterMovementComponent.h" 
#include "statemachine/StateMachineComponent.h" 
#include"TimerManager.h"
// ========================

// Sets default values for this component's properties
USprintComponent::USprintComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bStaminaDrained = false;
	bSprintRequested = false;
	CurrentStamina = MaxStamina;

	// ...
}
void USprintComponent::SetSpeedBuffMultiplier(float NewMultiplier)
{
	CurrentBuffMultiplier = NewMultiplier;

	// 每次 Buff 改变，立刻更新当前速度
	UpdateMovementSpeed();

	UE_LOG(LogTemp, Log, TEXT("喵！速度倍率变了: %f"), CurrentBuffMultiplier);
}

void USprintComponent::UpdateMovementSpeed() 
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)return;

	UCharacterMovementComponent* CharMoveComp = Character->GetCharacterMovement();
	if (!CharMoveComp)return;

	float BaseSpeed = bIsActurallySprinting ? SprintSpeed : WalkSpeed;

	CharMoveComp->MaxWalkSpeed = BaseSpeed * CurrentBuffMultiplier;


}

void USprintComponent::StartSpeedBuff(float Duration, float Multiplier)
{
	// 1. 设置倍率
	SetSpeedBuffMultiplier(Multiplier);

	// 2. 设置闹钟：时间到了就把倍率改回 1.0
	FTimerDelegate TimerDel;
	TimerDel.BindUObject(this, &USprintComponent::SetSpeedBuffMultiplier, 1.0f); // 恢复成 1.0

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Buff, TimerDel, Duration, false);
}
// Called when the game starts
void USprintComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter) 
	{
		MovementComp=OwnerCharacter->GetCharacterMovement();
		// 初始化速度
		if (MovementComp) 
		{
			MovementComp->MaxWalkSpeed = WalkSpeed;
			StateMachine = OwnerCharacter->FindComponentByClass<UStateMachineComponent>();

			CurrentStamina = MaxStamina;
			if (MovementComp)MovementComp->MaxWalkSpeed = WalkSpeed;
		}
	}
	
}


// Called every frame
void USprintComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerCharacter || !MovementComp || !StateMachine)return;
	float OldStamina = CurrentStamina;
	if (CurrentStamina <= 0.0f)
	{
		CurrentStamina = 0.0f;

		bStaminaDrained = true;
		//// 停止冲刺
		MovementComp->MaxWalkSpeed = WalkSpeed;
		StateMachine->SetState(ECharacterState::Idle);
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Stamina Depleted!"));
	}


	//判断是否冲刺
	ECharacterState CurrentState = StateMachine->GetCurrentState();
	//仅当移动或者空闲 且有速度 允许冲刺
	bool bCanSprint = ((CurrentState == ECharacterState::Moving || CurrentState == ECharacterState::Idle) && !OwnerCharacter->GetVelocity().IsZero());

	// 实际冲刺条件
	bIsActurallySprinting = bSprintRequested && bCanSprint && !bStaminaDrained && MovementComp->IsMovingOnGround()&&!OwnerCharacter->bIsCrouched;

	
	if (bIsActurallySprinting) 
	{
		
		CurrentStamina -= StaminaConsumeRate * DeltaTime;

		StaminaRegenDelay = MaxStaminaRegenDelay;

		//移动组件 设置为冲刺速度
		MovementComp->MaxWalkSpeed = SprintSpeed*CurrentBuffMultiplier;
		GEngine->AddOnScreenDebugMessage(4, 0.f, FColor::Purple,
			FString::Printf(TEXT("Actual Velocity: %.1f"), OwnerCharacter->GetVelocity().Size()));
		GEngine->AddOnScreenDebugMessage(3, 0.f, FColor::Red,
			FString::Printf(TEXT("MovementMode: %d"), (int32)MovementComp->MovementMode));

		// 设置状态机 (防止每帧重复Set，加个判断)
		if (CurrentState != ECharacterState::Sprinting)
		{
			StateMachine->SetState(ECharacterState::Sprinting);

		}
	
	}
	else 
	{
		if (!bSprintRequested) 
		{
			MovementComp->MaxWalkSpeed = WalkSpeed*CurrentBuffMultiplier;
			if (StaminaRegenDelay > 0.0f) 
			{
				StaminaRegenDelay -= DeltaTime;		
			}

			else 
			{
				if (CurrentStamina < MaxStamina) 
				{
					CurrentStamina += StaminaRegenRate * DeltaTime;
					GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(TEXT("Recovering.....: %.2f"), CurrentStamina));
					GEngine->AddOnScreenDebugMessage(0, 0.f, FColor::Cyan,
						FString::Printf(TEXT("WalkSpeed: %.1f | SprintSpeed: %.1f"), WalkSpeed, SprintSpeed));

					GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Yellow,
						FString::Printf(TEXT("Actual MaxWalkSpeed: %.1f"), MovementComp->MaxWalkSpeed));

					GEngine->AddOnScreenDebugMessage(2, 0.f, FColor::Green,
						FString::Printf(TEXT("bSprintRequested: %s | bStaminaDrained: %s"),
							bSprintRequested ? TEXT("true") : TEXT("false"),
							bStaminaDrained ? TEXT("true") : TEXT("false")));
					GEngine->AddOnScreenDebugMessage(4, 0.f, FColor::Purple,
						FString::Printf(TEXT("Actual Velocity: %.1f"), OwnerCharacter->GetVelocity().Size()));
					GEngine->AddOnScreenDebugMessage(3, 0.f, FColor::Red,
						FString::Printf(TEXT("MovementMode: %d"), (int32)MovementComp->MovementMode));

				}

				if (CurrentStamina > 10.0f)bStaminaDrained = false;
			}
		}
		// 限制范围
		CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);

		// =======================
		//      状态回退逻辑
		// =======================
		// 只有当前是 Sprinting 才需要回退，不要干扰 Jumping/Attacking
		if (CurrentState == ECharacterState::Sprinting)
		{
			// 如果速度很小，切回 Idle，否则切回 Moving
			if (OwnerCharacter->GetVelocity().SizeSquared() < 10.0f)
			{
				StateMachine->SetState(ECharacterState::Idle);
			}
			else
			{
				StateMachine->SetState(ECharacterState::Moving);
			}
		}
	}
	if (!FMath::IsNearlyEqual(OldStamina, CurrentStamina, 0.01f))
	{
		// 只有真的变了，才通知 UI
		ApplyStaminaChange();
		// 调试用：只有变化时才会打印，刷屏会少很多
		// UE_LOG(LogTemp, Warning, TEXT("Stamina Changed: %.2f"), CurrentStamina);
	}

}

void USprintComponent::StartSprinting()
{
	bSprintRequested = true;
}

void USprintComponent::StopSprinting()
{
	bSprintRequested = false;
}

float USprintComponent::GetCurrentStaminaPercent() const
{
	return CurrentStamina / MaxStamina;


}
void USprintComponent::StaminaChange(float Delta)
{
	// ...

	CurrentStamina += Delta;
	if (CurrentStamina > 100.0f)CurrentStamina = 100.0f;
	if (OnStaminaChanged.IsBound())
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}

	//OnStaminaChanged.Broadcast(CurrentStamina,MaxStamina);
}

void USprintComponent::ApplyMaxChange(float Delta)
{
	// ...
	MaxStamina += Delta;
	if (MaxStamina < 0.0f)MaxStamina = 0.0f;
	if (OnStaminaChanged.IsBound())
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
}

void USprintComponent::ApplyStaminaChange()
{
	if (OnStaminaChanged.IsBound())
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
}