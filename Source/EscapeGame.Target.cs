// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class EscapeGameTarget : TargetRules
{
	public EscapeGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
       
		// 对应 UE 5.7 的最新编译设置版本
		DefaultBuildSettings = BuildSettingsVersion.V6;
       
		// 对应 UE 5.7 的最新头文件包含顺序规则
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
       
		ExtraModuleNames.Add("EscapeGame");
	}
}