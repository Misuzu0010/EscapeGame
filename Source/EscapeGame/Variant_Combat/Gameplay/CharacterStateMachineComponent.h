// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimMontage.h"
#include "Engine/TimerHandle.h"
#include "CharacterStateMachineComponent.generated.h"

class ACharacter;
class UAnimInstance;
class UNiagaraSystem;
class UNiagaraComponent;
class USkeletalMeshComponent;

/**
 *  Character state enumeration
 *  Defines all possible states for a character using this state machine
 */
UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Moving		UMETA(DisplayName = "Moving"),
	Attacking	UMETA(DisplayName = "Attacking"),
	Stunned		UMETA(DisplayName = "Stunned"),
	Dead		UMETA(DisplayName = "Dead")
};

/** Delegate broadcast when the state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStateChanged, ECharacterState, OldState, ECharacterState, NewState);

/** Delegate broadcast when the character dies */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterDeath);

/** Delegate broadcast when stun ends */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStunEnded);

/** Delegate broadcast when attack ends */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackEnded);

/**
 *  Character State Machine Component
 *  Manages character states (Idle, Moving, Attacking, Stunned, Dead) 
 *  with clear entry/exit logic, timers, and animation management.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UCharacterStateMachineComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Constructor */
	UCharacterStateMachineComponent();

protected:

	/** Called when the game starts */
	virtual void BeginPlay() override;

	/** Called when the component is destroyed */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	/** Called every frame */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ================================
	// State Management
	// ================================

public:

	/**
	 * Sets the character's state
	 * @param NewState The new state to transition to
	 * @return True if the state change was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "State Machine")
	bool SetState(ECharacterState NewState);

	/**
	 * Gets the current character state
	 * @return The current state
	 */
	UFUNCTION(BlueprintPure, Category = "State Machine")
	ECharacterState GetCurrentState() const { return CurrentState; }

	/**
	 * Gets the previous character state
	 * @return The previous state before the last transition
	 */
	UFUNCTION(BlueprintPure, Category = "State Machine")
	ECharacterState GetPreviousState() const { return PreviousState; }

	/**
	 * Checks if the character is in a specific state
	 * @param State The state to check
	 * @return True if currently in that state
	 */
	UFUNCTION(BlueprintPure, Category = "State Machine")
	bool IsInState(ECharacterState State) const { return CurrentState == State; }

	/**
	 * Checks if the character can change to a new state
	 * @param NewState The state to check
	 * @return True if the state change is allowed
	 */
	UFUNCTION(BlueprintPure, Category = "State Machine")
	bool CanChangeState(ECharacterState NewState) const;

protected:

	/** Handles logic when exiting a state */
	void OnStateExit(ECharacterState ExitingState);

	/** Handles logic when entering a state */
	void OnStateEnter(ECharacterState EnteringState);

	// ================================
	// Attack State
	// ================================

public:

	/**
	 * Starts an attack action
	 * @return True if the attack was successfully started
	 */
	UFUNCTION(BlueprintCallable, Category = "State Machine|Attack")
	bool StartAttack();

	/**
	 * Called when a weapon collision is detected
	 * @param HitActor The actor that was hit
	 * @param HitLocation The world location of the hit
	 * @param HitNormal The normal at the hit point
	 */
	UFUNCTION(BlueprintCallable, Category = "State Machine|Attack")
	void OnWeaponHit(AActor* HitActor, const FVector& HitLocation, const FVector& HitNormal);

	/**
	 * Enables or disables weapon collision detection
	 * @param bEnable True to enable collision, false to disable
	 */
	UFUNCTION(BlueprintCallable, Category = "State Machine|Attack")
	void SetWeaponCollisionEnabled(bool bEnable);

	/**
	 * Checks if weapon collision is currently enabled
	 * @return True if weapon collision is enabled
	 */
	UFUNCTION(BlueprintPure, Category = "State Machine|Attack")
	bool IsWeaponCollisionEnabled() const { return bWeaponCollisionEnabled; }

protected:

	/** Called when the attack montage ends */
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// ================================
	// Stunned State
	// ================================

public:

	/**
	 * Applies stun to the character
	 * @param StunDuration Duration of the stun in seconds. If <= 0, uses default StunTime.
	 */
	UFUNCTION(BlueprintCallable, Category = "State Machine|Stun")
	void ApplyStun(float StunDuration = -1.0f);

