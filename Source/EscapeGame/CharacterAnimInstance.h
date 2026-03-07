// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterAnimInstance.generated.h"

/**
 * 
 */
class ACharacter;
class UCharacterMovementComponent;
class UWindSimulationComponent;
UCLASS()
class ESCAPEGAME_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaTime) override; // 极致优化：使用线程安全更新
	// 游戏线程更新：用于安全地跨组件获取数据
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
protected:
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent>MovementComponent;
	// --- 输出给 Kawaii Physics 的变量 ---

	// 控制物理开启程度 (0=完全关闭, 1=完全开启)
	// 比如：当武器穿模严重时，可以通过代码降低这个值

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kawaii Physics|Output")
	float PhysicsAlpha = 1.0f;

	// 这里的 FakeWindForce 变成了真实的 KawaiiWind
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kawaii Physics|Output")
	FVector KawaiiWind = FVector::ZeroVector;

	// 动态阻尼：跑得越快，阻尼越大，防止乱甩
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kawaii Physics|Output")
	float DynamicDamping = 0.1f;

	UPROPERTY(Transient)
	TObjectPtr<UWindSimulationComponent>WindComponent;
};
