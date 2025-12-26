// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ItemAction_Boosting.h"
#include "EscapeGame/SprintComponent.h"
void UItemAction_Boosting::OnUse_Implementation(AActor* User)
{
    if (!User) return;

    // 1. 尝试找到主人的腿（SprintComponent）
    USprintComponent* SprintComp = User->FindComponentByClass<USprintComponent>();

    if (SprintComp)
    {
        // 2. 发号施令！
        // 调用我们在 SprintComponent 里写好的那个带 Timer 的函数
        SprintComp->StartSpeedBuff(Duration, SpeedMultiplier);

        UE_LOG(LogTemp, Log, TEXT("喵！使用了加速药水！倍率: %f, 持续: %f 秒"), SpeedMultiplier, Duration);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("使用了加速药水，但身上没有 SprintComponent！无法加速！"));
    }
}
