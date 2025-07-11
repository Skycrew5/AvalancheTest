// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "Procedural/ATProceduralGeneratorTask.h"

#include "ATProceduralGeneratorTask_Landscape.generated.h"

/**
 *
 */
UCLASS(ClassGroup = ("Procedural"), meta = (DisplayName = "[AT] Procedural Generator Task (Landscape)", BlueprintSpawnableComponent))
class AVALANCHETEST_API UATProceduralGeneratorTask_Landscape : public UATProceduralGeneratorTask
{
	GENERATED_BODY()

public:

	UATProceduralGeneratorTask_Landscape();
	
//~ Begin Initialize
public:
	virtual void Initialize(class AATVoxelTree* InTargetTree) override; // UATProceduralGeneratorTask
//~ End Initialize
	
//~ Begin Task
public:
	virtual void PreWork_GameThread() override; // UATProceduralGeneratorTask
	virtual void DoWorkForSelectedChunk_SubThread(const class AATVoxelChunk* InTargetChunk) override; // UATProceduralGeneratorTask
	virtual void PostWork_GameThread() override; // UATProceduralGeneratorTask
protected:
	virtual void AllocatePerChunkData(class AATVoxelChunk* InChunk) override { PerChunkData.Add(InChunk); } // UATProceduralGeneratorTask

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly)
	float HillsNoiseFrequency;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const class UATVoxelTypeData> DefaultVoxelTypeData;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const class UATVoxelTypeData> WeakVoxelTypeData;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const class UATVoxelTypeData> FoundationVoxelTypeData;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly)
	float DefaultToWeakThreshold;

	UPROPERTY(Transient)
	TObjectPtr<class UFastNoise2PerlinGenerator> PerlinGenerator;

	struct FTaskChunkData
	{
		TArray<TObjectPtr<const class UATVoxelTypeData>> TypeDataArray;
		int32 LastProcessedIndex = INDEX_NONE;
	};
	TMap<TObjectPtr<class AATVoxelChunk>, FTaskChunkData> PerChunkData;
//~ End Task
};