protected:

	/** Called when the stun timer expires */
	UFUNCTION()
	void OnStunEnd();

	// ================================
	// Death State
	// ================================

public:

	/**
	 * Triggers character death
	 */
	UFUNCTION(BlueprintCallable, Category = "State Machine|Death")
	void TriggerDeath();

protected:

	/** Called after death animation/logic completes */
	UFUNCTION()
	void OnDeathFinished();

	// ================================
	// Damage Handling
	// ================================

public:

	/**
	 * Processes incoming damage
	 * @param Damage Amount of damage to apply
	 * @param DamageCauser The actor that caused the damage
	 * @param DamageLocation World location where damage was applied
	 * @return True if damage was successfully applied
	 */
	UFUNCTION(BlueprintCallable, Category = "State Machine|Damage")
	bool ProcessDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation);

protected:

	// ================================
	// Animation Helpers
	// ================================

	/** Gets the AnimInstance from the owner character */
	UAnimInstance* GetAnimInstance() const;

	/** Plays a montage and sets up the end delegate */
	float PlayMontageWithDelegate(UAnimMontage* Montage, FOnMontageEnded& EndDelegate, float PlayRate = 1.0f);

	/** Stops a currently playing montage */
	void StopMontage(UAnimMontage* Montage, float BlendOutTime = 0.2f);

	// ================================
	// Effect Helpers
	// ================================

	/** Activates the stun visual effect */
	void ActivateStunEffect();

	/** Deactivates the stun visual effect */
	void DeactivateStunEffect();

	// ================================
	// Properties - State
	// ================================

protected:

	/** Current character state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State Machine|State")
	ECharacterState CurrentState = ECharacterState::Idle;

	/** Previous character state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State Machine|State")
	ECharacterState PreviousState = ECharacterState::Idle;

	// ================================
	// Properties - Attack
	// ================================

protected:

	/** Montage to play for attack animations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

	/** Base damage dealt by attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Attack", meta = (ClampMin = 0.0f))
	float AttackDamage = 10.0f;

	/** Actors already hit during the current attack (prevents multiple hits) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State Machine|Attack")
	TArray<TObjectPtr<AActor>> HitActorsThisAttack;

	/** Whether weapon collision is currently enabled */
	bool bWeaponCollisionEnabled = false;

	/** Attack montage ended delegate */
	FOnMontageEnded AttackMontageEndedDelegate;

	// ================================
	// Properties - Stun
	// ================================

protected:

	/** Montage to play during stun */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Stun")
	TObjectPtr<UAnimMontage> StunMontage;

	/** Default duration of the stun state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Stun", meta = (ClampMin = 0.1f, Units = "s"))
	float StunTime = 2.0f;

	/** Niagara system for stun visual effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Stun")
	TObjectPtr<UNiagaraSystem> StunEffect;

	/** Socket name to attach stun effect to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Stun")
	FName StunEffectSocketName = FName("head");

	/** Reference to the spawned stun effect component */
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> StunEffectComponent;

	/** Timer handle for stun duration */
	FTimerHandle StunTimerHandle;

	// ================================
	// Properties - Death
	// ================================

protected:

	/** Montage to play on death */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Death")
	TObjectPtr<UAnimMontage> DeathMontage;

	/** Time to wait after death montage before calling death finished callback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Death", meta = (ClampMin = 0.0f, Units = "s"))
	float DeathFinishDelay = 2.0f;

	/** Timer handle for death finish delay */
	FTimerHandle DeathTimerHandle;

	/** Death montage ended delegate */
	FOnMontageEnded DeathMontageEndedDelegate;

	// ================================
	// Events
	// ================================

public:

	/** Broadcast when the state changes */
	UPROPERTY(BlueprintAssignable, Category = "State Machine|Events")
	FOnStateChanged OnStateChanged;

	/** Broadcast when the character dies */
	UPROPERTY(BlueprintAssignable, Category = "State Machine|Events")
	FOnCharacterDeath OnCharacterDeath;

	/** Broadcast when stun ends */
	UPROPERTY(BlueprintAssignable, Category = "State Machine|Events")
	FOnStunEnded OnStunEnded;

	/** Broadcast when attack ends */
	UPROPERTY(BlueprintAssignable, Category = "State Machine|Events")
	FOnAttackEnded OnAttackEnded;

	// ================================
	// Cached References
	// ================================

protected:

	/** Cached reference to the owner character */
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;
};
