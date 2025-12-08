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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStateChanged, ECharacterState, NewState, ECharacterState, OldState);

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
    
	// 获取当前状态
	UFUNCTION(BlueprintPure, Category = "State Machine")
	ECharacterState GetCurrentState() const { return CurrentState; }

	// 检查是否处于某种状态
	UFUNCTION(BlueprintPure, Category = "State Machine")
	bool IsState(ECharacterState StateToCheck) const { return CurrentState == StateToCheck; }

    // 这是一个代理，蓝图可以绑定它来更新UI
    UPROPERTY(BlueprintAssignable, Category = "State Machine")
    FOnStateChanged OnStateChanged;

	//状态机核心函数
	UFUNCTION(BlueprintCallable, Category = "State Machine")
	void SetState(ECharacterState NewState);

	// 申请进入眩晕（外部调用，比如被怪打了）
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyStun(float Duration);

	// 申请死亡
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyDeath();

    // === 缓存的引用 (关键！以后就靠它指挥角色) ===
protected:
	
	void OnStunFinished();

private:
	UPROPERTY(VisibleAnywhere, Category = "State Machine", meta = (AllowPrivateAccess = "true"))
	ECharacterState CurrentState;

	FTimerHandle TimerHandle_Stun;


};

