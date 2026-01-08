// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include"ItemData.h"
#include"InventoryDragDropOperation.h"
#include "ItemToolTipWidget.h" // 引用刚才写的弹窗头文件
#include "InventorySlotWidget.generated.h"


class UImage;
class UTextBlock;
class UButton;
class UInventoryComponent;
/**
 * 
 */
UCLASS()
class ESCAPEGAME_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// 设置格子显示的物品
	UFUNCTION(BlueprintCallable,Category="Inventory")
	void SetItem(const FItemStack& NewItem);  
	// 初始化格子，绑定背包组件和索引
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void InitSlot(UInventoryComponent*InComp,int32 Index);

protected:
	// 物品图标
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> IconImage;  // UE5 建议使用 TObjectPtr 替代原生指针
	// 物品数量文本
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> CountText;
	// 按钮，用于点击交互
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> SlotButton;
	// 格子索引
    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 SlotIndex;

    // 保存当前格子的数据，以备点击时使用
    UPROPERTY()
    FItemStack CurrentItem;

	// 指向拥有该格子的背包组件
    UPROPERTY(BlueprintReadOnly)
    TWeakObjectPtr<UInventoryComponent> OwnerComponent;

    // 1. 【配置】在编辑器里选择你的 WBP_ItemTooltip
    UPROPERTY(EditDefaultsOnly, Category = "Tooltip")
    TSubclassOf<UItemToolTipWidget> TooltipClass;

    

    virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

  
    UFUNCTION()
    void OnSlotClicked();

    virtual void NativeOnInitialized() override;
};
