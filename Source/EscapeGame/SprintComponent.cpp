// Fill out your copyright notice in the Description page of Project Settings.


#include "SprintComponent.h"
// === 必须要加的头文件 ===
#include "GameFramework/Character.h" // 为了能用 ACharacter 的函数
#include "GameFramework/CharacterMovementComponent.h" // 为了能操作 MaxWalkSpeed
// 假设你的状态机组件在 statemachine 文件夹下，根据实际路径调整
// 如果是在同一级目录，直接写 "StateMachineComponent.h" 即可
#include "statemachine/StateMachineComponent.h" 
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
	bool bIsActurallySprinting = bSprintRequested && bCanSprint && !bStaminaDrained && MovementComp->IsMovingOnGround();

	
	if (bIsActurallySprinting) 
	{
		
		CurrentStamina -= StaminaConsumeRate * DeltaTime;

		StaminaRegenDelay = MaxStaminaRegenDelay;

		//移动组件 设置为冲刺速度
		MovementComp->MaxWalkSpeed = SprintSpeed;
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
			MovementComp->MaxWalkSpeed = WalkSpeed;
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
void USprintComponent::ApplyStaminaChange()
{
	// ...
	if (OnStaminaChanged.IsBound())
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}

	//OnStaminaChanged.Broadcast(CurrentStamina,MaxStamina);
}
