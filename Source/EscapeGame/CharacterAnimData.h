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

	// 追踪半径 (TraceRadius = 0 时为线形追踪，大于0时为胶囊形追踪)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Trace")
	float TraceRadius = 40.f;

	// 伤害类型 (可以在蓝图里设置成不同的子类，触发不同的受击反应)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	float BaseDamage = 20.f;

	// 伤害倍率 (最终伤害 = BaseDamage * DamageMultiplier，可以用来实现暴击等效果)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	float DamageMultiplier = 1.0f;

	// 击退力度 (可以用来实现击飞等效果，数值越大击退越远)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Physics")
	float KnockbackImpulse = 600.f;

	// 发射力度 (可以用来实现抛投等效果，数值越大抛得越高)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Physics")
	float LaunchImpulse = 300.f;

	// 攻击输入缓存宽容度 (单位：秒，表示在攻击动画的哪个时间窗口内按下攻击键可以触发连招)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Input")
	float AttackInputCacheTolerance = 1.0f;

	// 连击输入缓存宽容度 (单位：秒，表示在连击动画的哪个时间窗口内按下攻击键可以触发下一段连招)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Input")
	float ComboInputCacheTolerance = 0.45f;

	//连击耐力消耗 (单位：点，表示执行该攻击动作时消耗的耐力值，可以用来限制连续攻击的次数或频率)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dodge")
	float DodgeStaminaCost = 15.f;

	// 闪避无敌持续时间 (单位：秒，表示执行闪避动作后角色处于无敌状态的持续时间，可以用来平衡闪避的强度和风险)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dodge")
	float DodgeInvisibilityDuration = 0.35f;

	// 下一段连招的标签 (可以用来在蓝图里查表，决定下一段连招的动作定义)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Combo")
	FGameplayTag NextComboTag;
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
	// 运动混合空间 (软引用，优化内存)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion")
	TSoftObjectPtr<UBlendSpace> MovementBlendSpace;
	// 怪物待机动画 (软引用，优化内存)
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
