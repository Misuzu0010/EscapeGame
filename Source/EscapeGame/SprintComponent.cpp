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
	//UpdateMovementSpeed();

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

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter) 
	{
		MovementComp = OwnerCharacter->GetCharacterMovement();
		StateMachine = OwnerCharacter->FindComponentByClass<UStateMachineComponent>();
		// 🚨 硬核 Debug 拦截：如果组件挂载失败，立刻在控制台高亮报错
		ensureMsgf(StateMachine, TEXT("香子兰警报：角色身上找不到状态机组件喵！"));
		CurrentStamina = MaxStamina;
		// 【核心修复】：刚开局时，必须主动触发一次多播代理广播，让 UI 刷新显示体力值！
		ApplyStaminaChange();

		if (MovementComp)
		{
			//MovementComp->MaxWalkSpeed = WalkSpeed;
			CurrentSmoothedSpeed=WalkSpeed*CurrentBuffMultiplier;
			MovementComp->MaxWalkSpeed = CurrentSmoothedSpeed;
		}
	}
}


// Called every frame
// 在 SprintComponent.cpp 中
void USprintComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerCharacter || !MovementComp || !StateMachine) return;
	float OldStamina = CurrentStamina;

	// 1. 体力耗尽检测
	if (CurrentStamina <= 0.0f)
	{
		CurrentStamina = 0.0f;
		bStaminaDrained = true;
		MovementComp->MaxWalkSpeed = WalkSpeed * CurrentBuffMultiplier;

		// 只有当前正在冲刺，体力干涸了才退回 Moving
		if (StateMachine->GetCurrentState() == ECharacterState::Sprinting)
		{
			StateMachine->SetState(ECharacterState::Moving);
		}
	}

	ECharacterState CurrentState = StateMachine->GetCurrentState();
	const bool bHasMovement = !OwnerCharacter->GetVelocity().IsZero();
	const bool bCanStartSprint = CurrentState == ECharacterState::Moving || CurrentState == ECharacterState::Idle;
	const bool bCanKeepSprint = CurrentState == ECharacterState::Sprinting;

	// 开始冲刺和维持冲刺分开判断，避免进入 Sprinting 后下一帧被自己打断。
	bIsActurallySprinting = bSprintRequested && (bCanStartSprint || bCanKeepSprint) && bHasMovement && !bStaminaDrained && MovementComp->IsMovingOnGround() && !OwnerCharacter->bIsCrouched;
	const float TargetSpeed = (bIsActurallySprinting? SprintSpeed: WalkSpeed)*CurrentBuffMultiplier;
	CurrentSmoothedSpeed=FMath::FInterpTo(CurrentSmoothedSpeed,TargetSpeed,DeltaTime,SpeedInterpRate);
	MovementComp->MaxWalkSpeed = CurrentSmoothedSpeed;
	
	if (bIsActurallySprinting) 
	{
		CurrentStamina -= StaminaConsumeRate * DeltaTime;
		StaminaRegenDelay = MaxStaminaRegenDelay;
		//MovementComp->MaxWalkSpeed = SprintSpeed * CurrentBuffMultiplier;
		
		if (CurrentState != ECharacterState::Sprinting)
		{
			StateMachine->SetState(ECharacterState::Sprinting);
		}
	}
	else 
	{
		//MovementComp->MaxWalkSpeed = WalkSpeed * CurrentBuffMultiplier;
		if (!bSprintRequested) 
		{
			if (StaminaRegenDelay > 0.0f) StaminaRegenDelay -= DeltaTime;
			else 
			{
				if (CurrentStamina < MaxStamina) CurrentStamina += StaminaRegenRate * DeltaTime;
				if (CurrentStamina > 10.0f) bStaminaDrained = false;
			}
		}
		CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);

		// =======================================================
		// 【认知纠偏核心修正】：冲刺组件只负责“退款”冲刺状态，不干扰别的主动状态！
		// =======================================================
		if (CurrentState == ECharacterState::Sprinting)
		{
			if (!bSprintRequested || OwnerCharacter->GetVelocity().IsZero())
			{
				if (OwnerCharacter->GetVelocity().SizeSquared() < 10.0f) StateMachine->SetState(ECharacterState::Idle);
				else StateMachine->SetState(ECharacterState::Moving);
			}
		}
		else if (CurrentState == ECharacterState::Moving)
		{
			// 兜底逻辑：如果处于物理静止，C++状态自动切回 Idle
			if (OwnerCharacter->GetVelocity().SizeSquared() < 10.0f) StateMachine->SetState(ECharacterState::Idle);
		}
	}

	if (!FMath::IsNearlyEqual(OldStamina, CurrentStamina, 0.01f))
	{
		ApplyStaminaChange();
	}
}
void USprintComponent::StartSprinting()
{
	if(bStaminaDrained) return; // 体力耗尽时按 Shift 无效

	bSprintRequested = true;

	// 强行激活一次速度更新，不等 Tick 的延迟
	//UpdateMovementSpeed();

	UE_LOG(LogTemp, Log, TEXT("香子兰检测：玩家按下了 Shift，冲刺请求已激活！"));
}

void USprintComponent::StopSprinting()
{
	bSprintRequested = false;
}

float USprintComponent::GetCurrentStaminaPercent() const
{
	return MaxStamina>0.f?CurrentStamina / MaxStamina:0.f;

}

float USprintComponent::GetCurrentStamina() const
{
	return CurrentStamina;
}
void USprintComponent::StaminaChange(float Delta)
{
	// ...
	CurrentStamina += Delta;
	if (CurrentStamina > MaxStamina)
	{
		CurrentStamina = MaxStamina;
	}
	if (OnStaminaChanged.IsBound())
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
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
