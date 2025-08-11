// Scientific Ways

#include "EditorMode/Tools/RegenerateWorldTool.h"

#define LOCTEXT_NAMESPACE "URegenerateWorldTool"

/*
 *	ToolBuilder
 */
URegenerateWorldToolBuilder::URegenerateWorldToolBuilder()
{

}

//~ Begin Initialize
bool URegenerateWorldToolBuilder::CanBuildTool(const FToolBuilderState& InSceneState) const // UInteractiveToolBuilder
{
	return true;
}

UInteractiveTool* URegenerateWorldToolBuilder::BuildTool(const FToolBuilderState& InSceneState) const
{
	URegenerateWorldTool* NewTool = NewObject<URegenerateWorldTool>(InSceneState.ToolManager);
	NewTool->SetWorld(InSceneState.World);
	return NewTool;
}
//~ End Initialize

/*
 *	Tool
 */
URegenerateWorldToolProperties::URegenerateWorldToolProperties()
{
	bOnlySelectedVoxelTree = true;
	bAsync = true;
}

//~ Begin Procedural
void URegenerateWorldToolProperties::GenerateVoxelTreeData()
{
	/*ensureReturn(OwnerTree);
	const auto& ChunksMap = OwnerTree->GetChunksMap();

	TArray<AATVoxelChunk*> ChunksToQueue;
	ChunksMap.GenerateValueArray(ChunksToQueue);

	QueueChunksForTaskAtIndex(ChunksToQueue, 0);

	bTickInEditor = true;*/
}
//~ End Procedural

const FString URegenerateWorldTool::Identifier = TEXT("RegenerateWorldTool");

URegenerateWorldTool::URegenerateWorldTool()
{

}

//~ Begin Initialize
void URegenerateWorldTool::SetWorld(UWorld* InWorld)
{
	this->TargetWorld = InWorld;
}

void URegenerateWorldTool::Setup() // USingleClickTool
{
	Super::Setup();

	Properties = NewObject<URegenerateWorldToolProperties>(this);
	AddToolPropertySource(Properties);
}
//~ End Initialize

//~ Begin Regenerate
void URegenerateWorldTool::HandleRegenerate()
{
	
}
//~ End Regenerate

#undef LOCTEXT_NAMESPACE
