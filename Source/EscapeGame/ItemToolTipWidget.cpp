// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemToolTipWidget.h"

void UItemToolTipWidget::UpdateToolTipInfo(const FItemData& InItemData)
{
	if (NameText)
	{
		UE_LOG(LogTemp, Warning, TEXT("成功加载物品名称"));
		NameText->SetText(InItemData.ItemText.Name);
		//可以考虑添加一个枚举类枚举类型，根据类型设置不同颜色
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("未成功加载物品名称"));
	}
	if (DescriptionText)
	{
		UE_LOG(LogTemp, Warning, TEXT("成功加载物品描述"));
		DescriptionText->SetText(InItemData.ItemText.Description);
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("未成功加载物品描述"));
	}
}