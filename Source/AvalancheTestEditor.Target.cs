// Scientific Ways

using UnrealBuildTool;
using System.Collections.Generic;

public class AvalancheTestEditorTarget : TargetRules
{
	public AvalancheTestEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.AddRange(new string[]
		{
			"AvalancheTest",
			"AvalancheTools",
        });
	}
}
