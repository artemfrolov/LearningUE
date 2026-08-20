// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LearningUE : ModuleRules
{
	public LearningUE(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"LearningUE",
			"LearningUE/Variant_Platforming",
			"LearningUE/Variant_Platforming/Animation",
			"LearningUE/Variant_Combat",
			"LearningUE/Variant_Combat/AI",
			"LearningUE/Variant_Combat/Animation",
			"LearningUE/Variant_Combat/Gameplay",
			"LearningUE/Variant_Combat/Interfaces",
			"LearningUE/Variant_Combat/UI",
			"LearningUE/Variant_SideScrolling",
			"LearningUE/Variant_SideScrolling/AI",
			"LearningUE/Variant_SideScrolling/Gameplay",
			"LearningUE/Variant_SideScrolling/Interfaces",
			"LearningUE/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
