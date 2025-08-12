// Scientific Ways

#include "EditorMode/Tools/ManageWorldTool.h"

#include "World/ATVoxelTree.h"

#define LOCTEXT_NAMESPACE "UManageWorldTool"

/**
 *	Tool Builder
 */
UManageWorldToolBuilder::UManageWorldToolBuilder()
{

}

//~ Begin Initialize
bool UManageWorldToolBuilder::CanBuildTool(const FToolBuilderState& InSceneState) const // UInteractiveToolBuilder
{
	return true;
}

UInteractiveTool* UManageWorldToolBuilder::BuildTool(const FToolBuilderState& InSceneState) const
{
	UManageWorldTool* NewTool = NewObject<UManageWorldTool>(InSceneState.ToolManager);
	NewTool->SetWorld(InSceneState.World);
	return NewTool;
}
//~ End Initialize

/**
 *	Tool Properties
 */
UManageWorldToolProperties::UManageWorldToolProperties()
{
	SaveSlot = TEXT("DefaultSlot");

	bOnlySelectedVoxelTree = false;
	bAsync = false;
	TreeSeed = 1337;
}

//~ Begin Generate
void UManageWorldToolProperties::GenerateVoxelTreeData()
{
	UManageWorldTool* OuterTool = Cast<UManageWorldTool>(GetOuter());
	ensureReturn(OuterTool);

	OuterTool->HandleGenerate();
}
//~ End Generate

//~ Begin Save / Load
void UManageWorldToolProperties::SaveVoxelTreeData()
{
	UManageWorldTool* OuterTool = Cast<UManageWorldTool>(GetOuter());
	ensureReturn(OuterTool);

	OuterTool->HandleSave();
}

void UManageWorldToolProperties::LoadVoxelTreeData()
{
	UManageWorldTool* OuterTool = Cast<UManageWorldTool>(GetOuter());
	ensureReturn(OuterTool);

	OuterTool->HandleLoad();
}
//~ End Save / Load

/**
 *	Tool
 */
const FString UManageWorldTool::Identifier = TEXT("ManageWorldTool");

UManageWorldTool::UManageWorldTool()
{
	
}

//~ Begin Initialize
void UManageWorldTool::SetWorld(UWorld* InWorld)
{
	this->TargetWorld = InWorld;
}

void UManageWorldTool::Setup() // UInteractiveTool
{
	Super::Setup();

	Properties = NewObject<UManageWorldToolProperties>(this);
	AddToolPropertySource(Properties);
}
//~ End Initialize

//~ Begin Generate
void UManageWorldTool::HandleGenerate()
{
	ensureReturn(TargetWorld);
	ensureReturn(Properties);

	TArray<AATVoxelTree*> TargetVoxelTrees;
	for (TActorIterator<AATVoxelTree> It(TargetWorld); It; ++It)
	{
		if (Properties->bOnlySelectedVoxelTree && !It->IsActorOrSelectionParentSelected())
		{
			continue;
		}
		It->HandleGenerate(Properties->bAsync, Properties->TreeSeed);
	}
}
//~ End Generate

//~ Begin Save / Load
void UManageWorldTool::HandleSave()
{
	ensureReturn(TargetWorld);
	ensureReturn(Properties);

	TArray<AATVoxelTree*> TargetVoxelTrees;
	for (TActorIterator<AATVoxelTree> It(TargetWorld); It; ++It)
	{
		if (Properties->bOnlySelectedVoxelTree && !It->IsActorOrSelectionParentSelected())
		{
			continue;
		}
		It->SaveData(Properties->SaveSlot);
	}
}

void UManageWorldTool::HandleLoad()
{
	ensureReturn(TargetWorld);
	ensureReturn(Properties);

	//UATSaveGame_VoxelTree::LoadSlot(OwnerTree, VoxelTreeDataSaveSlot);

	TArray<AATVoxelTree*> TargetVoxelTrees;
	for (TActorIterator<AATVoxelTree> It(TargetWorld); It; ++It)
	{
		if (Properties->bOnlySelectedVoxelTree && !It->IsActorOrSelectionParentSelected())
		{
			continue;
		}
		It->LoadData(Properties->SaveSlot);
	}
}
//~ End Save / Load

#undef LOCTEXT_NAMESPACE
