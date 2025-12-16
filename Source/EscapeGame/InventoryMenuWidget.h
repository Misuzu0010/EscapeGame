// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryMenuWidget.generated.h"

/**
 * 
 */

class UWrapBox; // 自动换行的容器，最适合做网格背包
class UInventoryComponent;
class UInventorySlotWidget;


UCLASS()
class ESCAPEGAME_API UInventoryMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable,Category="Inventory")
	void InitializeInventory(UInventoryComponent* InventoryComp);
protected:
	UPROPERTY(meta = (BindWidget))
	UWrapBox* ItemGrid;

	//配置项 UI类
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UInventorySlotWidget>SlotWidgetClass;

private:
	TWeakObjectPtr<UInventoryComponent> InventoryRef;

	UFUNCTION()
	void RefreshInventory();

	
};
