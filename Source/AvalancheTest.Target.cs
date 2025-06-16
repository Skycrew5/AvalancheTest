// Avalanche Test

using UnrealBuildTool;
using System.Collections.Generic;

public class AvalancheTestTarget : TargetRules
{
	public AvalancheTestTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.AddRange(new string[]
		{
			"AvalancheTest",
		}); ;
	}
}
