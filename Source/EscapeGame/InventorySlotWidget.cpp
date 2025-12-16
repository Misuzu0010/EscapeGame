// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySlotWidget.h"
#include"Components/Image.h"
#include"Components/TextBlock.h"
#include"Components/Button.h"
#include "Engine/Engine.h"
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
		if (IsValid(Item.ItemData.Icon)) 
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
	else 
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("IconImage is null!"));
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
	// [安全检查] 确保物品有效再使用
	if (IsValid(CurrentItem.ItemData.Icon) && CurrentItem.Count > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("使用了物品: %s"), *CurrentItem.ItemData.Icon->GetName());
		// TODO: 调用 Gameplay 层的 UseItem 逻辑
	}

}


