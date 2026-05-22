// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySlotWidget.h"
#include"Components/Image.h"
#include"Components/TextBlock.h"
#include"Components/Button.h"
#include"InventoryComponent.h"
#include"EscapeGameCharacter.h"
#include "Engine/Engine.h"
void UInventorySlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized(); // 👈 注意这里是 OnInitialized

	if (SlotButton)
	{
		// 这里一辈子只跑一次，绝对安全！
		SlotButton->OnClicked.AddDynamic(this, &UInventorySlotWidget::OnSlotClicked);
	}
}

//设置物品基本属性
void UInventorySlotWidget::SetItem(const FItemStack& Item) 
{
	//如果本物件存在图片
	CurrentItem = Item;
	
	
	if (IconImage) 
	{
		//设置图片步骤
		if (IsValid(Item.ItemData.Icon)) 
		{
			//设置图标
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

	if (Item.Count > 0)
	{
		if (TooltipClass)
		{
			// 正常创建逻辑
			UItemToolTipWidget* CreatedItemTipWidget = CreateWidget<UItemToolTipWidget>(GetOwningPlayer(), TooltipClass);
			if (CreatedItemTipWidget)
			{
				CreatedItemTipWidget->UpdateToolTipInfo(Item.ItemData);
				this->SetToolTip(CreatedItemTipWidget);
			}
		}
		else
		{
			// 只有 Count > 0 且 Class 为空时，才是真的忘了赋值
			UE_LOG(LogTemp, Error, TEXT("严重错误：格子有东西，但在蓝图里没设置 TooltipClass！"));
		}
	}
	else
	{
		// 格子是空的，不需要 Tip，设为 nullptr
		this->SetToolTip(nullptr);
	}
	
}

void UInventorySlotWidget::OnSlotClicked()
{
	// [安全检查] 确保物品有效再使用
	if (CurrentItem.Count > 0&&!CurrentItem.ItemData.ID.IsNone())
	{
		UE_LOG(LogTemp, Log, TEXT("使用了物品: %s"), *CurrentItem.ItemData.ID.ToString());

		// 🌟 核心逻辑：通过 OwnerComponent 调用 UseItem
		if (OwnerComponent.IsValid())
		{
			// 这里的 SlotIndex 是 InitSlot 时存下来的
			UE_LOG(LogTemp, Log, TEXT("正在请求组件执行 UseItem，槽位: %d"), this->SlotIndex);

			// 📞 拨通电话，告诉组件“用掉这个东西”！
			OwnerComponent->UseItem(this->SlotIndex);
		}
		else
		{
			// 如果 OwnerComponent 是空的，说明 InitSlot 没跑，或者指针丢了
			UE_LOG(LogTemp, Error, TEXT("点击无效！OwnerComponent 是空的！(InitSlot 没跑？)"));
		}
	}
}
void UInventorySlotWidget::InitSlot(UInventoryComponent* InComp, int32 InIndex)
{
	OwnerComponent = InComp;
	SlotIndex = InIndex;

	// 🕵️‍♀️ 侦探埋伏在这里！
	if (InComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Slot %d 初始化成功！Owner是: %s"), InIndex, *InComp->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Slot %d 初始化失败！传进来的 Component 是空的！"), InIndex);
	}
	// 顺便刷新一下显示
	if (InComp && InComp->Items.IsValidIndex(InIndex))
	{
		SetItem(InComp->Items[InIndex]);
	}
}
//鼠标按下 开始拖拽
FReply UInventorySlotWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) 
	{
		//大于0才可以拖拽
		if (CurrentItem.Count > 0) 
		{
			
			return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
		}
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) 
{
	UInventoryDragDropOperation* DragOp = NewObject<UInventoryDragDropOperation>();

	DragOp->SourceSlotIndex = this->SlotIndex;
	DragOp->SourceComponent = this->OwnerComponent;

	// 设置视觉效果 (拖动时鼠标上显示的那个图标)
	// 这里简单粗暴地把自己作为视觉反馈，或者你可以专门做一个图标 Widget
	DragOp->DefaultDragVisual = this;
	DragOp->Pivot = EDragPivot::CenterCenter;

	OutOperation = DragOp;

	UE_LOG(LogTemp, Log, TEXT("开始拖拽物品，来自格子: %d"), SlotIndex);

}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) 
{
	UInventoryDragDropOperation* MyOp = Cast<UInventoryDragDropOperation>(InOperation);

	// 确保是我们的背包拖拽操作，并且组件都有效
	if (MyOp && OwnerComponent.IsValid() && MyOp->SourceComponent.IsValid())
	{
		// 🛑 目前只处理“同一个背包内部的交换”
		// 如果未来要做箱子，这里判断 MyOp->SourceComponent != OwnerComponent 即可
		if (MyOp->SourceComponent== OwnerComponent)
		{
			int32 FromIndex = MyOp->SourceSlotIndex;
			int32 ToIndex = this->SlotIndex;

			UE_LOG(LogTemp, Log, TEXT("UI请求交换：从 %d 到 %d"), FromIndex, ToIndex);

			// 📞 调用组件的交换逻辑
			OwnerComponent->SwapSlots(FromIndex, ToIndex);

			return true; // 告诉系统处理成功
		}
	}

	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

