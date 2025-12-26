// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include"HealthController/AttributeComponent.h"
#include"SprintComponent.h"
#include"ItemDefinition.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

int32 UInventoryComponent::AddItem(const FItemData& InItemData, int32 InCount) 
{
	if (InItemData.ID.IsNone() || InCount <= 0) 
	{
		return InCount;
	}
	int32 LeftoverCount = InCount;

	//堆叠规则
	bool bCanStack = true;
	int32 MaxStackSize = 99;

	UItemDefinition* LogicAsset = InItemData.ItemLogic;
	if (LogicAsset) 
	{
		bCanStack = LogicAsset->bStackable;
		MaxStackSize = LogicAsset->MaxStackCount;
	}

	if (bCanStack) 
	{
		for (FItemStack& Slot : Items) 
		{
			//如果发现相同ID的物品，并且该格子未满
			if (Slot.ItemData.ID == InItemData.ID && Slot.Count < MaxStackSize)
			{
				//计算该格子还能放多少
				int32 RestSpace = MaxStackSize - Slot.Count;

				//计算这次能放多少进去
				int32 ToAdd = FMath::Min(RestSpace, LeftoverCount);

				//放进去
				Slot.Count += ToAdd;
				LeftoverCount -= ToAdd;

				//如果已经放完了，直接跳出循环
				if (LeftoverCount <= 0) 
				{
					break;
				}
			
			}
		}
	}

	//当我们发现剩余物品>99*k时候，就要开始开拓新的格子
	while (LeftoverCount > 0) 
	{
		//创建一个新的物品格子
		FItemStack NewStack;
		NewStack.ItemData = InItemData;

		//计算新格子里放多少
		int32 AmountForNewSlot = bCanStack ? FMath::Min(LeftoverCount, MaxStackSize) : 1;
		//放进去
		NewStack.Count = AmountForNewSlot;

		Items.Add(NewStack);

		LeftoverCount -= AmountForNewSlot;

	}
	//广播更新事件，发现有物品被成功添加
	if (LeftoverCount < InCount) 
	{
		if(OnInventoryUpdated.IsBound())
			OnInventoryUpdated.Broadcast();
	}
	return LeftoverCount;
}

void UInventoryComponent::RemoveItem(const FItemData &InItemData, int32 InCount) 
{
	if (InCount <= 0)return;

	//剩余需要移除的数量
	int32 LeftoverToRemove = InCount;
	//倒叙遍历 因为加入物品是从前往后加的
	for (int32 i = Items.Num() - 1; i >= 0; i--)
	{
		if (Items[i].ItemData.ID == InItemData.ID)
		{
			//足够被移除
			if (Items[i].Count >= LeftoverToRemove)
			{
				Items[i].Count -= LeftoverToRemove;
				LeftoverToRemove = 0;
				if (LeftoverToRemove == 0)
				{
					//如果该格子被清空了，移除该格子
					Items.RemoveAt(i);
					//break;
				}
				break;
			}
			else
			{
				//不够被移除，清空该格子，继续往前找
				LeftoverToRemove -= Items[i].Count;
				Items.RemoveAt(i);
				break;
			}
		}
	}
}

int32 UInventoryComponent::GetTotalCountOfItem(FName ItemID) 
{
	int32 TotalCount = 0;
	for (const FItemStack& Slot : Items) 
	{
		if (Slot.ItemData.ID == ItemID)
		{
			TotalCount += Slot.Count;
		}
	}
	return TotalCount;
}
// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInventoryComponent::UseItem(int32 SlotIndex) 
{
	if (!Items.IsValidIndex(SlotIndex)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("无效的背包槽位索引！"));
		return;
	}

	FItemStack& ItemStack = Items[SlotIndex];

	if (ItemStack.Count <= 0) 
	{
		UE_LOG(LogTemp, Warning, TEXT("背包槽位 %d 为空！"), SlotIndex);
		return;
	}

	UItemDefinition* LogicAsset = ItemStack.ItemData.ItemLogic;

	if (LogicAsset) 
	{
		LogicAsset->OnUse(GetOwner());

		if (LogicAsset->bConsumeOnUse)
		{
			// 调用之前写好的按索引移除
			//这里的1应该是物品定义里的消耗数量
			//但是 一次只消耗一个 所以我觉得 没啥问题
			RemoveItem(ItemStack.ItemData, 1);
		}
	}

	else
	{
		UE_LOG(LogTemp, Warning, TEXT("物品没有定义 LogicAsset，无法使用！"));
	}
}