// Scientific Ways

using UnrealBuildTool;

public class AvalancheTools : ModuleRules
{
	public AvalancheTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(new string[]
        {
            "AvalancheTools",
        });
		
		PrivateIncludePaths.AddRange(new string[]
		{
			
		});
		
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",

			"Slate",
			"SlateCore",
			"InputCore",

			"EditorStyle",
			"EditorSubsystem",
            "EditorFramework",

            "UnrealEd",
			"LevelEditor",
            "PropertyEditor",
            "PlacementMode",

            "InteractiveToolsFramework",
			"EditorInteractiveToolsFramework",
			"GameplayDebugger",

            "UnrealCommons",
            "AvalancheTest",
        });
		
		DynamicallyLoadedModuleNames.AddRange(new string[]
		{
			
		});
	}
}
