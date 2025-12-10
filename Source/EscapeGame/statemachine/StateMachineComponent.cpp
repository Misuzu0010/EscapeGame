// Fill out your copyright notice in the Description page of Project Settings.


#include "statemachine/StateMachineComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

// Sets default values for this component's properties
UStateMachineComponent::UStateMachineComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	CurrentState = ECharacterState::Idle;
	// ...
}
// Called when the game starts
void UStateMachineComponent::BeginPlay()
{
	Super::BeginPlay();

	
// ...
	
}


// Called every frame
void UStateMachineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...
}

void UStateMachineComponent::SetState(ECharacterState NewState) 
{
	if (CurrentState == NewState)return;

	ECharacterState OldState = CurrentState;

	//状态保护逻辑
	//如果当前是死亡状态，不能切换到其他状态
	if (CurrentState == ECharacterState::Dead)return;

	//眩晕状态 不可切换为死亡/默认状态
	if (CurrentState == ECharacterState::Stunned && NewState != ECharacterState::Dead && NewState != ECharacterState::Idle)return;

	//更新状态
	CurrentState = NewState;

	//广播状态变化事件
	if (OnStateChanged.IsBound()) 
	{
		OnStateChanged.Broadcast(NewState, OldState);
	}

	if(NewState==ECharacterState::Dead)
	{
		ACharacter* OwnCharacter = Cast<ACharacter>(GetOwner());
		if (OwnCharacter) 
		{
			// 1. 获取移动组件
			UCharacterMovementComponent* CharMoveComp = OwnCharacter->GetCharacterMovement();

			// 2. 如果组件存在，就打断腿（禁止移动）
			if (CharMoveComp)
			{
				CharMoveComp->DisableMovement();
				CharMoveComp->StopMovementImmediately(); // 顺便把当前的惯性也停掉，更干脆
			}

		}
	}
}

void UStateMachineComponent::ApplyStun(float Duration)
{
	if (CurrentState == ECharacterState::Dead) return;

	SetState(ECharacterState::Stunned);

	if (UWorld* World = GetWorld()) 
	{
		World->GetTimerManager().SetTimer(TimerHandle_Stun, this, &UStateMachineComponent::OnStunFinished, Duration, false);
	}

}

void UStateMachineComponent::ApplyDeath()
{
	if (CurrentState == ECharacterState::Dead) return;
	SetState(ECharacterState::Dead);
}

void UStateMachineComponent::OnStunFinished()
{
	// 只有当前还是眩晕状态才恢复（防止中途被打死）
	if (CurrentState == ECharacterState::Stunned)
	{
		SetState(ECharacterState::Idle);
	}
}