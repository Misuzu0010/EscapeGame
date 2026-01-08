// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDefinition.h"
#include "ItemAction_Boosting.generated.h"

class USprintComponent;

/**
 * 
 */
UCLASS()
class ESCAPEGAME_API UItemAction_Boosting : public UItemDefinition
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SpringBuff")
	float SpeedMultiplier = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SpringBuff")
	float Duration = 15.0f;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite,Category ="SprintBuff")
	FItemText ItemName;

	// --- ÖØÐ´º¯Êý (Override) ---
	virtual bool OnUse_Implementation(AActor* TargetActor) override;


	
};
