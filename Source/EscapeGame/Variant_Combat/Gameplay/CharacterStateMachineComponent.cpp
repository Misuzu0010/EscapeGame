// Copyright Epic Games, Inc. All Rights Reserved.


#include "CharacterStateMachineComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

UCharacterStateMachineComponent::UCharacterStateMachineComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Bind delegates
	AttackMontageEndedDelegate.BindUObject(this, &UCharacterStateMachineComponent::OnAttackMontageEnded);
}

void UCharacterStateMachineComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache the owner character reference
	OwnerCharacter = Cast<ACharacter>(GetOwner());

	// Ensure we have a valid owner
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterStateMachineComponent: Owner is not a Character!"));
	}
}

void UCharacterStateMachineComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear all timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StunTimerHandle);
		World->GetTimerManager().ClearTimer(DeathTimerHandle);
	}

	// Clean up stun effect
	DeactivateStunEffect();

	Super::EndPlay(EndPlayReason);
}

void UCharacterStateMachineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// ================================
// State Management Implementation
// ================================

bool UCharacterStateMachineComponent::SetState(ECharacterState NewState)
{
	// Check if state change is allowed
	if (!CanChangeState(NewState))
	{
		// Special case: If stunned and trying to apply stun again, refresh the stun timer
		if (CurrentState == ECharacterState::Stunned && NewState == ECharacterState::Stunned)
		{
			// Refresh stun timer
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().ClearTimer(StunTimerHandle);
				World->GetTimerManager().SetTimer(StunTimerHandle, this, &UCharacterStateMachineComponent::OnStunEnd, StunTime, false);
			}
			return true;
		}
		return false;
	}

	// Store previous state
	PreviousState = CurrentState;

	// Exit current state
	OnStateExit(CurrentState);

	// Update current state
	CurrentState = NewState;

	// Enter new state
	OnStateEnter(CurrentState);

	// Broadcast state change event
	OnStateChanged.Broadcast(PreviousState, CurrentState);

	return true;
}

bool UCharacterStateMachineComponent::CanChangeState(ECharacterState NewState) const
{
	// Dead characters cannot change state (except to Dead to allow re-triggering death logic)
	if (CurrentState == ECharacterState::Dead && NewState != ECharacterState::Dead)
	{
		return false;
	}

	// Same state transitions are only allowed for Stunned (to refresh timer)
	if (CurrentState == NewState && NewState != ECharacterState::Stunned)
	{
		return false;
	}

	// Stunned characters can only transition to Dead
	if (CurrentState == ECharacterState::Stunned && NewState != ECharacterState::Dead && NewState != ECharacterState::Stunned)
	{
		return false;
	}

	return true;
}

void UCharacterStateMachineComponent::OnStateExit(ECharacterState ExitingState)
{
	switch (ExitingState)
	{
	case ECharacterState::Idle:
		// No special exit logic for Idle
		break;

	case ECharacterState::Moving:
		// No special exit logic for Moving
		break;

	case ECharacterState::Attacking:
		// Stop attack animation
		if (AttackMontage)
		{
			StopMontage(AttackMontage, 0.2f);
		}
		// Disable weapon collision
		SetWeaponCollisionEnabled(false);
		// Clear hit actors list
		HitActorsThisAttack.Empty();
		break;

	case ECharacterState::Stunned:
		// Clear stun timer
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(StunTimerHandle);
		}
		// Deactivate stun effect
		DeactivateStunEffect();
		// Stop stun montage
		if (StunMontage)
		{
			StopMontage(StunMontage, 0.2f);
		}
		break;

	case ECharacterState::Dead:
		// No special exit logic for Dead (typically death is final)
		break;

	default:
		break;
	}
}

void UCharacterStateMachineComponent::OnStateEnter(ECharacterState EnteringState)
{
	switch (EnteringState)
	{
	case ECharacterState::Idle:
		// Enable movement when entering idle
		if (OwnerCharacter)
		{
			if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
			{
				MovementComp->SetMovementMode(MOVE_Walking);
			}
		}
		break;

	case ECharacterState::Moving:
		// Movement state is handled by the character's movement component
		// This state is typically set when the character receives movement input
		break;

	case ECharacterState::Attacking:
		// Play attack montage
		if (AttackMontage)
		{
			PlayMontageWithDelegate(AttackMontage, AttackMontageEndedDelegate);
		}
		// Clear hit actors list for new attack
		HitActorsThisAttack.Empty();
		break;

	case ECharacterState::Stunned:
		// Set stun timer
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(StunTimerHandle, this, &UCharacterStateMachineComponent::OnStunEnd, StunTime, false);
		}
		// Play stun montage
		if (StunMontage)
		{
			if (UAnimInstance* AnimInstance = GetAnimInstance())
			{
				AnimInstance->Montage_Play(StunMontage);
			}
		}
		// Activate stun visual effect
		ActivateStunEffect();
		// Disable movement
		if (OwnerCharacter)
		{
			if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
			{
				MovementComp->StopMovementImmediately();
			}
		}
		break;

	case ECharacterState::Dead:
		// Disable movement
		if (OwnerCharacter)
		{
			if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
			{
				MovementComp->DisableMovement();
			}
		}
		// Play death montage
		if (DeathMontage)
		{
			if (UAnimInstance* AnimInstance = GetAnimInstance())
			{
				const float MontageLength = AnimInstance->Montage_Play(DeathMontage);
				// Set death finished timer based on montage length or configured delay
				const float DeathDelay = MontageLength > 0.0f ? MontageLength + DeathFinishDelay : DeathFinishDelay;
				if (UWorld* World = GetWorld())
				{
					World->GetTimerManager().SetTimer(DeathTimerHandle, this, &UCharacterStateMachineComponent::OnDeathFinished, DeathDelay, false);
				}
			}
		}
		else
		{
			// No death montage, call death finished after delay
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(DeathTimerHandle, this, &UCharacterStateMachineComponent::OnDeathFinished, DeathFinishDelay, false);
			}
		}
		// Broadcast death event
		OnCharacterDeath.Broadcast();
		break;

	default:
		break;
	}
}

