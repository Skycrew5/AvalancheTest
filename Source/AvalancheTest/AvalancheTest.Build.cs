// Avalanche Test

using UnrealBuildTool;

public class AvalancheTest : ModuleRules
{
	public AvalancheTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[]
		{
			"AvalancheTest",
		});
		
		PublicDependencyModuleNames.AddRange(new string[] {

			"Core",
            "EngineSettings",

			"InputCore",
			"EnhancedInput",
			"UMG",

			"AIModule",
			"Navmesh",
			"NavigationSystem",

			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",

			"Networking",

			"UnrealCommons",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
        {
            "CoreUObject",
            "Engine",
        });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
