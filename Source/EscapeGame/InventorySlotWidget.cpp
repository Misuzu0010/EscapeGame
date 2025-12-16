// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySlotWidget.h"
#include"Components/Image.h"
#include"Components/TextBlock.h"
#include"Components/Button.h"
void UInventorySlotWidget::NativeConstruct()
{
	UUserWidget::NativeConstruct();

	if (SlotButton) 
	{
		SlotButton->OnClicked.AddDynamic(this, &UInventorySlotWidget::OnSlotClicked);
	}
}

//设置物品基本属性
void UInventorySlotWidget::SetItem(const FItemStack& Item) 
{
	CurrentItem = Item;
	//如果本物件存在图片
	if (IconImage) 
	{
		//设置图片
		if (Item.ItemData.Icon) 
		{
			IconImage->SetBrushFromTexture(Item.ItemData.Icon);
			IconImage->SetVisibility(ESlateVisibility::Visible);
		}
		else 
		{
			//空格子
			IconImage->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	
	if (CountText) 
	{
		if (Item.Count > 1) 
		{
			CountText->SetText(FText::AsNumber(Item.Count));
			CountText->SetVisibility(ESlateVisibility::Visible);
		}
		else CountText->SetVisibility(ESlateVisibility::Hidden);
	}
	
}

void UInventorySlotWidget::OnSlotClicked()
{
	//使用物品的逻辑

}


