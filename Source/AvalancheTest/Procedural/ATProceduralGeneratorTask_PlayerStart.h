// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "Procedural/ATProceduralGeneratorTask.h"

#include "ATProceduralGeneratorTask_PlayerStart.generated.h"

/**
 *
 */
UCLASS(ClassGroup = ("Procedural"), meta = (DisplayName = "[AT] Procedural Generator Task (Player Start)", BlueprintSpawnableComponent))
class AVALANCHETEST_API UATProceduralGeneratorTask_PlayerStart : public UATProceduralGeneratorTask
{
	GENERATED_BODY()

public:

	UATProceduralGeneratorTask_PlayerStart();
	
//~ Begin Initialize
public:
	virtual void Initialize(class UATProceduralGeneratorComponent* InOwnerComponent, int32 InTaskIndex) override; // UATProceduralGeneratorTask
//~ End Initialize
	
//~ Begin Task
public:
	virtual void PreWork_GameThread() override; // UATProceduralGeneratorTask
	virtual void DoWorkGlobalOnce_SubThread() override; // UATProceduralGeneratorTask
	virtual void DoWorkForSelectedChunk_SubThread(const class AATVoxelChunk* InTargetChunk) override; // UATProceduralGeneratorTask
	virtual void PostWork_GameThread() override; // UATProceduralGeneratorTask
protected:
	virtual void AllocatePerChunkData(class AATVoxelChunk* InChunk) override { PerChunkData.Add(InChunk); } // UATProceduralGeneratorTask
	virtual void RemovePerChunkData(class AATVoxelChunk* InChunk) override { PerChunkData.Remove(InChunk); } // UATProceduralGeneratorTask

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const class UATVoxelTypeData> PlatformVoxelTypeData;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly)
	FIntVector PlatformSize;

	UPROPERTY(Transient)
	TObjectPtr<class UFastNoise2PerlinGenerator> PerlinGenerator;
//~ End Task
	
//~ Begin Data
public:

	struct FPerChunkData_PlayerStart
	{
		TArray<FIntVector> PointsArray;
		TArray<TObjectPtr<const class UATVoxelTypeData>> TypeDataArray;

		int32 LastProcessedIndex = INDEX_NONE;
	};

	const FPerChunkData_PlayerStart InvalidChunkData = FPerChunkData_PlayerStart();

	const FPerChunkData_PlayerStart& GetChunkData(const class AATVoxelChunk* InChunk) const { ensureReturn(PerChunkData.Contains(InChunk), InvalidChunkData); return PerChunkData[InChunk]; }
protected:
	TMap<TObjectPtr<class AATVoxelChunk>, FPerChunkData_PlayerStart> PerChunkData;

	UPROPERTY(Transient)
	TMap<FIntVector, TObjectPtr<const class UATVoxelTypeData>> PlatformPointsMap;
//~ End Data

//~ Begin Landscape
protected:

	UPROPERTY(Transient)
	TObjectPtr<class UATProceduralGeneratorTask_Landscape> LandscapeTask;
//~ End Landscape
};
