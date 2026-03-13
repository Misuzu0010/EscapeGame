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
class UClothLODControllerComponent;
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

	// --- 线程安全数据中转站 (GameThread 写入, WorkerThread 读取) ---
	// 【修改点】：缓存完整的速度向量，而不仅仅是标量
	UPROPERTY(Transient)
	FVector CachedVelocity = FVector::ZeroVector;

	UPROPERTY(Transient)
	float CachedClothLODFactor = 0.0f; // 缓存LOD因子

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kawaii Physics|Output")
	float PhysicsAlpha = 1.0f;

	// 这里的 FakeWindForce 变成了真实的 KawaiiWind
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kawaii Physics|Output")
	FVector KawaiiWind = FVector::ZeroVector;

	// 动态阻尼：跑得越快，阻尼越大，防止乱甩
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kawaii Physics|Output")
	float DynamicDamping = 1.35f;
	// 动态阻尼：跑得越快，阻尼越大，防止乱甩
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kawaii Physics|Output")
	float DynamicDampingForIdle = 1.35f;
	// 动态阻尼：跑得越快，阻尼越大，防止乱甩
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kawaii Physics|Output")
	float DynamicDampingForMoving = 1.05f;

	// --- 线程安全数据中转站 (GameThread 写入, WorkerThread 读取) ---
	float CurrentFrameSpeed = 0.0f;

	// --- 内部缓存的组件指针 ---
	TWeakObjectPtr<UWindSimulationComponent>WindComponent;
	TWeakObjectPtr<UClothLODControllerComponent> ClothLODComponent; // 【新增】缓存LOD组件
};
