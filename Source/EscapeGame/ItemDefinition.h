// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemData.h"
#include "ItemDefinition.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPEGAME_API UItemDefinition : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Logic")
	bool bConsumeOnUse = true;

	// 是否可堆叠
	UPROPERTY(EditAnywhere, Category = "Logic")
	bool bStackable = true;

	// 最大堆叠数量
	UPROPERTY(EditAnywhere, Category = "Logic", meta = (EditCondition = "bStackable"))
	int32 MaxStackCount = 99;

	// 使用道具的逻辑
	virtual void OnUse_Implementation(ACharacter* Character);
	
};
