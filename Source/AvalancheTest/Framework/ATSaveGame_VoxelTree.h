// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "World/ATTypes_World.h"

#include "ATSaveGame_VoxelTree.generated.h"

USTRUCT(BlueprintType)
struct FATVoxelTypeSaveData
{
	GENERATED_BODY()

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadWrite)
	TArray<FIntVector> Points;
};

/**
 * 
 */
UCLASS()
class AVALANCHETEST_API UATSaveGame_VoxelTree : public USaveGame
{
	GENERATED_BODY()
	
//~ Begin Save
public:

	UFUNCTION(Category = "Save", BlueprintCallable)
	static void SaveSlot(class AATVoxelTree* InTargetTree, const FString& InSaveSlot);
//~ End Save

//~ Begin Load
public:

	UFUNCTION(Category = "Load", BlueprintCallable)
	static void LoadSlot(class AATVoxelTree* InTargetTree, const FString& InSaveSlot);
//~ End Load

//~ Begin Data
	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AATVoxelChunk> ChunkClass;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadWrite)
	FIntVector TreeSizeInChunks;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadWrite)
	int32 ChunkSize;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadWrite)
	float VoxelSize;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadWrite)
	int32 ChunksUpdateMaxSquareExtent;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadWrite)
	TMap<TObjectPtr<const class UATVoxelTypeData>, FATVoxelTypeSaveData> VoxelTypeData_To_SaveData_Map;
//~ End Data
};
