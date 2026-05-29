// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class EscapeGameEditorTarget : TargetRules
{
	public EscapeGameEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
       
		// 编辑器 Target 也要同步升级到 V6
		DefaultBuildSettings = BuildSettingsVersion.V6;
       
		// 头文件包含顺序同步升级到 5.7
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
       
		ExtraModuleNames.Add("EscapeGame");
	}
}