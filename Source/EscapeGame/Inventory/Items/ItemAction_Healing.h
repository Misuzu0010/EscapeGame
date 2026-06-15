// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/ItemDefinition.h"
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

	// 暴露给编辑器的变量：道具名字
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	FItemText ItemName;
	// 重写使用逻辑
	virtual bool OnUse_Implementation(AActor* TargetActor) override;



	
};
