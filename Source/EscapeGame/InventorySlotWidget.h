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
    TObjectPtr<UImage> IconImage;  // UE5 建议使用 TObjectPtr 替代原生指针

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> CountText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> SlotButton;

    // 保存当前格子的数据，以备点击时使用
    UPROPERTY()
    FItemStack CurrentItem;

    UFUNCTION()
    void OnSlotClicked();

    virtual void NativeConstruct() override;
};
