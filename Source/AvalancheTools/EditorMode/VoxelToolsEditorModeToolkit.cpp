// Scientific Ways

#include "EditorMode/VoxelToolsEditorModeToolkit.h"

#define LOCTEXT_NAMESPACE "FVoxelToolsEditorModeToolkit"

FVoxelToolsEditorModeToolkit::FVoxelToolsEditorModeToolkit()
{

}

//~ Begin Initialize
void FVoxelToolsEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode) // FModeToolkit
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);
}
//~ End Initialize

//~ Begin Info
void FVoxelToolsEditorModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const // FModeToolkit
{
	PaletteNames.Add(NAME_Default);
}

FName FVoxelToolsEditorModeToolkit::GetToolkitFName() const // IToolkit
{
	return FName("VoxelToolsEditorMode");
}

FText FVoxelToolsEditorModeToolkit::GetBaseToolkitName() const // IToolkit
{
	return NSLOCTEXT("VoxelToolsEditorModeToolkit", "DisplayName", "VoxelToolsEditorMode Tool");
}
//~ End Info

#undef LOCTEXT_NAMESPACE
