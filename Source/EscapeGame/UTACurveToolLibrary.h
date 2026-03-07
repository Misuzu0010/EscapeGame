// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UTACurveToolLibrary.generated.h"
class UCurveFloat;
/**
 * 
 */
 //可支持的函数类型枚举
UENUM(BlueprintType)
enum class ECurveMathModel : uint8
{
    Linear       UMETA(DisplayName = "线性过渡 (y = kx + b)"),
    SmoothStep   UMETA(DisplayName = "平滑阶跃 (Piecewise Smooth)"),
    SineWave     UMETA(DisplayName = "正弦波动 (y = A*sin(fx) + c)"),
	CosineWave   UMETA(DisplayName = "余弦波动 (y = A*cos(fx) + c)"),
	Power        UMETA(DisplayName = "幂函数 (y = x^p) [缓入/缓出]"),
	DampedSpring UMETA(DisplayName = "阻尼弹簧 (e^(-decay*x) * cos(fx))"),
	PerlinNoise  UMETA(DisplayName = "柏林噪声 (1D Perlin) [随机风力/灯光]"),
	Stepped      UMETA(DisplayName = "阶梯步进 (Quantized) [抽帧/色阶]")

};
UCLASS()
class ESCAPEGAME_API UUTACurveToolLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "TATools|Curve Generator", meta = (ClampMin = "2", ClampMax = "256", UIMin = "5", UIMax = "50", Delta = "0.05"))
	static bool BakeMathToCurve
	(
		UCurveFloat* TargetCurveAsset,
		ECurveMathModel MathModel,
		float StartValue,
		float EndValue,
		float ExtraParamA, // 比如正弦波的频率，或者平滑阶跃的拐点
		float Decay, // 阻尼弹簧的衰减率
		int32 SampleCount
	);
};
