// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WindSimulationComponent.generated.h"
class UCurveFloat;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ESCAPEGAME_API UWindSimulationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWindSimulationComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	//基础全局风力
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wind|Settings")
	FVector BasicGlobalWind;
	//角色移动对风的影响程度（负数表示迎风，正数表示顺风）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wind|Settings")
	float MovementWindScale;
	//风力变化的插值速度，数值越大变化越快，数值越小变化越平滑
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wind|Settings")
	float WindInterpSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Wind|Settings")
	UCurveFloat* WindNoiseCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Wind|Settings")
	float CurvePlayRate;

	// 噪声更新频率：多久计算一次柏林噪声（例如 0.05 表示每秒20次）
	UPROPERTY(EditDefaultsOnly, Category = "Wind|Optimization")
	float NoiseUpdateInterval;

	// 空间缩放：决定不同位置风力的差异程度（值越小，大范围内的风向越一致）
	UPROPERTY(EditAnywhere, Category = "Wind|Noise")
	float NoiseSpatialScale;

	// 时间缩放：决定风力随时间变化的速度
	UPROPERTY(EditAnywhere, Category = "Wind|Noise")
	float NoiseTimeScale;

	// 噪声强度：扰动风力的最大幅度
	UPROPERTY(EditAnywhere, Category = "Wind|Noise")
	float NoiseIntensity;



public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	inline FVector GetCurrentWind() const { return CurrentWind; }
private:
	FVector CurrentWind;
	FVector TargetWind;
	FVector CachedNoiseWind;
	UPROPERTY(Transient)
	AActor* CachedOwner;
	float TimeSinceLastWindUpdate;
		
};
