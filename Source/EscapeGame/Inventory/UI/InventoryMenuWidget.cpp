// Fill out your copyright notice in the Description page of Project Settings.

#include "Inventory/UI/InventoryMenuWidget.h"
#include "Components/WrapBox.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/UI/InventorySlotWidget.h"

void UInventoryMenuWidget::InitializeInventory(UInventoryComponent* InventoryComp)
{
	if (!InventoryComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("背包菜单初始化失败：InventoryComp 为空。"));
		return;
	}

	InventoryRef = InventoryComp;

	// 绑定广播！这是最关键的一步！
	// 只有当 C++ 组件喊 "我变了"，UI 才会动。
	InventoryRef->OnInventoryUpdated.AddDynamic(this, &UInventoryMenuWidget::RefreshInventory);

	RefreshInventory();
}

void UInventoryMenuWidget::RefreshInventory()
{
	if (!ItemGrid || !SlotWidgetClass || !InventoryRef.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("背包菜单刷新失败：ItemGrid=%s, SlotWidgetClass=%s, InventoryRefValid=%s。"),
			*GetNameSafe(ItemGrid),
			SlotWidgetClass ? *SlotWidgetClass->GetName() : TEXT("None"),
			InventoryRef.IsValid() ? TEXT("true") : TEXT("false"));
		return;
	}

	ItemGrid ->ClearChildren();
	
	int32 MaxCapacity =InventoryRef->InventoryCapacity;
	for (int32 i = 0; i < MaxCapacity; i++) 
	{
		UInventorySlotWidget* NewSlot = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);

		if (NewSlot) 
		{
			
			NewSlot->InitSlot(InventoryRef.Get(), i);

			if (InventoryRef->Items.IsValidIndex(i))
			{
				NewSlot->SetItem(InventoryRef->Items[i]);
			}
			//添加到格子里
			ItemGrid->AddChild(NewSlot);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("背包菜单刷新警告：创建 SlotWidget 失败，Index=%d, WidgetClass=%s。"),
				i,
				SlotWidgetClass ? *SlotWidgetClass->GetName() : TEXT("None"));
		}
	}
}
