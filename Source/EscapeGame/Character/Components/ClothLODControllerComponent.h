// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClothLODControllerComponent.generated.h"


class USkeletalMeshComponent;

/**
 * 单个布料LOD级别的参数配置
 * 按 MaxDistance 从近到远排列，定义该距离范围内的布料模拟质量
 */
USTRUCT(BlueprintType)
struct FClothLODProfile
{
	GENERATED_BODY()

	// 该LOD级别生效的最大距离(cm)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD", meta = (ClampMin = "0.0"))
	float MaxDistance = 0.f;

	// 布料与蒙皮姿态的混合权重 (0=纯蒙皮, 1=完全布料模拟)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ClothBlendWeight = 1.0f;

	// 布料约束MaxDistance缩放 (值越小布料运动幅度越小，越贴合蒙皮)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxDistanceScale = 1.0f;

	// 是否启用布料模拟
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD")
	bool bEnableSimulation = true;
};

/**
 * 布料性能与权重动态控制器
 *
 * 基于相机距离与骨骼网格体LOD融合，动态调整布料模拟质量。
 * 不使用Tick，通过FTimerHandle定时器驱动LOD评估。
 *
 * 典型用法：挂载在带有布料模拟（头发/披风/裙摆等）的角色上，
 * 远距离自动降低或关闭布料模拟以节省性能。
 *
 * 输出的 LODFactor 可在 AnimBP 中用于同步缩放 Kawaii Physics 参数。
 */
UCLASS(ClassGroup = (Physics), meta = (BlueprintSpawnableComponent))
class ESCAPEGAME_API UClothLODControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UClothLODControllerComponent();

	// 当前LOD级别 (0=最高质量)
	UFUNCTION(BlueprintCallable, Category = "Cloth LOD")
	int32 GetCurrentLODLevel() const { return CurrentLODIndex; }

	// LOD因子 (0=最高质量, 1=最低质量)，可用于AnimBP缩放Kawaii Physics
	UFUNCTION(BlueprintCallable, Category = "Cloth LOD")
	float GetLODFactor() const { return CurrentLODFactor; }

	// 当前插值后的布料混合权重
	UFUNCTION(BlueprintCallable, Category = "Cloth LOD")
	float GetCurrentBlendWeight() const { return SmoothedBlendWeight; }

	// 手动通知传送，调用后立即重置布料状态
	UFUNCTION(BlueprintCallable, Category = "Cloth LOD")
	void NotifyTeleported();

	// 强制立即重新评估LOD（场景切换等特殊场景使用）
	UFUNCTION(BlueprintCallable, Category = "Cloth LOD")
	void ForceReevaluate();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ---------- 可配置参数 ----------

	// LOD级别配置表，将在BeginPlay时按MaxDistance自动排序
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cloth LOD|Profiles")
	TArray<FClothLODProfile> LODProfiles;

	// LOD评估间隔(秒)，多个实例会自动随机错开以避免同帧峰值
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cloth LOD|Performance", meta = (ClampMin = "0.03", ClampMax = "2.0"))
	float EvaluationInterval = 0.15f;

	// 布料参数平滑过渡速度
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cloth LOD|Performance", meta = (ClampMin = "0.5", ClampMax = "20.0"))
	float BlendSpeed = 4.0f;

	// 是否融合骨骼网格体LOD（取距离LOD与网格LOD中更保守的值）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cloth LOD|Fusion")
	bool bFuseMeshLOD = true;

	// 网格LOD→布料LOD映射偏移（=1表示网格LOD0映射到布料LOD1，更早降级）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cloth LOD|Fusion", meta = (EditCondition = "bFuseMeshLOD", ClampMin = "-2", ClampMax = "3"))
	int32 MeshLODOffset = 0;

	// 不可见时自动关闭布料模拟
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cloth LOD|Optimization")
	bool bDisableWhenHidden = true;

	// 帧间移动距离超过此阈值(cm)视为传送，将重置布料状态
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cloth LOD|Optimization", meta = (ClampMin = "100.0"))
	float TeleportThreshold = 500.0f;

	// 目标骨骼网格体组件（为空时自动查找Owner上的第一个SkeletalMeshComponent）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cloth LOD|Target")
	TObjectPtr<USkeletalMeshComponent> TargetMesh;

private:
	void OnTimerEvaluate();
	int32 ResolveDistanceLOD(float DistanceSq) const;
	int32 FuseWithMeshLOD(int32 DistanceLOD) const;
	void SmoothAndApply(int32 TargetLOD, float DeltaTime);
	void ApplyToMesh(float InBlendWeight, float InMaxDistScale, bool bEnable);
	void DetectTeleportation(const FVector& CurrentLocation);
	void ForceClothReset();

	FTimerHandle EvalTimerHandle;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> CachedMesh;

	int32 CurrentLODIndex;
	float CurrentLODFactor;
	float SmoothedBlendWeight;
	float SmoothedMaxDistScale;
	bool bSimulationActive;

	FVector PreviousLocation;
	bool bFirstEvaluation;

	TArray<float> CachedDistanceSqThresholds;
};
