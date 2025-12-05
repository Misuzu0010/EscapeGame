//// Fill out your copyright notice in the Description page of Project Settings.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "Components/ActorComponent.h"
//#include "Components/BoxComponent.h"
//#include "NiagaraComponent.h"
//#include "TimerManager.h"
//#include "Animation/AnimMontage.h"
//#include "StateMachineComponent.generated.h"
//
//UENUM(BlueprintType)
//enum class ECharacterState : uint8
//{
//	Idle     UMETA(DisplayName = "Idle"),
//	Moving   UMETA(DisplayName = "Moving"),
//	Attacking UMETA(DisplayName = "Attacking"),
//	Sprinting UMETA(DisplayName = "Sprinting"),
//	Stunned  UMETA(DisplayName = "Stunned"),
//	Dead     UMETA(DisplayName = "Dead")
//};
//
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChanged, ECharacterState, NewState);
//
//UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
//class ESCAPEGAME_API UStateMachineComponent : public UActorComponent
//{
//	GENERATED_BODY()
//
//public:	
//	// Sets default values for this component's properties
//	UStateMachineComponent();
//
//protected:
//	// Called when the game starts
//	virtual void BeginPlay() override;
//
//public:	
//	// Called every frame
//	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
//
//	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
//	ECharacterState CurrentState = ECharacterState::Idle;
//
//	void SetState(ECharacterState NewState);
//
//	//定时器句柄
//
//	FTimerHandle StunTimerHandle;
//
//	FTimerHandle DeathTimerHandle;
//
//	//开关
//	bool bCanMove = true;
//	bool bCanAttack = true;
//
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
//	UNiagaraComponent* StunVFX;
//
//	UFUNCTION()
//	void OnStunEnd();
//
//	UFUNCTION()
//	void OnDeathFinished();
//
//	UFUNCTION()
//	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
//
//	//伤害函数
//	UFUNCTION()
//	void OnWeaponHit();
//
//	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
//	UBoxComponent* WeaponCollisionBox;
//
//	// 蓝图可设置的各种参数
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stun")
//	float StunDuration = 2.0f;
//
//	// 动画蒙太奇
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
//	UAnimMontage* AttackMontage;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
//	UAnimMontage* StunMontage;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
//	UAnimMontage* DeathMontage;
//
//	// 自己实现的声音或脚步函数
//	void PlayFootstepSound();
//	void StopFootstepSound();
//
//	// 武器伤害数组，可在蓝图中设置每段攻击伤害
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
//	TArray<float> ComboDamage;
//
//	//連擊計數
//	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
//	int32 ComboStep = 0;
//
//	//上一次連擊時間計時器
//	float LastAttackTime = 0.0f;
//
//	//Combo最大段數
//	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
//	int32 MaxCombo = 3;
//
//	//Combo時間窗口
//	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
//	float ComboInputWindow = 0.5f;
//
//	//可接受連擊輸入（即是否被打斷）
//	bool bCanReceiveComboInput = false;
//
//	void AttackInput();
//
//	//連擊函數
//	void PlayComboStep(int Step);
//
//	//連擊動畫
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Attack")
//	UAnimMontage* AttackMontage1;
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Attack")
//	UAnimMontage* AttackMontage2;
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Attack")
//	UAnimMontage* AttackMontage3;	
//};
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "TimerManager.h"
#include "Animation/AnimMontage.h"
#include "StateMachineComponent.generated.h"

// 前置声明，防止循环引用
class ACharacter; 
class UBoxComponent;
class UNiagaraComponent;
class UAnimMontage;

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Idle        UMETA(DisplayName = "Idle"),
	Moving      UMETA(DisplayName = "Moving"),
	Attacking   UMETA(DisplayName = "Attacking"),
	Sprinting   UMETA(DisplayName = "Sprinting"), // 记得和 SprintComponent 同步
	Stunned     UMETA(DisplayName = "Stunned"),
	Dead        UMETA(DisplayName = "Dead")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChanged, ECharacterState, NewState);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ESCAPEGAME_API UStateMachineComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UStateMachineComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === 核心状态逻辑 ===
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State Machine")
	ECharacterState CurrentState;

    // 这是一个代理，蓝图可以绑定它来更新UI
    UPROPERTY(BlueprintAssignable, Category = "State Machine")
    FOnStateChanged OnStateChanged;

	UFUNCTION(BlueprintCallable, Category = "State Machine")
	void SetState(ECharacterState NewState);

    // 检查当前是否可以执行某些操作
    bool CanMove() const;
    bool CanAttack() const;

    // === 缓存的引用 (关键！以后就靠它指挥角色) ===
protected:
    UPROPERTY()
    ACharacter* OwnerCharacter;

public:
    // === 战斗与连击系统 ===

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    bool bCanAttack = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    bool bCanMove = true;

	// 连击段数 (当前是第几段)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 ComboIndex = 0;

    // 最大的连击段数 (比如3连击)
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    int32 MaxComboCount = 3;

    // 是否接受连击输入 (窗口期)
    bool bAcceptingComboInput = false;

    // 玩家是否按下了攻击键 (缓存输入)
    bool bInputBuffer = false; 

    // 攻击蒙太奇数组 (填3个动画)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TArray<UAnimMontage*> AttackMontages;

    // 基础伤害值数组
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TArray<float> ComboDamage;

    // 处理玩家按下攻击键
	void HandleAttackInput();

    // 播放具体的攻击动画
    void PlayComboAttack();

    // 动画通知回调 (需要在蒙太奇里设置 Notify)
    UFUNCTION(BlueprintCallable)
    void EnableComboWindow(); // 开启输入窗口

    UFUNCTION(BlueprintCallable)
    void DisableComboWindow(); // 关闭输入窗口

    // === 受击与状态重置 ===
    
	FTimerHandle StunTimerHandle;
	FTimerHandle DeathTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float StunDuration = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* StunMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* DeathMontage;

	UFUNCTION()
	void OnStunEnd();

	UFUNCTION()
	void OnDeathFinished();

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    // === 碰撞检测 (建议还是放在 Character 里做，或者这里只存指针) ===
    // 如果你非要在这写逻辑，你需要让 Character 把碰撞事件转发过来
    UFUNCTION()
    void OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

