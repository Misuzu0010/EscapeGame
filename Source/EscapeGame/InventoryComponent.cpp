// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
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
		UE_LOG(LogTemp, Warning, TEXT("添加物品失败：ItemID=%s, Count=%d。ItemID 为空或数量非法。"),
			*InItemData.ID.ToString(),
			InCount);
		return InCount;
	}
	int32 LeftoverCount = InCount;

	// 堆叠规则来自物品逻辑资产；没有逻辑资产时使用默认 99 堆叠，保证纯数据物品也能进入背包。
	bool bCanStack = true;
	int32 MaxStackSize = 99;

	UItemDefinition* LogicAsset = InItemData.ItemLogic;
	if (LogicAsset) 
	{
		bCanStack = LogicAsset->bStackable;
		MaxStackSize = LogicAsset? FMath::Max(1,LogicAsset->MaxStackCount):99;
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
		int32 EmptySlotIndex = -1;
		for (int32 i = 0; i < Items.Num(); i++) 
		{
			if (Items[i].Count <= 0) 
			{
				EmptySlotIndex = i;
				break;
			}
		}
		if (EmptySlotIndex == -1)
		{
			UE_LOG(LogTemp, Warning, TEXT("背包满了"));
			break;
		}
		// 填入数据
		FItemStack& TargetStack = Items[EmptySlotIndex];
		TargetStack.ItemData = InItemData;
		//如果不能堆叠 移动<=99 或者1个
		int32 AmountToMove = bCanStack ? FMath::Min(LeftoverCount, MaxStackSize) : 1;
		TargetStack.Count = AmountToMove;

		LeftoverCount -= AmountToMove;

	}

	//广播更新事件，发现有物品被成功添加
	if (LeftoverCount < InCount) 
	{
		if(OnInventoryUpdated.IsBound())
			OnInventoryUpdated.Broadcast();
	}
	return LeftoverCount;
}

void UInventoryComponent::SwapSlots(int32 IndexA, int32 IndexB)
{
	// 1. 安全检查
	if (!Items.IsValidIndex(IndexA) || !Items.IsValidIndex(IndexB)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("无效的背包槽位索引！IndexA: %d, IndexB: %d"), IndexA, IndexB);
		return;
	}
	if (IndexA == IndexB) 
	{
		// 相同槽位，不需要交换
		UE_LOG(LogTemp,Warning,TEXT("这是一样的槽位捏"));
		return;
	}

	// 2. 原地交换 (TArray 自带的高效交换)
	Items.Swap(IndexA, IndexB);

	// 3. 告诉 UI 刷新
	if (OnInventoryUpdated.IsBound())
	{
		OnInventoryUpdated.Broadcast();
	}
}
void UInventoryComponent::RemoveItem(const FItemData &InItemData, int32 InCount) 
{
	if (InCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("移除物品失败：ItemID=%s, Count=%d。移除数量必须大于 0。"),
			*InItemData.ID.ToString(),
			InCount);
		return;
	}

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
				if (Items[i].Count<= 0)
				{
					//如果该格子被清空了，移除该格子
					Items[i] = FItemStack();
					//break;
				}
				break;
			}
			else
			{
				//不够被移除，清空该格子，继续往前找
				LeftoverToRemove -= Items[i].Count;
				Items[i] = FItemStack();
				continue;
			}
		}
	}
	if (OnInventoryUpdated.IsBound())
	{
		OnInventoryUpdated.Broadcast();
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
	Items.SetNum(InventoryCapacity);
	
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
		const bool bUsedSuccessfully = LogicAsset->OnUse(GetOwner());
		// 5. 数量归零处理
		
		if (bUsedSuccessfully&& LogicAsset->bConsumeOnUse) 
		{
			ItemStack.Count -= 1;

			UE_LOG(LogTemp, Warning, TEXT("使用成功！！"));

			if (ItemStack.Count <= 0) 
			{
				Items[SlotIndex] = FItemStack(); // 重置为默认空结构体
				UE_LOG(LogTemp, Warning, TEXT("用完啦"));
			}

			if (OnInventoryUpdated.IsBound()) 
			{
				OnInventoryUpdated.Broadcast();
			}
		}
		else if (!bUsedSuccessfully)
		{
			UE_LOG(LogTemp, Warning, TEXT("使用物品失败：Slot=%d, ItemID=%s, LogicAsset=%s 返回 false。"),
				SlotIndex,
				*ItemStack.ItemData.ID.ToString(),
				*GetNameSafe(LogicAsset));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("物品使用成功但未消耗：Slot=%d, ItemID=%s。"),
				SlotIndex,
				*ItemStack.ItemData.ID.ToString());
		}
	}

	else
	{
		UE_LOG(LogTemp, Warning, TEXT("物品没有定义 LogicAsset，无法使用！"));
	}
}