// ================================
// Attack State Implementation
// ================================

bool UCharacterStateMachineComponent::StartAttack()
{
	// Can only attack from Idle or Moving states
	if (CurrentState != ECharacterState::Idle && CurrentState != ECharacterState::Moving)
	{
		return false;
	}

	return SetState(ECharacterState::Attacking);
}

void UCharacterStateMachineComponent::OnWeaponHit(AActor* HitActor, const FVector& HitLocation, const FVector& HitNormal)
{
	// Validate we're in attacking state with collision enabled
	if (CurrentState != ECharacterState::Attacking || !bWeaponCollisionEnabled)
	{
		return;
	}

	// Validate hit actor
	if (!HitActor || HitActor == GetOwner())
	{
		return;
	}

	// Check if we already hit this actor during this attack
	if (HitActorsThisAttack.Contains(HitActor))
	{
		return;
	}

	// Add to hit list
	HitActorsThisAttack.Add(HitActor);

	// Apply damage using UE's damage system
	UGameplayStatics::ApplyDamage(
		HitActor,
		AttackDamage,
		OwnerCharacter ? OwnerCharacter->GetController() : nullptr,
		GetOwner(),
		nullptr // DamageTypeClass
	);
}

void UCharacterStateMachineComponent::SetWeaponCollisionEnabled(bool bEnable)
{
	bWeaponCollisionEnabled = bEnable;

	// Clear hit actors when disabling collision
	if (!bEnable)
	{
		HitActorsThisAttack.Empty();
	}
}

void UCharacterStateMachineComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// Only process if we're still in attacking state
	if (CurrentState == ECharacterState::Attacking)
	{
		// Return to Idle state
		SetState(ECharacterState::Idle);

		// Broadcast attack ended event
		OnAttackEnded.Broadcast();
	}
}

// ================================
// Stunned State Implementation
// ================================

void UCharacterStateMachineComponent::ApplyStun(float StunDuration)
{
	// Update stun time if a custom duration is provided
	if (StunDuration > 0.0f)
	{
		StunTime = StunDuration;
	}

	SetState(ECharacterState::Stunned);
}

void UCharacterStateMachineComponent::OnStunEnd()
{
	// Only process if we're still in stunned state
	if (CurrentState == ECharacterState::Stunned)
	{
		// Return to Idle state
		SetState(ECharacterState::Idle);

		// Broadcast stun ended event
		OnStunEnded.Broadcast();
	}
}

// ================================
// Death State Implementation
// ================================

void UCharacterStateMachineComponent::TriggerDeath()
{
	SetState(ECharacterState::Dead);
}

void UCharacterStateMachineComponent::OnDeathFinished()
{
	// This callback allows for cleanup or respawn logic
	// The actual implementation is left to the owning character or game mode
	// This is called after the death animation/delay completes
}

// ================================
// Damage Handling Implementation
// ================================

bool UCharacterStateMachineComponent::ProcessDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation)
{
	// Dead characters don't process damage
	if (CurrentState == ECharacterState::Dead)
	{
		return false;
	}

	// Damage processing is handled here
	// Subclasses or owning actors can implement specific health management

	return true;
}

// ================================
// Animation Helpers Implementation
// ================================

UAnimInstance* UCharacterStateMachineComponent::GetAnimInstance() const
{
	if (!OwnerCharacter)
	{
		return nullptr;
	}

	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	if (!Mesh)
	{
		return nullptr;
	}

	return Mesh->GetAnimInstance();
}

float UCharacterStateMachineComponent::PlayMontageWithDelegate(UAnimMontage* Montage, FOnMontageEnded& EndDelegate, float PlayRate)
{
	UAnimInstance* AnimInstance = GetAnimInstance();
	if (!AnimInstance || !Montage)
	{
		return 0.0f;
	}

	const float MontageLength = AnimInstance->Montage_Play(Montage, PlayRate, EMontagePlayReturnType::MontageLength, 0.0f, true);

	if (MontageLength > 0.0f)
	{
		AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
	}

	return MontageLength;
}

void UCharacterStateMachineComponent::StopMontage(UAnimMontage* Montage, float BlendOutTime)
{
	UAnimInstance* AnimInstance = GetAnimInstance();
	if (!AnimInstance || !Montage)
	{
		return;
	}

	AnimInstance->Montage_Stop(BlendOutTime, Montage);
}

// ================================
// Effect Helpers Implementation
// ================================

void UCharacterStateMachineComponent::ActivateStunEffect()
{
	if (!StunEffect || !OwnerCharacter)
	{
		return;
	}

	// Get the mesh component for attachment
	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	if (!Mesh)
	{
		return;
	}

	// Spawn the Niagara effect attached to the character
	StunEffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		StunEffect,
		Mesh,
		StunEffectSocketName,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget,
		true // Auto destroy
	);
}

void UCharacterStateMachineComponent::DeactivateStunEffect()
{
	if (StunEffectComponent)
	{
		StunEffectComponent->Deactivate();
		StunEffectComponent = nullptr;
	}
}
