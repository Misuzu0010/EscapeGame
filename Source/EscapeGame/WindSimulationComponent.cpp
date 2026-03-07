// Fill out your copyright notice in the Description page of Project Settings.


#include "WindSimulationComponent.h"
#include"Curves/CurveFloat.h"
#include"Engine/World.h"
#include"GameFrameWork/Actor.h"


// Sets default values for this component's properties
UWindSimulationComponent::UWindSimulationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;

	BasicGlobalWind = FVector(30.f, 30.f, 5.f);
	MovementWindScale = -0.2f;
	WindInterpSpeed = 4.f;
	CurvePlayRate = 0.5f;
	CachedOwner = nullptr;
	NoiseUpdateInterval = 0.05f;
	TimeSinceLastWindUpdate = 0.0f;

	NoiseSpatialScale = 0.01f;
	NoiseTimeScale = 1.0f;
	NoiseIntensity = 50.0f;
	CachedNoiseWind = FVector::ZeroVector;
	// ...
}


// Called when the game starts
void UWindSimulationComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UWindSimulationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValid(CachedOwner)) { UE_LOG(LogTemp, Log, TEXT("非法的actor，请查看代码内容")); return; }
	//角色移动模拟风力
	TimeSinceLastWindUpdate += DeltaTime;

	// 核心优化：只在达到时间间隔时，才执行昂贵的 3D Perlin 计算
	if (TimeSinceLastWindUpdate >= NoiseUpdateInterval)
	{
		FVector WorldLoc = CachedOwner->GetActorLocation();
		float CurrentTime = GetWorld()->GetTimeSeconds();
		FVector NoiseInput = WorldLoc * NoiseSpatialScale;

		// 带有偏移量的噪声采样
		float NoiseX = FMath::PerlinNoise3D(NoiseInput + FVector(CurrentTime * NoiseTimeScale, 0.f, 0.f));
		float NoiseY = FMath::PerlinNoise3D(NoiseInput + FVector(0.f, CurrentTime * NoiseTimeScale + 100.f, 0.f));
		float NoiseZ = FMath::PerlinNoise3D(NoiseInput + FVector(0.f, 0.f, CurrentTime * NoiseTimeScale + 200.f));

		// 更新缓存的噪声结果
		CachedNoiseWind = FVector(NoiseX, NoiseY, NoiseZ) * NoiseIntensity;

		// 重置计时器，保留溢出的时间以保证精确性
		TimeSinceLastWindUpdate = FMath::Fmod(TimeSinceLastWindUpdate, NoiseUpdateInterval);
	}

	// 每一帧必做的轻量级计算：获取速度、合并与插值
	FVector VelocityWind = CachedOwner->GetVelocity() * MovementWindScale;

	// 目标风力 = 基础风力 + 缓存的噪声风力 + 移动风力
	TargetWind = BasicGlobalWind + CachedNoiseWind + VelocityWind;

	// 利用 VInterpTo 在帧与帧之间进行极其丝滑的平滑过渡，完美掩盖降频带来的卡顿感！
	CurrentWind = FMath::VInterpTo(CurrentWind, TargetWind, DeltaTime, WindInterpSpeed);

	// ...
}

