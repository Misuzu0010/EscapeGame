// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ItemAction_RestoreStamina.h"
#include "SprintComponent.h"

bool UItemAction_RestoreStamina::OnUse_Implementation(AActor* User)
{
	if (!User)
	{
		UE_LOG(LogTemp, Warning, TEXT("恢复体力失败：User 为空，无法查找 SprintComponent。"));
		return false;
	}

	USprintComponent* SprintComp = User->FindComponentByClass<USprintComponent>();
	if (SprintComp)
	{
		// 通过组件接口恢复体力，让组件内部统一负责 Clamp 和 UI 委托广播。
		SprintComp->StaminaChange(StaminaHealing);
		UE_LOG(LogTemp, Log, TEXT("恢复体力成功：目标=%s, 恢复值=%.2f"), *GetNameSafe(User), StaminaHealing);
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("恢复体力失败：目标 %s 身上没有 SprintComponent。"), *GetNameSafe(User));
	return false;
}
