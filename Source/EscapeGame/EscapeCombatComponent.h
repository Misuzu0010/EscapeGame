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

	//检查蓄力按键是否持续按住，决定循环或释放
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CheckChargedAttack();
	
	// 3. 触发函数 (Trigger Function) - 可选，用于封装 Broadcast
	// 范式：Broadcast + [事件名] 或 Notify + [事件名]
	void BroadcastComboChange(int32 NewCount);


private:

    // EscapeCombatComponent.h

    UFUNCTION()
    void HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UPROPERTY(Transient)
    int32 ComboCount = 0;

    UPROPERTY(Transient)
    float CachedAttackInputTime = 0.0f;

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



protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    class ACharacter* GetOwnerCharacter() const;

		
};
