// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "CharacterAnimData.generated.h"

class UAnimMontage;
class UBlendSpace;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FActionDefinition
{
	GENERATED_BODY()

	// 对应的动画蒙太奇 (软引用，优化内存)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> Montage;

	// 播放速率 (1.0 = 正常速度)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Trace")
	float TraceDistance = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Trace")
	float TraceRadius = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	float BaseDamage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Physics")
	float KnockbackImpulse = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Physics")
	float LaunchImpulse = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Input")
	float AttackInputCacheTolerance = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Input")
	float ComboInputCacheTolerance = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dodge")
	float DodgeStaminaCost = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dodge")
	float DodgeInvisibilityDuration = 0.35f;


};
//动画数据库

UCLASS()
class ESCAPEGAME_API UCharacterAnimData : public UDataAsset
{
public:
	GENERATED_BODY()

	// ==========================================
	// 1. 基础移动 (给 AnimBP 用的)
	// ==========================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion")
	TSoftObjectPtr<UBlendSpace> MovementBlendSpace;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion")
	TSoftObjectPtr<UAnimSequenceBase> IdleAnim;

	// ==========================================
	// 2. 动作映射表 (给 Component 查表用的)
	// ==========================================
	// Key: GameplayTag (比如 Action.Attack.Light)
	// Value: 动作定义 (包含Montage)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions")
	TMap<FGameplayTag, FActionDefinition> ActionMap;
};
