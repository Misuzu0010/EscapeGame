// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ItemAction_RestoreStamina.h"
#include"SprintComponent.h"
bool UItemAction_RestoreStamina::OnUse_Implementation(AActor* User)
{
	if (!User) return false;
	USprintComponent* SprintComp = User->FindComponentByClass<USprintComponent>();
	if (SprintComp)
	{
		SprintComp->StaminaChange(StaminaHealing);
		return true;
	}
	return false;
}