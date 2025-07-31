// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "Simulation/ATSimulationTask.h"

#include "World/ATTypes_World.h"

#include "ATSimulationTask_StabilityWaves.generated.h"

/**
 *
 */
UCLASS(ClassGroup = ("Simulations"), meta = (DisplayName = "[AT] Simulation Task (StabilityWaves)", BlueprintSpawnableComponent))
class AVALANCHETEST_API UATSimulationTask_StabilityWaves : public UATSimulationTask
{
	GENERATED_BODY()

public:

	UATSimulationTask_StabilityWaves();
	
//~ Begin Initialize
public:
	virtual void Initialize(class AATVoxelTree* InTargetTree) override; // UATSimulationTask
	virtual void DeInitialize() override; // UATSimulationTask
//~ End Initialize
	
//~ Begin Task
protected:
	//virtual void OnSelectedUpdatePointAdded(const FIntVector& InPoint) override { PointsCache.Add(InPoint); } // UATSimulationTask
	//virtual void OnQueuedPointAdded(const FIntVector& InPoint) override { PointsCache.Remove(InPoint); } // UATSimulationTask
public:
	virtual void DoWork_SubThread() override; // UATSimulationTask
	virtual void PostWork_GameThread() override; // UATSimulationTask
protected:

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadWrite)
	uint8 EmptyColorValue;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadWrite)
	uint8 DirtyColorValue;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadWrite)
	uint8 SupportedColorValue;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadWrite)
	uint8 UpdatedNeighborColorValue;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadWrite)
	float BelowStabilityConveyMul;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadWrite)
	float SideStabilityConveyMul;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadWrite)
	int32 WaveIterationsHardLimit;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadWrite)
	int32 BoundingBoxAdditionalExtent;

	UPROPERTY(Transient)
	FIntVector BoundingBoxMin;

	UPROPERTY(Transient)
	FIntVector BoundingBoxMax;

	UPROPERTY(Transient)
	FIntVector BoundingBoxSize;

	struct FBoundingBoxData
	{
		uint8 ColorValue;
		float StabilityValue;
		float MassValue;
		float AvalancheValue;
	};

	//UPROPERTY(Transient)
	TMap<FIntVector, FBoundingBoxData> BoundingBoxDataMap;
//~ End Task

//~ Begin Avalanche
protected:

	UPROPERTY(Category = "Avalanche", EditAnywhere, BlueprintReadWrite)
	float AvalancheStabilityThreshold;

	UPROPERTY(Category = "Avalanche", EditAnywhere, BlueprintReadWrite)
	float AvalancheMassThreshold;

	UPROPERTY(Transient)
	TObjectPtr<class UATSimulationTask_Avalanche> AvalancheSimulationTask;
//~ End Avalanche

//~ Begin Utils
protected:
	bool IsPointInBoundingBox(const FIntVector& InPoint) const;

	void InitDataForPoints(const TArray<FIntVector>& InPoints);
	void GetDirtyNeighbors(const FIntVector& InPoint, TArray<FIntVector>& OutDirtyNeighbors) const;
	float GetAttachmentNeighbors(const FIntVector& InPoint, TArray<FIntVector>& OutAttachmentNeighbors) const;
//~ End Utils
};
