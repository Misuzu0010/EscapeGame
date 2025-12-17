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
	
	int32 MaxCapacity =30;
	for (int32 i = -0; i < MaxCapacity; i++) 
	{
		UInventorySlotWidget* NewSlot = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);

		if (NewSlot) 
		{
			
			if (InventoryRef->Items.IsValidIndex(i)) 
			{
				NewSlot->SetItem(InventoryRef->Items[i]);
			}

			else 
			{
				FItemStack EmptyItem;
				NewSlot->SetItem(EmptyItem);
			}

			ItemGrid->AddChild(NewSlot);
		}
	}
}