// Copyright (c) 2026. Private personal project.

using UnrealBuildTool;
using System.Collections.Generic;

public class HayDayFarmTarget : TargetRules
{
	public HayDayFarmTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;

		ExtraModuleNames.AddRange(new string[] { "HayDayFarm" });
	}
}
