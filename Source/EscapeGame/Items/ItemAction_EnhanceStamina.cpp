// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ItemAction_EnhanceStamina.h"
#include "SprintComponent.h"

bool UItemAction_EnhanceStamina::OnUse_Implementation(AActor* User)
{
	if (!User)
	{
		UE_LOG(LogTemp, Warning, TEXT("提升体力上限失败：User 为空，无法查找 SprintComponent。"));
		return false;
	}

	USprintComponent* SprintComp = User->FindComponentByClass<USprintComponent>();
	if (SprintComp)
	{
		// 体力上限道具只修改 SprintComponent，不直接触碰角色移动组件，避免和冲刺速度平滑逻辑互相覆盖。
		SprintComp->ApplyMaxChange(StaminaBoostAmount);
		UE_LOG(LogTemp, Log, TEXT("提升体力上限成功：目标=%s, 增加值=%.2f"), *GetNameSafe(User), StaminaBoostAmount);
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("提升体力上限失败：目标 %s 身上没有 SprintComponent。"), *GetNameSafe(User));
	return false;
}
