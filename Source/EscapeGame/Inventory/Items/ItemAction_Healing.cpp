// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Items/ItemAction_Healing.h"
#include "Character/Components/AttributeComponent.h"

//因为可能会给别的敌人使用回复道具 所以命名改为 TargetActor
bool UItemAction_Healing::OnUse_Implementation(AActor* TargetActor)
{
	if (!TargetActor) 
	{
		UE_LOG(LogTemp, Warning, TEXT("使用了治疗道具: %s,但是没东西喵"),*ItemName.Name.ToString());
		return false;
	}
	UAttributeComponent* AttributeComp = TargetActor->FindComponentByClass<UAttributeComponent>();

	if (AttributeComp) 
	{
		if (AttributeComp->IsFullHealth()) 
		{
			UE_LOG(LogTemp, Warning, TEXT("目标满血喵"));
			return false;
		}
		
		AttributeComp->ApplyHealthChange(HealAmount);
		UE_LOG(LogTemp, Warning, TEXT("使用了目标道具: %s 喵"), *ItemName.Name.ToString());
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("使用了治疗道具: %s,但是目标没有属性组件喵"),*ItemName.Name.ToString());
	return false;
}

