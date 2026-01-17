// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SprintComponent.generated.h"

// 体力值改变广播 (用于UI更新)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, CurrentStamina, float, MaxStamina);

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

    UFUNCTION(BlueprintCallable, Category = "Sprint")
	float GetCurrentStaminaPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Sprint")
    void ApplyStaminaChange();

    UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StaminaChange(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Sprint")
    void ApplyMaxChange(float Delta);

    // === 属性配置 ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
    float StaminaConsumeRate = 20.0f; // 每秒消耗

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
    float StaminaRegenRate = 10.0f; // 每秒恢复
	//issprinting=true，不启用恢复

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
    float WalkSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
    float SprintSpeed = 1000.0f;

    // 广播代理
    UPROPERTY(BlueprintAssignable, Category = "Sprint")
    FOnStaminaChanged OnStaminaChanged;

    /** 冲刺输入动作 (插座) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputAction* SprintAction; // <--- 加上这行！
    // 供 Character 使用：获取目标速度

    bool bStaminaDrained;//是否耗尽

    float StaminaRegenDelay;//体力恢复延迟计时器

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
    float MaxStaminaRegenDelay = 1.0f; //体力恢复延迟时间

public:
    float CurrentStamina;

    bool bSprintRequested; // 玩家是否按下了 Shift

	bool bIsActurallySprinting; // 实际是否在冲刺

    // 缓存引用
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComp;

    UPROPERTY()
    class UStateMachineComponent* StateMachine;


    // 修改之前的 Buff 函数接口，不需要传时间，只传倍率
    UFUNCTION(BlueprintCallable, Category = "Buff")
    void SetSpeedBuffMultiplier(float NewMultiplier);

    //新增：更新速度的工具函数 (DRY原则)
    void UpdateMovementSpeed();

    // 现在的 Buff 倍率 (1.0 表示没 Buff)
    float CurrentBuffMultiplier = 1.0f;

    FTimerHandle TimerHandle_Buff;

    void StartSpeedBuff(float Duration, float Multiplier);
};
