// Copyright (c) 2026. Private personal project.

using UnrealBuildTool;

public class HayDayFarm : ModuleRules
{
	public HayDayFarm(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore"
		});

		// Source/HayDayFarm is the module root, so headers are included as
		// "Core/FarmLog.h", "Player/FarmCharacter.h", etc.
		PublicIncludePaths.AddRange(new string[] { ModuleDirectory });
	}
}
