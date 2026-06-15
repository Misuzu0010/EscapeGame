// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/HitResult.h"
#include "UObject/Interface.h"
#include "EscapeCombatAttacker.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEscapeCombatAttacker : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ESCAPEGAME_API IEscapeCombatAttacker
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    /** * 获取攻击者的基础伤害
     * 用于受击方在计算最终伤害前进行二次修正（比如某些怪对特定攻击者有防御加成）
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Attacker")
    float GetBaseDamage() const;

    /** * 获取当前的连击数
     * 受击方可以根据你的连击数播放不同的受击动画（比如第3段攻击触发大击退）
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Attacker")
    int32 GetCurrentComboCount() const;

    /** * 命中反馈通知
     * 当 Damageable 确实扣血后，反向调用此函数。
     * 这是你触发“卡肉顿帧（HitStop）”或“吸血效果”的最佳时机！
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Attacker")
    void NotifyHitConfirmed(AActor* HitTarget, const FHitResult& HitResult);
};
