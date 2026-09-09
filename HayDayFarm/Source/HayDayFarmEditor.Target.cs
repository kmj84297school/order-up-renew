// Copyright (c) 2026. Private personal project.

using UnrealBuildTool;
using System.Collections.Generic;

public class HayDayFarmEditorTarget : TargetRules
{
	public HayDayFarmEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;

		ExtraModuleNames.AddRange(new string[] { "HayDayFarm" });
	}
}
