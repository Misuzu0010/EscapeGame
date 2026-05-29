// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include"GameplayTagContainer.h"
#include "Interface/EscapeCombatDamageable.h"
#include "Containers/Set.h"
#include "UObject/ObjectKey.h"
#include "EscapeCombatComponent.generated.h"
class UCharacterAnimData;
class ACharacter;
class UAnimMontage;
class UAnimInstance;
class UAttributeComponent;
class USprintComponent;
class UMeshComponent;
class UWeaponDefinition;
class UStateMachineComponent;

USTRUCT()
struct FCombatRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	int32 ComboCount = 0;

	UPROPERTY(Transient)
	bool bHasSavedComboInput = false;

	UPROPERTY(Transient)
	FGameplayTag CurrentActionTag;

	UPROPERTY(Transient)
	FGameplayTagContainer ActiveTags;

	UPROPERTY(Transient)
	FGameplayTagContainer CurrentActionTags;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> CurrentPlayingMontage = nullptr;
	
	TSet<TObjectKey<AActor>> HitActorsThisAction;

	UPROPERTY(Transient)
	FTimerHandle InputBufferTimer;

	void BeginAction(FGameplayTag ActionTag, UAnimMontage* MontageToPlay)
	{
		ClearCurrentActionTags();

		CurrentActionTag = ActionTag;
		CurrentPlayingMontage = MontageToPlay;
		
		HitActorsThisAction.Reset();

		AddActionTag(ActionTag);
	}

	void ResetAction()
	{
		ComboCount = 0;
		bHasSavedComboInput = false;
		CurrentActionTag = FGameplayTag::EmptyTag;
		CurrentPlayingMontage = nullptr;
		HitActorsThisAction.Reset();
		ClearCurrentActionTags();
	}

	void AddCombatTag(FGameplayTag TagToAdd)
	{
		if (TagToAdd.IsValid())
		{
			ActiveTags.AddTagFast(TagToAdd);
		}
		else
		{
			UE_LOG(LogTemp,Error,TEXT("打不上Tag %s喵"),*TagToAdd.ToString());
		}
	}

	void RemoveCombatTag(FGameplayTag TagToRemove)
	{
		if (TagToRemove.IsValid())
		{
			ActiveTags.RemoveTag(TagToRemove);
			CurrentActionTags.RemoveTag(TagToRemove);
		}
		else
		{
			UE_LOG(LogTemp,Error,TEXT("去不掉Tag %s喵"),*TagToRemove.ToString());
		}
	}
	void AddActionTag(FGameplayTag Tag)
	{
		if (Tag.IsValid())
		{
			ActiveTags.AddTag(Tag);
			CurrentActionTags.AddTag(Tag);
		}
	}

	void ClearCurrentActionTags()
	{
		for (auto It = CurrentActionTags.CreateConstIterator(); It; ++It)
		{
			ActiveTags.RemoveTag(*It);
		}

		CurrentActionTags.Reset();
	}
};

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
class ESCAPEGAME_API UEscapeCombatComponent : public UActorComponent
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

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Combat")
	float CombatWindow;


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
		return RuntimeState.ActiveTags.HasTag(TagToCheck);
	}
	
	
	// 3. 触发函数 (Trigger Function) - 可选，用于封装 Broadcast
	// 范式：Broadcast + [事件名] 或 Notify + [事件名]
	void BroadcastComboChange(int32 NewCount);
	
	
	// 只需要这两个绑定函数
	void Input_AttackStarted();
	void Input_AttackCompleted();
	
	// EscapeCombatComponent.h
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetCurrentActionBaseDamage() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	int32 GetCurrentComboCount() const
	{
		return RuntimeState.ComboCount;
	}
	
	void SetEquippedWeapon(UWeaponDefinition* WeaponDef, UMeshComponent* WeaponMesh);
	void ClearEquippedWeapon();


private:

	bool TryPlayActionByTagInternal(FGameplayTag ActionTag);

	bool CanStartCombatAction(FGameplayTag ActionTag, FString* OutFailReason = nullptr) const;

    UFUNCTION()
    void HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(Transient)
	FCombatRuntimeState RuntimeState;
	
    UPROPERTY(Transient)
    TObjectPtr<ACharacter>OwnerCharacter;

    UPROPERTY(Transient)
	TObjectPtr<UAnimInstance> CachedAnimInstance;

    UPROPERTY(Transient)
    TObjectPtr<UAttributeComponent>AttributeComp;

    UPROPERTY(Transient)
    TObjectPtr<USprintComponent>SprintComp;
	
	UPROPERTY(Transient)
	TObjectPtr<UStateMachineComponent>StateMachineComp;
	
	UPROPERTY(Transient)
	TObjectPtr<UWeaponDefinition> EquippedWeaponDef;

	UPROPERTY(Transient)
	TObjectPtr<UMeshComponent> EquippedWeaponMesh;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    class ACharacter* GetOwnerCharacter() const;

		
};
