// Scientific Ways

using UnrealBuildTool;

public class AvalancheTools : ModuleRules
{
	public AvalancheTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(new string[]
		{
			
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
			"EditorFramework",
			"EditorStyle",
			"UnrealEd",
			"LevelEditor",
			"InteractiveToolsFramework",
			"EditorInteractiveToolsFramework",
			"PropertyEditor",
			"PlacementMode",
			"GameplayDebugger",

            "UnrealCommons",
            "AvalancheTest",
        });
		
		DynamicallyLoadedModuleNames.AddRange(new string[]
		{
			
		});
	}
}
