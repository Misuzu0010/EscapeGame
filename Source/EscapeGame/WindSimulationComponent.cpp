// Fill out your copyright notice in the Description page of Project Settings.


#include "WindSimulationComponent.h"
#include"Curves/CurveFloat.h"
#include"Engine/World.h"
#include"GameFrameWork/Actor.h"
#include "Engine/Scene.h" 
#include"Engine/Engine.h"
// Sets default values for this component's properties
UWindSimulationComponent::UWindSimulationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;

	BasicGlobalWind = FVector(300.f, 300.f, 200.f);
	MovementWindScale = -0.00001f;
	WindInterpSpeed = 1.2f;
	CurvePlayRate = 0.5f;
	CachedOwner = nullptr;
	MaxVelocityWindForce = 150.f;
	

	NoiseSpatialScale = 0.01f;
	NoiseTimeScale = 1.0f;
	NoiseIntensity = 95.0f;
	TimerSettings = 0.0f;
	CachedNoiseWind = FVector::ZeroVector;
	// ...
}


// Called when the game starts
void UWindSimulationComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	CachedOwner = GetOwner();
	
}


// Called every frame
void UWindSimulationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TimerSettings -= DeltaTime;
	// --- 放在 WindSimulationComponent.cpp 的 TickComponent 函数最前面 ---
	if (!IsValid(CachedOwner)) { UE_LOG(LogTemp, Log, TEXT("非法的actor，请查看代码内容")); return; }
	// 每一帧必做的轻量级计算：获取速度、合并与插值
	FVector VelocityWind = CachedOwner->GetVelocity() * MovementWindScale;
	// 无论跑多快，向后的风力绝不能超过 MaxVelocityWindForce！
	FVector ClampedVelocityWind = VelocityWind.GetClampedToMaxSize(MaxVelocityWindForce);

	// 计算基础目标风（全局风 + 安全的移动风）
	FVector BaseTargetWind = BasicGlobalWind + ClampedVelocityWind;
	
	// 【关键修正】只对大方向的基础风进行 VInterpTo 插值，保持丝滑的惯性
	CurrentWind = FMath::VInterpTo(CurrentWind, BaseTargetWind, DeltaTime, WindInterpSpeed);


	FVector WorldLoc = CachedOwner->GetActorLocation();
	float CurrentTime = GetWorld()->GetTimeSeconds();

	FVector NoiseInput = WorldLoc * NoiseSpatialScale + FVector(CurrentTime * NoiseTimeScale);

	// 3. 构造飘带专用的“起伏风”
	// X, Y 轴使用柏林噪声提供无序的横向摆动
	float NoiseX = FMath::PerlinNoise3D(NoiseInput);
	float NoiseY = FMath::PerlinNoise3D(NoiseInput + FVector(100.f));
	// Z 轴（垂直方向）融合正弦波
	float SineWaveZ = FMath::Sin(CurrentTime * 3.0f) * 0.5f;
	float NoiseZ = FMath::PerlinNoise3D(NoiseInput + FVector(200.f)) * 0.5f + SineWaveZ;
	CachedNoiseWind = FVector(NoiseX, NoiseY, NoiseZ) * NoiseIntensity;
	// 目标风力 = 基础风力 + 缓存的噪声风力
	TargetWind = CurrentWind + CachedNoiseWind;;

	if (TimerSettings<=0.0f) {
		UE_LOG(LogTemp, Warning, TEXT("TargetWind: X=%.2f Y=%.2f Z=%.2f"),
			TargetWind.X, TargetWind.Y, TargetWind.Z);
		TimerSettings = 0.5f;// 每0.5秒输出一次日志，避免过于频繁
	}
	

	// ...
}

