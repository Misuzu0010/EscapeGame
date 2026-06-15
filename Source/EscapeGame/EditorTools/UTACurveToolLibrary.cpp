// Fill out your copyright notice in the Description page of Project Settings.


#include "EditorTools/UTACurveToolLibrary.h"
#include"Curves/CurveFloat.h"

bool UUTACurveToolLibrary::BakeMathToCurve(UCurveFloat* TargetCurveAsset, ECurveMathModel MathModel, float StartValue, float EndValue, float ExtraParamA,float Decay, int32 SampleCount)
{
	SampleCount = FMath::Clamp(SampleCount, 2, 256);

	if (!TargetCurveAsset) 
	{
		UE_LOG(LogTemp, Warning, TEXT("非法的曲线类型，请重新选择"));
		return false;

	}
#if WITH_EDITOR
	//记录编辑器事务（Transaction），允许用户按下 Ctrl+Z 撤销曲线的修改！
	TargetCurveAsset->Modify();
#endif
	//可编辑的曲线
	FRichCurve& RichCurve = TargetCurveAsset->FloatCurve;
	RichCurve.Reset(); // 清空原有数据

	for (int32 i = 0; i <= SampleCount; ++i) 
	{
		float X = (float)i / (float)SampleCount;
		float Y = 0.0f;
		
		switch (MathModel)
		{
		case ECurveMathModel::Linear:
		{
			Y = FMath::Lerp(StartValue, EndValue, X);
			break;
		}
		case ECurveMathModel::SmoothStep: 
		{
			// 1. 计算出基础的 0~1 线性进度
			float BaseAlpha = FMath::Clamp((X - ExtraParamA) / FMath::Max(1.0f - ExtraParamA, 0.00001f), 0.0f, 1.0f);

			// 【核心修复】2. 把 0~1 的直男线性进度，揉捏成 0~1 的 S型平滑进度
			// SmoothStep 的前两个参数永远填 0.0f 和 1.0f，因为我们在平滑 Alpha！
			float SmoothedAlpha = FMath::SmoothStep(0.0f, 1.0f, BaseAlpha);

			// 3. 用加工好的完美 Alpha，去决定最终 Y 值。这样无论 Start 和 End 谁大谁小，Lerp 都能完美处理！
			Y = FMath::Lerp(StartValue, EndValue, SmoothedAlpha);
			break;
		}
		case ECurveMathModel::SineWave:
		{
			Y = StartValue + (EndValue - StartValue) * 0.5f * (1.0f + FMath::Sin(2.0f * PI * ExtraParamA * X));
			break;
		}
		case ECurveMathModel::CosineWave:
		{
			Y = StartValue + (EndValue - StartValue) * 0.5f * (1.0f + FMath::Cos(2.0f * PI * ExtraParamA * X));
			break;
		}
		case ECurveMathModel::Power:
		{
			float M_Alpha = FMath::Pow(X, FMath::Max(0.0001f, ExtraParamA));
			Y = FMath::Lerp(StartValue, EndValue, M_Alpha);
			break;
		}
		case ECurveMathModel::DampedSpring:
		{
			float SpringAlpha = 1.0f - (FMath::Exp(-Decay * X) * FMath::Cos(2.0f * PI * ExtraParamA * X));

			// 然后再用标准的 Lerp 展开式
			Y = StartValue + (EndValue - StartValue) * SpringAlpha;
			break;
		}
		case ECurveMathModel::PerlinNoise:
		{
			float M_Noise = FMath::PerlinNoise1D(X * ExtraParamA); // 返回 -1 到 1
			// 归一化
			float M_NormalizedNoise = M_Noise * 0.5f + 0.5f;
			Y = FMath::Lerp(StartValue, EndValue, M_NormalizedNoise);
			break;
		}
		case ECurveMathModel::Stepped:
		{	// ExtraParamA 作为台阶数量（比如输入 5.0，就把 0~1 切成 5 份）
			int32 Steps = FMath::Max(1, FMath::RoundToInt(ExtraParamA));
			// 利用向下取整实现阶梯化
			float SteppedAlpha = FMath::FloorToFloat(X * Steps) / (float)Steps;
			Y = FMath::Lerp(StartValue, EndValue, SteppedAlpha);
			break;
		}
		default:
			break;
		}		
		FKeyHandle KeyHandle = RichCurve.UpdateOrAddKey(X, Y);
		RichCurve.SetKeyInterpMode(KeyHandle, RCIM_Linear);
	}
	TargetCurveAsset->MarkPackageDirty(); // 标记资源已修改

	return true;
}
