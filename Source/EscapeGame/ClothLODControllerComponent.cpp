// Fill out your copyright notice in the Description page of Project Settings.

#include "ClothLODControllerComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

UClothLODControllerComponent::UClothLODControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;

	CurrentLODIndex = 0;
	CurrentLODFactor = 0.f;
	SmoothedBlendWeight = 1.f;
	SmoothedMaxDistScale = 1.f;
	bSimulationActive = true;
	bFirstEvaluation = true;
	PreviousLocation = FVector::ZeroVector;

	LODProfiles = {
		{ 1000.f,  1.0f, 1.0f, true  },
		{ 2500.f,  0.7f, 0.7f, true  },
		{ 4000.f,  0.3f, 0.4f, true  },
		{ 6000.f,  0.0f, 0.0f, false },
	};
}

void UClothLODControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!TargetMesh)
	{
		if (AActor* Owner = GetOwner())
		{
			TargetMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
		}
	}
	CachedMesh = TargetMesh;

	if (!CachedMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("ClothLODController: 未找到 SkeletalMeshComponent，组件不会生效"));
		return;
	}

	LODProfiles.Sort([](const FClothLODProfile& A, const FClothLODProfile& B)
		{
			return A.MaxDistance < B.MaxDistance;
		});

	CachedDistanceSqThresholds.Reset(LODProfiles.Num());
	for (const FClothLODProfile& Profile : LODProfiles)
	{
		CachedDistanceSqThresholds.Add(Profile.MaxDistance * Profile.MaxDistance);
	}

	if (AActor* Owner = GetOwner())
	{
		PreviousLocation = Owner->GetActorLocation();
	}

	UWorld* World = GetWorld();
	if (World && LODProfiles.Num() > 0)
	{
		// 随机初始延迟，多实例自动错峰
		float InitialDelay = FMath::FRandRange(0.f, EvaluationInterval);
		World->GetTimerManager().SetTimer(
			EvalTimerHandle,
			this,
			&UClothLODControllerComponent::OnTimerEvaluate,
			EvaluationInterval,
			true,
			InitialDelay
		);
	}
}

void UClothLODControllerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(EvalTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

// ==================== 定时器回调 ====================

void UClothLODControllerComponent::OnTimerEvaluate()
{
	if (!CachedMesh || LODProfiles.Num() == 0)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// 不可见时直接关闭模拟，跳过后续所有计算
	if (bDisableWhenHidden && !CachedMesh->WasRecentlyRendered(0.5f))
	{
		if (bSimulationActive)
		{
			ApplyToMesh(0.f, 0.f, false);
			bSimulationActive = false;
		}
		return;
	}

	FVector CurrentLocation = Owner->GetActorLocation();

	DetectTeleportation(CurrentLocation);

	// 计算到相机的距离平方
	float DistanceSq = 0.f;
	if (const UWorld* World = GetWorld())
	{
		if (const APlayerController* PC = World->GetFirstPlayerController())
		{
			if (const APlayerCameraManager* CamMgr = PC->PlayerCameraManager)
			{
				DistanceSq = FVector::DistSquared(CurrentLocation, CamMgr->GetCameraLocation());
			}
		}
	}

	int32 DistanceLOD = ResolveDistanceLOD(DistanceSq);
	int32 FinalLOD = bFuseMeshLOD ? FuseWithMeshLOD(DistanceLOD) : DistanceLOD;

	SmoothAndApply(FinalLOD, EvaluationInterval);

	PreviousLocation = CurrentLocation;
	bFirstEvaluation = false;
}

// ==================== LOD 评估 ====================

int32 UClothLODControllerComponent::ResolveDistanceLOD(float DistanceSq) const
{
	for (int32 i = 0; i < CachedDistanceSqThresholds.Num(); ++i)
	{
		if (DistanceSq < CachedDistanceSqThresholds[i])
		{
			return i;
		}
	}
	return FMath::Max(0, LODProfiles.Num() - 1);
}

int32 UClothLODControllerComponent::FuseWithMeshLOD(int32 DistanceLOD) const
{
	if (!CachedMesh)
	{
		return DistanceLOD;
	}

	int32 MeshLOD = CachedMesh->GetPredictedLODLevel();
	int32 MappedClothLOD = MeshLOD + MeshLODOffset;

	// 取更保守（数值更大=质量更低）的级别
	int32 FusedLOD = FMath::Max(DistanceLOD, MappedClothLOD);
	return FMath::Clamp(FusedLOD, 0, LODProfiles.Num() - 1);
}

// ==================== 参数平滑与应用 ====================

void UClothLODControllerComponent::SmoothAndApply(int32 TargetLOD, float DeltaTime)
{
	TargetLOD = FMath::Clamp(TargetLOD, 0, LODProfiles.Num() - 1);
	CurrentLODIndex = TargetLOD;

	const FClothLODProfile& Target = LODProfiles[TargetLOD];

	CurrentLODFactor = (LODProfiles.Num() > 1)
		? static_cast<float>(TargetLOD) / static_cast<float>(LODProfiles.Num() - 1)
		: 0.f;

	if (bFirstEvaluation)
	{
		// 首帧直接赋值，不做插值
		SmoothedBlendWeight = Target.ClothBlendWeight;
		SmoothedMaxDistScale = Target.MaxDistanceScale;
	}
	else
	{
		SmoothedBlendWeight = FMath::FInterpTo(SmoothedBlendWeight, Target.ClothBlendWeight, DeltaTime, BlendSpeed);
		SmoothedMaxDistScale = FMath::FInterpTo(SmoothedMaxDistScale, Target.MaxDistanceScale, DeltaTime, BlendSpeed);
	}

	ApplyToMesh(SmoothedBlendWeight, SmoothedMaxDistScale, Target.bEnableSimulation);
}

void UClothLODControllerComponent::ApplyToMesh(float InBlendWeight, float InMaxDistScale, bool bEnable)
{
	if (!CachedMesh)
	{
		return;
	}

	bool bShouldDisable = !bEnable || InBlendWeight < KINDA_SMALL_NUMBER;

	if (bShouldDisable != CachedMesh->bDisableClothSimulation)
	{
		CachedMesh->bDisableClothSimulation = bShouldDisable;

		if (!bShouldDisable)
		{
			ForceClothReset();
		}
	}

	if (!bShouldDisable)
	{
		CachedMesh->ClothBlendWeight = InBlendWeight;
		CachedMesh->SetClothMaxDistanceScale(InMaxDistScale);
	}

	bSimulationActive = !bShouldDisable;
}

// ==================== 传送检测与重置 ====================

void UClothLODControllerComponent::DetectTeleportation(const FVector& CurrentLocation)
{
	if (bFirstEvaluation)
	{
		return;
	}

	float MovedDistSq = FVector::DistSquared(CurrentLocation, PreviousLocation);
	float ThresholdSq = TeleportThreshold * TeleportThreshold;

	if (MovedDistSq > ThresholdSq)
	{
		ForceClothReset();
	}
}

void UClothLODControllerComponent::ForceClothReset()
{
	if (CachedMesh)
	{
		CachedMesh->ForceClothNextUpdateTeleportAndReset();
	}
}

void UClothLODControllerComponent::NotifyTeleported()
{
	ForceClothReset();
}

void UClothLODControllerComponent::ForceReevaluate()
{
	bFirstEvaluation = true;
	OnTimerEvaluate();
}
