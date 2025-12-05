// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SprintComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSprintChanged, float, CurrentSprint);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ESCAPEGAME_API USprintComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USprintComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Start/Stop sprint (可以在 Character 中调用)
    UFUNCTION(BlueprintCallable, Category = "Sprint")
    void StartSprinting();

    UFUNCTION(BlueprintCallable, Category = "Sprint")
    void StopSprinting();

    // 供 Character 查询
    UFUNCTION(BlueprintCallable, Category = "Sprint")
    bool IsSprinting() const { return bIsSprinting; }

    // 设置基础速度（可在蓝图/编辑器改）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
    float WalkSpeed = 400.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
    float SprintSpeed = 700.f;

    // 是否使用插值平滑速度（在 Character Tick 中插值）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
    bool bSmoothSpeed = true;

    // 插值速度系数（越大切换越快）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint", meta = (EditCondition = "bSmoothSpeed"))
    float SpeedInterpRate = 10.f;

    // 冲刺条属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
    float MaxSprint = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sprint")
    float CurrentSprint = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
    float SprintConsumeRate = 25.0f; // 每秒消耗

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
    float SprintRecoverRate = 15.0f; // 每秒恢复

    // 广播给 UI
    UPROPERTY(BlueprintAssignable, Category = "Sprint")
    FOnSprintChanged OnSprintChanged;

    /** 冲刺输入动作 (插座) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputAction* SprintAction; // <--- 加上这行！

protected:
    bool bIsSprinting = false;

    // 供 Character 使用：获取目标速度
public:
    float GetTargetSpeed() const { return bIsSprinting ? SprintSpeed : WalkSpeed; }
};
