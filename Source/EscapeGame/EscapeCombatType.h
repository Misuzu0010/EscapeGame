#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/HitResult.h"
#include "EscapeCombatType.generated.h"

USTRUCT(BlueprintType)
struct ESCAPEGAME_API FCombatDamageContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Combat|Damage")
	float DamageValue = 0.f;
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> InstigatorActor = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FVector HitImpulse = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag ActionTag;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag DamageTypeTag;

	UPROPERTY(BlueprintReadWrite)
	FHitResult HitResult;
};

USTRUCT(BlueprintType)
struct ESCAPEGAME_API FCombatDamageResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Combat|Damage")
	bool bApplied = false;
	
	UPROPERTY(BlueprintReadWrite, Category = "Combat|Damage")
	float ActualDamage=0.f;
	
	UPROPERTY(BlueprintReadWrite, Category = "Combat|Damage")
	bool bKilled=false;
};