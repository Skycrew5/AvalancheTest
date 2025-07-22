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

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StrongToWeakWidth;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const class UATVoxelTypeData> StrongVoxelTypeData;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const class UATVoxelTypeData> WeakVoxelTypeData;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const class UATVoxelTypeData> BedrockVoxelTypeData;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<const class UATVoxelTypeData>> OresVoxelTypeDataArray;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	FVector2D AboveHillsOresWidthMinMax;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly)
	float HillsNoiseFrequency;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HillsHeightOffset;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly)
	float HillsHeightPow;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly)
	float CavesNoiseFrequency;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CavesNoiseThreshold;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly)
	float OresNoiseFrequency;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OresNoiseThreshold;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly)
	float BedrockNoiseFrequency;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BedrockNoiseThreshold;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	FIntPoint BedrockWidthVoxelsSidesBottom;

	UPROPERTY(Transient)
	TObjectPtr<class UFastNoise2PerlinGenerator> PerlinGenerator;

	//UPROPERTY(Transient)
	//TObjectPtr<class UFastNoise2CellularDistanceGenerator> CellularDistanceGenerator;
//~ End Task
	
//~ Begin Data
public:

	UFUNCTION(Category = "Data", BlueprintCallable)
	float GetHillsValueAtPoint2D(const FIntPoint& InPoint2D) const;

	struct FPerChunkData_Landscape
	{
		TArray<float> CavesNoiseValues3D;
		TArray<float> OresNoiseValues3D;

		TArray<TObjectPtr<const class UATVoxelTypeData>> TypeDataArray;
		int32 LastProcessedIndex = INDEX_NONE;
	};

	const FPerChunkData_Landscape InvalidChunkData = FPerChunkData_Landscape();

	const FPerChunkData_Landscape& GetChunkData(const class AATVoxelChunk* InChunk) const { ensureReturn(PerChunkData.Contains(InChunk), InvalidChunkData); return PerChunkData[InChunk]; }
protected:

	UPROPERTY(Transient)
	TArray<float> HillsValues2D;

	UPROPERTY(Transient)
	FIntPoint HillsBoundsSize2D;

	TMap<TObjectPtr<class AATVoxelChunk>, FPerChunkData_Landscape> PerChunkData;
//~ End Data
};
