// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryHotbarWidget.h"
#include"Components/HorizontalBox.h"
#include"InventoryComponent.h"
#include"InventorySlotWidget.h"


void UInventoryHotbarWidget::InitializeHotbar(UInventoryComponent* InventoryComp)
{
	if (InventoryComp) 
	{
		//弱指针指向组件防崩溃
		InventoryRef = InventoryComp;

		//监听广播
		InventoryComp->OnInventoryUpdated.AddDynamic(this, &UInventoryHotbarWidget::RefreshHotbar);

		RefreshHotbar();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("快捷栏初始化失败：InventoryComp 为空。"));
	}
}

void UInventoryHotbarWidget::RefreshHotbar() 
{
	if (!HotbarGrid || !SlotWidgetClass) 
	{
		UE_LOG(LogTemp, Warning, TEXT("快捷栏刷新失败：HotbarGrid=%s, SlotWidgetClass=%s。"),
			*GetNameSafe(HotbarGrid),
			SlotWidgetClass ? *SlotWidgetClass->GetName() : TEXT("None"));
		if (!HotbarGrid) 
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("HotbarGrid is null!"));
		}
		if (!SlotWidgetClass) 
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("SlotWIdgetClass Not Found!!"));
		}
		return;
	}


	HotbarGrid->ClearChildren();

	for (int32 i = 0; i < NumSlot; i++) 
	{
		UInventorySlotWidget* NewSlot = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);

		if (NewSlot) 
		{
			if (InventoryRef.IsValid() && InventoryRef->Items.IsValidIndex(i)) 
			{
				// 3. 尝试从背包里获取数据
				// 注意：我们要检查背包里的物品够不够
				NewSlot->InitSlot(InventoryRef.Get(),i);
				NewSlot->SetItem(InventoryRef->Items[i]);
			}

			else 
			{
				// 没数据 (背包里只有2个苹果，但这是第3个格子)，填入一个空的结构体
				// 你的 SlotWidget 需要能处理空数据（隐藏图标）
				FItemStack EmptyItem;
				NewSlot->SetItem(EmptyItem);
			}
			//将格子添加到UI
			HotbarGrid->AddChildToHorizontalBox(NewSlot);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("快捷栏刷新警告：创建 SlotWidget 失败，Index=%d, WidgetClass=%s。"),
				i,
				SlotWidgetClass ? *SlotWidgetClass->GetName() : TEXT("None"));
		}
	}
}
