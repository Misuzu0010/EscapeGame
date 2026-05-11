// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include"ItemData.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ESCAPEGAME_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Inventory")
	TArray<FItemStack>Items;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	/**
	 * 添加物品到背包 (最复杂的逻辑都在这里！)
	 * @param InItemData - 要添加的物品	数据
	 * @param InCount - 添加多少个？
	 * @return 剩下的没存下的数量 (0表示全存进去了)
	 */
	//引用传递 避免拷贝开销
	//相当于直接找到对应的内存地址
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddItem(const FItemData &InItemData, int32 InCount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItem(const FItemData &InItemData, int32 InCount);

	// 辅助：获取物品数量
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetTotalCountOfItem(FName ItemID);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseItem(int32 SlotIndex);

	/*UPROPERTY(BlueprintReadWrite,Category="Inventory")
	TMap<FGameplayTag, double>LastUseTime;*/

	// 背包容量 (比如 20 格)
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 InventoryCapacity = 30;

	// ?? 核心功能：交换两个格子的物品
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SwapSlots(int32 IndexA, int32 IndexB);

		
};
