// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EscapeGame : ModuleRules
{
	public EscapeGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"EscapeGame",
			"EscapeGame/Variant_Platforming",
			"EscapeGame/Variant_Platforming/Animation",
			"EscapeGame/Variant_Combat",
			"EscapeGame/Variant_Combat/AI",
			"EscapeGame/Variant_Combat/Animation",
			"EscapeGame/Variant_Combat/Gameplay",
			"EscapeGame/Variant_Combat/Interfaces",
			"EscapeGame/Variant_Combat/UI",
			"EscapeGame/Variant_SideScrolling",
			"EscapeGame/Variant_SideScrolling/AI",
			"EscapeGame/Variant_SideScrolling/Gameplay",
			"EscapeGame/Variant_SideScrolling/Interfaces",
			"EscapeGame/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
