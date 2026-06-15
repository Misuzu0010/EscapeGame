// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/ItemData.h"
#include "Components/TextBlock.h"
#include "ItemToolTipWidget.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPEGAME_API UItemToolTipWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void UpdateToolTipInfo(const FItemData& InItemData);
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock>NameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>DescriptionText;



	
};
