// Scientific Ways

#include "VoxelToolsEditorMode.h"

#include "VoxelToolsEditorModeToolkit.h"
#include "VoxelToolsEditorModeCommands.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// AddYourTool Step 1 - include the header file for your Tools here	//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "EditorMode/Tools/ManageWorldTool.h"

// step 2: register a ToolBuilder in FVoxelToolsEditorMode::Enter()

#define LOCTEXT_NAMESPACE "FVoxelToolsEditorMode"

const FEditorModeID UVoxelToolsEditorMode::EM_VoxelToolsEditorModeId = TEXT("EM_VoxelToolsEditorMode");

UVoxelToolsEditorMode::UVoxelToolsEditorMode()
{
	Info = FEditorModeInfo(UVoxelToolsEditorMode::EM_VoxelToolsEditorModeId,
		LOCTEXT("VoxelToolsEditorModeName", "Voxel Tools"),
		FSlateIcon(),
		true);
}

UVoxelToolsEditorMode::~UVoxelToolsEditorMode()
{

}

//~ Begin Initialize
void UVoxelToolsEditorMode::Enter() // UEdMode
{
	Super::Enter();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// AddYourTool Step 2 - register the ToolBuilders for your Tools here.
	// The string name you pass to the ToolManager is used to select/activate your ToolBuilder later.
	//////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////// 
	const auto& VoxelToolCommands = FVoxelToolsEditorModeCommands::Get();

	auto ManageWorldToolBuilder = NewObject<UManageWorldToolBuilder>(this);
	RegisterTool(VoxelToolCommands.ManageWorldTool, UManageWorldTool::Identifier, ManageWorldToolBuilder);

	// active tool type is not relevant here, we just set to default
	GetToolManager()->SelectActiveToolType(EToolSide::Left, UManageWorldTool::Identifier);
}

void UVoxelToolsEditorMode::CreateToolkit() // UEdMode
{
	Toolkit = MakeShareable(new FVoxelToolsEditorModeToolkit);
}

void UVoxelToolsEditorMode::ActorSelectionChangeNotify()
{
	// @todo support selection change
}
//~ End Initialize

//~ Begin Getters
TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> UVoxelToolsEditorMode::GetModeCommands() const // UEdMode
{
	return FVoxelToolsEditorModeCommands::Get().GetCommands();
}
//~ End Getters

#undef LOCTEXT_NAMESPACE
