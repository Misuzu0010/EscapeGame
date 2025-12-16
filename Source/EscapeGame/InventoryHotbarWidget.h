// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryHotbarWidget.generated.h"

/**
 * 
 */

class UHorizontalBox;
class UInventoryComponent;
class UInventorySlotWidget;

UCLASS()
class ESCAPEGAME_API UInventoryHotbarWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	//初始化 绑定组件
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeHotbar(UInventoryComponent* InventoryComp);

protected:
	//横向排
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* HotbarGrid;

	//格子显示个数
	UPROPERTY(EditDefaultsOnly,Category="Inventory")
	int32 NumSlot = 5;

	//格子UI类 
	UPROPERTY(EditDefaultsOnly,Category="Inventory")
	TSubclassOf<UInventorySlotWidget>SlotWidgetClass;

private:
	
	TWeakObjectPtr<UInventoryComponent>InventoryRef;

	UFUNCTION()
	void RefreshHotbar();


	
};
