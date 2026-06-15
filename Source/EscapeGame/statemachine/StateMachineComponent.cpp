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
	if (CurrentState == NewState)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("状态切换被忽略：已经处于 %s。"), *UEnum::GetValueAsString(CurrentState));
		return;
	}

	ECharacterState OldState = CurrentState;

	//状态保护逻辑
	//如果当前是死亡状态，不能切换到其他状态
	if (CurrentState == ECharacterState::Dead)
	{
		UE_LOG(LogTemp, Warning, TEXT("状态切换被拒绝：角色已经死亡，无法从 %s 切换到 %s。"),
			*UEnum::GetValueAsString(CurrentState),
			*UEnum::GetValueAsString(NewState));
		return;
	}

	//眩晕状态 不可切换为死亡/默认状态
	if (CurrentState == ECharacterState::Stunned && NewState != ECharacterState::Dead && NewState != ECharacterState::Idle)
	{
		UE_LOG(LogTemp, Warning, TEXT("状态切换被拒绝：眩晕状态只能恢复 Idle 或进入 Dead，当前请求=%s。"),
			*UEnum::GetValueAsString(NewState));
		return;
	}

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
			// 死亡状态是最高优先级状态，进入后立即停止角色移动，避免死亡后继续滑行或响应输入。
			UCharacterMovementComponent* CharMoveComp = OwnCharacter->GetCharacterMovement();

			if (CharMoveComp)
			{
				CharMoveComp->DisableMovement();
				CharMoveComp->StopMovementImmediately(); // 顺便把当前的惯性也停掉，更干脆
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("进入死亡状态时未找到 CharacterMovementComponent：Owner=%s。"), *GetNameSafe(OwnCharacter));
			}

		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("进入死亡状态时 Owner 不是 ACharacter：Owner=%s。"), *GetNameSafe(GetOwner()));
		}
	}
}

void UStateMachineComponent::ApplyStun(float Duration)
{
	if (CurrentState == ECharacterState::Dead)
	{
		UE_LOG(LogTemp, Warning, TEXT("眩晕申请被拒绝：角色已经死亡，Duration=%.2f。"), Duration);
		return;
	}

	SetState(ECharacterState::Stunned);

	if (UWorld* World = GetWorld()) 
	{
		World->GetTimerManager().SetTimer(TimerHandle_Stun, this, &UStateMachineComponent::OnStunFinished, Duration, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("眩晕计时器启动失败：World 为空，Duration=%.2f。"), Duration);
	}

}

void UStateMachineComponent::ApplyDeath()
{
	if (CurrentState == ECharacterState::Dead)
	{
		UE_LOG(LogTemp, Warning, TEXT("死亡申请被忽略：角色已经处于 Dead 状态。"));
		return;
	}
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
