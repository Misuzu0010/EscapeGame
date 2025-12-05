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
	PrimaryComponentTick.bCanEverTick = true;
	CurrentState = ECharacterState::Idle;		
	bCanMove = true;	
	bCanAttack = true;
	ComboIndex = 0;
	


	// ...
}


// Called when the game starts
void UStateMachineComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (!OwnerCharacter) 
	{
		UE_LOG(LogTemp, Error, TEXT("StateMachineComponent: Owner isn`t a character!"));
	}
// ...
	
}


// Called every frame
void UStateMachineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...
}

void UStateMachineComponent::SetState(ECharacterState Newstate) 
{
	if (!OwnerCharacter) 
	{
		UE_LOG(LogTemp, Error, TEXT("StateMachineComponent: Owner isn`t a character!"));
		return;
	}
	if (CurrentState == Newstate)return;

	if (CurrentState == ECharacterState::Stunned && Newstate == ECharacterState::Stunned) 
	{
		GetWorld()->GetTimerManager().ClearTimer(StunTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(
			StunTimerHandle, 
			this, 
			&UStateMachineComponent::OnStunEnd,
			StunDuration,
			false
		);
		return;
	}

	if (CurrentState == ECharacterState::Dead)return;

	CurrentState = Newstate;
	switch (CurrentState)
	{
	case ECharacterState::Idle:
		break;
	case ECharacterState::Moving:
		StopFootstepSound();
		break;
	case ECharacterState::Attacking:
		OwnerCharacter->StopAnimMontage();
		if()
		break;
	case ECharacterState::Sprinting:
		break;
	case ECharacterState::Stunned:
		break;
	case ECharacterState::Dead:
		break;
	default:
		break;
	}
}
