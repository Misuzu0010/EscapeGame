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
class USprintComponent;
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
	// 极致优化：既不存盘（Transient），又允许蓝图读取（BlueprintReadOnly），还保证了 C++ 的封装性（AllowPrivateAccess）
	
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	FVector CachedVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float VerticalVelocity = 0.0f;


	UPROPERTY(BlueprintReadOnly, Transient, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float GroundSpeed = 0.0f;

	UPROPERTY(Transient)
	float CachedClothLODFactor = 0.0f; // 缓存LOD因子

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kawaii Physics|Output")
	float PhysicsAlpha = 1.0f;

	// 这里的 FakeWindForce 变成了真实的 KawaiiWind
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kawaii Physics|Output")
	FVector KawaiiWind = FVector::ZeroVector;

	// 运行时头发阻尼：由移动速度插值驱动，输出给 Kawaii Physics。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kawaii Physics|Output")
	float RuntimeHairDamping = 1.35f;

	// 低速头发阻尼：待机或慢速移动时使用，保留更轻的摆动。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kawaii Physics|Output")
	float HairDampingLowSpeed = 1.05f;

	// 高速头发阻尼：跑动时提高阻尼，防止发束乱甩。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kawaii Physics|Output")
	float HairDampingHighSpeed = 1.35f;

	// --- 线程安全数据中转站 (GameThread 写入, WorkerThread 读取) ---
	float CurrentFrameSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bShouldMove= false;

	// 是否在空中
	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsFalling = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsRunning = false;
	
	

	// --- 动画匹配专用缓存 ---
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	FRotator CachedRotation = FRotator::ZeroRotator;

	// 当前移动角度 (-180 到 180)
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float LocomotionAngle = 0.0f;

	// 动态播放速率 (用于对齐脚步)
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float LocomotionPlayRate = 1.0f;

	// 动画资产的标准奔跑速度 (TA 需要根据具体的跑步动画位移速度来设定此值)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Config")
	float AuthoredRunSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kawaii Physics|Run Bounce")
	FVector HairRunBounceForce = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kawaii Physics|Run Bounce")
	float HairRunBounceStrength = 120.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kawaii Physics|Run Bounce")
	float HairRunBounceFrequency = 2.6f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kawaii Physics|Run Bounce")
	float HairRunBounceMinSpeed=120.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kawaii Physics|Run Bounce")
	float HairRunBounceMaxSpeed=600.f;
	

	// --- 内部缓存的组件指针 ---
	TWeakObjectPtr<UWindSimulationComponent>WindComponent;
	TWeakObjectPtr<UClothLODControllerComponent> ClothLODComponent; // 【新增】缓存LOD组件
	TWeakObjectPtr<USprintComponent>SprintComp;
};
