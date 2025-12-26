// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ItemAction_Healing.h"
#include"HealthController/AttributeComponent.h"

void UItemAction_Healing::OnUse_Implementation(AActor* User)
{
	Super::OnUse_Implementation(User);
	if (!User)return;
	UAttributeComponent* AttributeComp = User->FindComponentByClass<UAttributeComponent>();
	if (AttributeComp) 
	{
		AttributeComp->ApplyHealthChange(HealAmount);
	}
}	

