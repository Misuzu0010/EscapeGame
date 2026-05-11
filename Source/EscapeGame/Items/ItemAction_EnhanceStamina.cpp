// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ItemAction_EnhanceStamina.h"
#include"SprintComponent.h"

bool UItemAction_EnhanceStamina::OnUse_Implementation(AActor* User)
{
	if (!User) return false;
	USprintComponent* SprintComp = User->FindComponentByClass<USprintComponent>();
	if (SprintComp)
	{
		SprintComp->ApplyMaxChange(StaminaBoostAmount);
		return true;
	}
	return false;
}