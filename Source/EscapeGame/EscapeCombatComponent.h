// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include"GameplayTagContainer.h"
#include "Interface/EscapeCombatDamageable.h"
#include "EscapeCombatComponent.generated.h"
class UCharacterAnimData;
class ACharacter;
class UAnimMontage;
class UAnimInstance;
class UAttributeComponent;
class USprintComponent;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	Idle            UMETA(DisplayName = "空闲"),
	Attacking       UMETA(DisplayName = "轻击连招中"),
	Charging        UMETA(DisplayName = "蓄力中"),
	HeavyAttacking  UMETA(DisplayName = "重击释放中")
};
USTRUCT(BlueprintType)
struct FAttackHitPayload
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Hit")
    TObjectPtr<AActor> DamageCauser = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Hit")
    FVector DamageLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Hit")
    FVector DamageImpulse = FVector::ZeroVector;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackHitSignature, FAttackHitPayload, HitData);
// 传递连击数变化
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboCountChangedSignature, int32, NewComboCount);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ESCAPEGAME_API UEscapeCombatComponent : public UActorComponent, public IEscapeCombatDamageable
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEscapeCombatComponent();


    // ==========================================
    // 配置项 (Configuration)
    // ==========================================
    // 核心数据库：请在蓝图里把你的 DataAsset 拖进去！
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<UCharacterAnimData> CharacterAnimData;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnAttackHitSignature OnAttackHit;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnComboCountChangedSignature OnComboCountChanged;


    // ==========================================
    // 接口 (Interface)
    // ==========================================
    // 尝试执行某个动作 (查表播放)
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TryPlayActionByTag(FGameplayTag ActionTag);

	//从组件获取当前动作的 Trace 参数，执行 SweepMultiByObjectType
	//攻击判定起点骨骼/socket名称
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void DoAttackTrace(FName DamageSourceBone);

	//检查输入缓冲时间戳，决定是否进入连击状态
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CheckCombo();

    // 蓝图 Enhanced Input 的 Started 或 Triggered 引脚调用此函数
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void BeginOrUpdateChargedAttack();

    // 蓝图 Enhanced Input 的 Completed 或 Canceled 引脚调用此函数
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ReleaseChargedAttack();
    
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RequestLightAttack();
	
	
	UFUNCTION(BlueprintCallable, Category = "Combat|Tags")
	bool HasCombatTag(FGameplayTag TagToCheck) const
	{
		// 真正调用底层容器的 HasTag 函数！
		return ActiveTags.HasTag(TagToCheck);
	}
	
	
	// 3. 触发函数 (Trigger Function) - 可选，用于封装 Broadcast
	// 范式：Broadcast + [事件名] 或 Notify + [事件名]
	void BroadcastComboChange(int32 NewCount);
	
	
	// 只需要这两个绑定函数
	void Input_AttackStarted();
	void Input_AttackCompleted();


private:

    

    UFUNCTION()
    void HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UPROPERTY(Transient)
    int32 ComboCount = 0;
	
	//
    UPROPERTY(Transient)
	bool bHasSavedComboInput = false;

    UPROPERTY(Transient)
	FGameplayTag CurrentActionTag;

    UPROPERTY(Transient)
    FGameplayTagContainer ActiveTags;

    UPROPERTY(Transient)
    TObjectPtr<ACharacter>OwnerCharacter;

    UPROPERTY(Transient)
	TObjectPtr<UAnimInstance> CachedAnimInstance;

    UPROPERTY(Transient)
    TObjectPtr<UAttributeComponent>AttributeComp;

    UPROPERTY(Transient)
    TObjectPtr<USprintComponent>SprintComp;
	
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> CurrentPlayingMontage;
	
	// 增加一个定时器句柄，用来记录按下的时间
	UPROPERTY(Transient)
	FTimerHandle InputBufferTimer;



protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    class ACharacter* GetOwnerCharacter() const;

		
};
