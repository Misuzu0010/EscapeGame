// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDefinition.h"
#include "ItemAction_Healing.generated.h"
class UAttributeComponent;

/**
 * 
 */
UCLASS()
class ESCAPEGAME_API UItemAction_Healing : public UItemDefinition
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category = "Heal")
	float HealAmount = 25.0f;

	// ÷ÿ–¥ π”√¬ﬂº≠
	virtual void OnUse_Implementation(AActor* User) override;
	
};
