// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AICharacter_1.h"
#include "Character/Components/AttributeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

// Sets default values
AAICharacter_1::AAICharacter_1()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw = false;

	AttributeComp = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComp"));
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 500.f;

}

// Called when the game starts or when spawned
void AAICharacter_1::BeginPlay()
{
	Super::BeginPlay();

	if (AttributeComp)
	{
		AttributeComp->CurrentHealth = AttributeComp->MaxHealth;
		UE_LOG(LogTemp, Log, TEXT("[AICharacter_1] Health initialized to %.2f/%.2f"),
			AttributeComp->CurrentHealth,
			AttributeComp->MaxHealth);
	}
	
}

// Called every frame
void AAICharacter_1::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAICharacter_1::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

FCombatDamageResult AAICharacter_1::ApplyDamage_Implementation(const FCombatDamageContext& DamageContext)
{
	FCombatDamageResult Result;

	if (!AttributeComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[AICharacter_1] ApplyDamage failed: AttributeComp is null on %s."), *GetName());
		return Result;
	}

	const float Damage = FMath::Max(0.f, DamageContext.DamageValue);
	if (Damage <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AICharacter_1] ApplyDamage ignored: invalid damage %.2f on %s."), DamageContext.DamageValue, *GetName());
		return Result;
	}
	
	const float OldHealth = AttributeComp->CurrentHealth;
	AttributeComp->ApplyHealthChange(-Damage);
	const float NewHealth = AttributeComp->CurrentHealth;

	Result.bApplied = NewHealth < OldHealth;
	Result.ActualDamage = OldHealth - NewHealth;
	Result.bKilled = NewHealth <= 0.f && OldHealth > 0.f;
	
	if (Result.bApplied)
	{
		AActor* Attacker = DamageContext.InstigatorActor;

		if (IsValid(Attacker) && Attacker != this)
		{
			if (AAIController* AIController = Cast<AAIController>(GetController()))
			{
				if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
				{
					Blackboard->SetValueAsObject(TEXT("TargetActor"), Attacker);
				}
			}
		}

		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->AddImpulse(DamageContext.HitImpulse, true);
		}
	}

	if (Result.bKilled)
	{
		HandleDeath();
	}

	return Result;
}

void AAICharacter_1::HandleDeath()
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
		MoveComp->StopMovementImmediately();
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();

		if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
		{
			Blackboard->SetValueAsBool(TEXT("bIsdead"), true);
			Blackboard->ClearValue(TEXT("TargetActor"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[AICharacter_1] HandleDeath failed to update blackboard: BlackboardComponent is null on %s."), *GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[AICharacter_1] HandleDeath failed to stop AI: controller is not AAIController on %s."), *GetName());
	}

	SetLifeSpan(3.0f);
}
