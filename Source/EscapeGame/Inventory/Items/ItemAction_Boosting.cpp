// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Items/ItemAction_Boosting.h"
#include "Character/Components/SprintComponent.h"
bool UItemAction_Boosting::OnUse_Implementation(AActor* TargetActor)
{
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("使用加速药水失败：TargetActor 为空，无法查找 SprintComponent。"));
        return false;
    }

    // 加速效果只通过 SprintComponent 进入，避免道具直接改 CharacterMovement 后被冲刺组件下一帧覆盖。
    USprintComponent* SprintComp = TargetActor->FindComponentByClass<USprintComponent>();

    if (SprintComp)
    {
        // 2. 发号施令！
        // 调用我们在 SprintComponent 里写好的那个带 Timer 的函数
        SprintComp->StartSpeedBuff(Duration, SpeedMultiplier);

        UE_LOG(LogTemp, Log, TEXT("喵！使用了加速药水 %s！倍率: %f, 持续: %f 秒"),*ItemName.Name.ToString(), SpeedMultiplier, Duration);
		return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("使用了加速药水 %s，但身上没有 SprintComponent！无法加速！"), *ItemName.Name.ToString());
		return false;
    }
}
