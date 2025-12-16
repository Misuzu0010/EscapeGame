// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include"ItemData.h"
#include "InventorySlotWidget.generated.h"


class UImage;
class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class ESCAPEGAME_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable,Category="Inventory")
	void SetItem(const FItemStack& NewItem);
protected:

    UPROPERTY(meta = (BindWidget))
    UImage* IconImage;  // 显示道具图标

    UPROPERTY(meta = (BindWidget))
    UTextBlock* CountText; // 显示数量

    UPROPERTY(meta = (BindWidget))
    UButton* SlotButton;   // 点击按钮（用来使用或丢弃）

    // 保存当前格子的数据，以备点击时使用
    FItemStack CurrentItem;

    UFUNCTION()
    void OnSlotClicked();

    virtual void NativeConstruct() override;
};
