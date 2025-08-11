// Scientific Ways

#include "EditorMode/VoxelToolsEditorModeCommands.h"

#define LOCTEXT_NAMESPACE "FVoxelToolsEditorMode"

FVoxelToolsEditorModeCommands::FVoxelToolsEditorModeCommands()
	: TCommands<FVoxelToolsEditorModeCommands>("VoxelToolsEditorMode",
		NSLOCTEXT("VoxelToolsEditorMode", "VoxelToolsEditorModeCommands", "Sample Tools Editor Mode"),
		NAME_None,
		FAppStyle::GetAppStyleSetName())
{

}

//~ Begin Initialize
void FVoxelToolsEditorModeCommands::RegisterCommands() // TCommands<>
{
	TArray <TSharedPtr<FUICommandInfo>>& ToolCommands = Commands.FindOrAdd(NAME_Default);

	UI_COMMAND(RegenerateWorldTool, "Regenerate World", "Procedural Regenerate World", EUserInterfaceActionType::Button, FInputChord());
	ToolCommands.Add(RegenerateWorldTool);

	UI_COMMAND(SaveWorldTool, "Save World", "Save World to Disk", EUserInterfaceActionType::Button, FInputChord());
	ToolCommands.Add(SaveWorldTool);
}
//~ End Initialize

//~ Begin Getters
TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> FVoxelToolsEditorModeCommands::GetCommands()
{
	return FVoxelToolsEditorModeCommands::Get().Commands;
}
//~ End Getters

#undef LOCTEXT_NAMESPACE
