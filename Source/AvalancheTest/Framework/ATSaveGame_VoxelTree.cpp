// Scientific Ways

#include "Framework/ATSaveGame_VoxelTree.h"

#include "World/ATVoxelTree.h"

//~ Begin Save
void UATSaveGame_VoxelTree::SaveSlot(AATVoxelTree* InTargetTree, const FString& InSaveSlot)
{
	ensureReturn(InTargetTree);

	ThisClass* SaveData = Cast<ThisClass>(UGameplayStatics::CreateSaveGameObject(ThisClass::StaticClass()));

	SaveData->ChunkClass = InTargetTree->ChunkClass;
	SaveData->TreeSizeInChunks = InTargetTree->TreeSizeInChunks;
	SaveData->ChunkSize = InTargetTree->ChunkSize;
	SaveData->VoxelSize = InTargetTree->VoxelSize;
	SaveData->ChunksUpdateMaxSquareExtent = InTargetTree->ChunksUpdateMaxSquareExtent;
	SaveData->Point_To_VoxelInstanceData_Map = InTargetTree->Point_To_VoxelInstanceData_Map;

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
	InTargetTree->ChunksUpdateMaxSquareExtent = LoadedData->ChunksUpdateMaxSquareExtent;
	InTargetTree->Point_To_VoxelInstanceData_Map = LoadedData->Point_To_VoxelInstanceData_Map;
}
//~ End Load
