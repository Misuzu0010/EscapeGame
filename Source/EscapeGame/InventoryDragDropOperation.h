// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "InventoryDragDropOperation.generated.h"

/**
 * 
 */
class UInventoryComponent;
UCLASS()
class ESCAPEGAME_API UInventoryDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragDrop")
	int32 SourceSlotIndex;

	// ?? 来源组件 (弱指针引用，防止组件销毁后崩溃)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragDrop")
	TWeakObjectPtr<UInventoryComponent> SourceComponent;
};
