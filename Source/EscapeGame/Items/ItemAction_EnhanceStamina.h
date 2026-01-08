// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDefinition.h"
#include "ItemAction_EnhanceStamina.generated.h"

class USprintComponent;
/**
 * 
 */
UCLASS()
class ESCAPEGAME_API UItemAction_EnhanceStamina : public UItemDefinition
{
	GENERATED_BODY()
public:
	virtual bool OnUse_Implementation(AActor* User) override;

	UPROPERTY(EditDefaultsOnly, Category = "SprintBuff")
	float StaminaBoostAmount = 15.0f;
	
};
