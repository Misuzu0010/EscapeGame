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


	// 【核心】当物品被使用时触发。
	// 传入 User (玩家)，这样物品就知道给谁加血/加速。
	virtual bool OnUse(AActor* TargetActor);

	// 默认实现返回 true，防止那些不需要判断逻辑的道具无法使用
	virtual bool OnUse_Implementation(AActor* TargetActor)
	{
		return true;
	}



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	float CoolDownTime=0.0f;
	
};
