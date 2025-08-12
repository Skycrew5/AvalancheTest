// Scientific Ways

#include "EditorMode/VoxelToolsEditorModeCommands.h"

#define LOCTEXT_NAMESPACE "FVoxelToolsEditorMode"

FVoxelToolsEditorModeCommands::FVoxelToolsEditorModeCommands()
	: TCommands<FVoxelToolsEditorModeCommands>("VoxelToolsEditorMode",
		NSLOCTEXT("VoxelToolsEditorMode", "VoxelToolsEditorModeCommands", "Voxel Tools Editor Mode"),
		NAME_None,
		FAppStyle::GetAppStyleSetName())
{

}

//~ Begin Initialize
void FVoxelToolsEditorModeCommands::RegisterCommands() // TCommands<>
{
	TArray<TSharedPtr<FUICommandInfo>>& ToolCommands = Commands.FindOrAdd(NAME_Default);

	UI_COMMAND(ManageWorldTool, "Manage World", "Manage Global World Data", EUserInterfaceActionType::Button, FInputChord());
	ToolCommands.Add(ManageWorldTool);
}
//~ End Initialize

//~ Begin Getters
TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> FVoxelToolsEditorModeCommands::GetCommands()
{
	return FVoxelToolsEditorModeCommands::Get().Commands;
}
//~ End Getters

#undef LOCTEXT_NAMESPACE
