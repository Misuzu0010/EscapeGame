// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryMenuWidget.h"
#include "Components/WrapBox.h"
#include "InventoryComponent.h"
#include "InventorySlotWidget.h"

void UInventoryMenuWidget::InitializeInventory(UInventoryComponent* InventoryComp)
{
	if (!InventoryComp) return;

	InventoryRef = InventoryComp;

	// 绑定广播！这是最关键的一步！
	// 只有当 C++ 组件喊 "我变了"，UI 才会动。
	InventoryRef->OnInventoryUpdated.AddDynamic(this, &UInventoryMenuWidget::RefreshInventory);

	RefreshInventory();
}

void UInventoryMenuWidget::RefreshInventory()
{
	if (!ItemGrid || !SlotWidgetClass || !InventoryRef.IsValid())return;

	ItemGrid ->ClearChildren();
	
	for (const FItemStack& Item : InventoryRef->Items) 
	{
		//创建一个新的物品槽UI
		UInventorySlotWidget* NewSlot = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);

		if (NewSlot) 
		{
			NewSlot->SetItem(Item);

			ItemGrid->AddChildToWrapBox(NewSlot);
		}
	}
}