// Scientific Ways

#include "Framework/ATSaveGame_VoxelTree.h"

#include "World/ATVoxelTree.h"

//~ Begin Save
void UATSaveGame_VoxelTree::SaveSlot(AATVoxelTree* InTargetTree, const FString& InSaveSlot)
{
	ensureReturn(InTargetTree);
	ensureReturn(InTargetTree->Queued_Point_To_VoxelInstanceData_Map.IsEmpty()); // Ensure no pending changes

	ThisClass* SaveData = Cast<ThisClass>(UGameplayStatics::CreateSaveGameObject(ThisClass::StaticClass()));

	SaveData->ChunkClass = InTargetTree->ChunkClass;
	SaveData->TreeSizeInChunks = InTargetTree->TreeSizeInChunks;
	SaveData->ChunkSize = InTargetTree->ChunkSize;
	SaveData->VoxelSize = InTargetTree->VoxelSize;
	//SaveData->ChunksUpdateMaxSquareExtent = InTargetTree->ChunksUpdateMaxSquareExtent;

	for (const auto& Pair : InTargetTree->Point_To_VoxelInstanceData_Map)
	{
		const UATVoxelTypeData* TypeData = Pair.Value.TypeData;

		if (!SaveData->VoxelTypeData_To_SaveData_Map.Contains(TypeData))
		{
			SaveData->VoxelTypeData_To_SaveData_Map.Add(TypeData);
		}
		SaveData->VoxelTypeData_To_SaveData_Map[TypeData].Points.Add(Pair.Key);
	}
	UGameplayStatics::SaveGameToSlot(SaveData, InSaveSlot, 0);
}
//~ End Save

//~ Begin Load
void UATSaveGame_VoxelTree::LoadSlot(class AATVoxelTree* InTargetTree, const FString& InSaveSlot)
{
	ThisClass* LoadedData = Cast<ThisClass>(UGameplayStatics::LoadGameFromSlot(InSaveSlot, 0));
	ensureReturn(LoadedData);

	ensureReturn(InTargetTree);
	InTargetTree->ChunkClass = LoadedData->ChunkClass;
	InTargetTree->TreeSizeInChunks = LoadedData->TreeSizeInChunks;
	InTargetTree->ChunkSize = LoadedData->ChunkSize;
	InTargetTree->VoxelSize = LoadedData->VoxelSize;
	//InTargetTree->ChunksUpdateMaxSquareExtent = LoadedData->ChunksUpdateMaxSquareExtent;

	if (IS_EDITOR_WORLD(InTargetTree->))
	{
		InTargetTree->InitAllChunks();
		InTargetTree->MarkAllChunksAsSimulationReady();
	}
	else
	{
		InTargetTree->Reset();
	}
	ensureReturn(InTargetTree->Point_To_VoxelInstanceData_Map.IsEmpty());
	for (const auto& Pair : LoadedData->VoxelTypeData_To_SaveData_Map)
	{
		for (const FIntVector& SamplePoint : Pair.Value.Points)
		{
			InTargetTree->SetVoxelAtPoint(SamplePoint, Pair.Key);
		}
	}
}
//~ End Load
