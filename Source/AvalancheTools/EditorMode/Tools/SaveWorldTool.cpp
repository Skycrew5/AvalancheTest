// Scientific Ways

#include "EditorMode/Tools/SaveWorldTool.h"

#define LOCTEXT_NAMESPACE "USaveWorldTool"

/*
 *	ToolBuilder
 */
USaveWorldToolBuilder::USaveWorldToolBuilder()
{

}

//~ Begin Initialize
bool USaveWorldToolBuilder::CanBuildTool(const FToolBuilderState& InSceneState) const // UInteractiveToolBuilder
{
	return true;
}

UInteractiveTool* USaveWorldToolBuilder::BuildTool(const FToolBuilderState& InSceneState) const
{
	USaveWorldTool* NewTool = NewObject<USaveWorldTool>(InSceneState.ToolManager);
	NewTool->SetWorld(InSceneState.World);
	return NewTool;
}
//~ End Initialize

/*
 *	Tool
 */
USaveWorldToolProperties::USaveWorldToolProperties()
{
	bOnlySelectedVoxelTree = true;
	bAsync = true;
}

const FString USaveWorldTool::Identifier = TEXT("SaveWorldTool");

USaveWorldTool::USaveWorldTool()
{

}

//~ Begin Initialize
void USaveWorldTool::SetWorld(UWorld* InWorld)
{
	this->TargetWorld = InWorld;
}

void USaveWorldTool::Setup() // USingleClickTool
{
	Super::Setup();

	Properties = NewObject<USaveWorldToolProperties>(this);
	AddToolPropertySource(Properties);
}
//~ End Initialize

//~ Begin Regenerate
void USaveWorldTool::HandleRegenerate()
{

}
//~ End Regenerate

#undef LOCTEXT_NAMESPACE
