// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "Simulation/ATSimulationTask.h"

#include "World/ATTypes_World.h"

#include "ATSimulationTask_StabilityRecursive.generated.h"

/**
 *
 */
UCLASS(ClassGroup = ("Simulations"), meta = (DisplayName = "[AT] Simulation Task (StabilityRecursive)", BlueprintSpawnableComponent))
class AVALANCHETEST_API UATSimulationTask_StabilityRecursive : public UATSimulationTask
{
	GENERATED_BODY()

public:

	UATSimulationTask_StabilityRecursive();
	
//~ Begin Initialize
public:
	virtual void Initialize(class AATVoxelTree* InTargetTree) override; // UATSimulationTask
	virtual void DeInitialize() override; // UATSimulationTask
//~ End Initialize
	
//~ Begin Task
public:
	virtual void DoWork_SubThread() override; // UATSimulationTask
	virtual void PostWork_GameThread() override; // UATSimulationTask
protected:
	virtual void AllocateCacheAtPoint(const FIntVector& InPoint) override { PointsCache.Add(InPoint); } // UATSimulationTask
	virtual void ResetCacheAtPoint(const FIntVector& InPoint) override { PointsCache.Remove(InPoint); } // UATSimulationTask

	struct FRecursiveThreadData
	{
		const TArray<EATAttachmentDirection>* DirectionsOrderPtr = nullptr;
		TSet<FIntVector> ThisOrderUpdatedPoints = TSet<FIntVector>();

		FRecursiveThreadData(const TArray<EATAttachmentDirection>& InOrder)
			: DirectionsOrderPtr(&InOrder), ThisOrderUpdatedPoints({}) {
		}
	};

	struct FRecursivePointCache
	{
		bool bIsThreadSafe = false;
		float Stability = 0.0f;
		TSet<FIntVector> FinalUpdatedPoints = TSet<FIntVector>();

		bool Intersects(const TSet<FIntVector>& InOther) const
		{
			const TSet<FIntVector>& SmallerSet = (FinalUpdatedPoints.Num() < InOther.Num()) ? FinalUpdatedPoints : InOther;
			const TSet<FIntVector>& LargerSet = (FinalUpdatedPoints.Num() < InOther.Num()) ? InOther : FinalUpdatedPoints;

			for (const FIntVector& SampleItem : SmallerSet)
			{
				if (LargerSet.Contains(SampleItem))
				{
					return true;
				}
			}
			return false;
		}
	};

	float DoWork_SubThread_GetStabilityFromAllNeighbors(const FIntVector& InTargetPoint, FRecursiveThreadData& InThreadData, EATAttachmentDirection InNeighborDirection = EATAttachmentDirection::None, uint8 InCurrentRecursionLevel = 0u);

	//UPROPERTY()
	TMap<FIntVector, FRecursivePointCache> PointsCache;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadWrite)
	uint8 MaxRecursionLevel;
//~ End Task

//~ Begin Directions
protected:

	UPROPERTY(Category = "Directions", EditAnywhere, BlueprintReadWrite)
	TArray<EATAttachmentDirection> DirectionsOrder1;

	UPROPERTY(Category = "Directions", EditAnywhere, BlueprintReadWrite)
	TArray<EATAttachmentDirection> DirectionsOrder2;

	UPROPERTY(Category = "Directions", EditAnywhere, BlueprintReadWrite)
	TArray<EATAttachmentDirection> DirectionsOrder3;

	UPROPERTY(Category = "Directions", EditAnywhere, BlueprintReadWrite)
	TArray<EATAttachmentDirection> DirectionsOrder4;

	UPROPERTY(Category = "Directions", EditAnywhere, BlueprintReadWrite)
	TArray<EATAttachmentDirection> DirectionsOrder5;

	UPROPERTY(Category = "Directions", EditAnywhere, BlueprintReadWrite)
	TArray<EATAttachmentDirection> DirectionsOrder6;

	UPROPERTY(Category = "Directions", EditAnywhere, BlueprintReadWrite)
	TArray<EATAttachmentDirection> DirectionsOrder_OnlyBottom;

	TArray<TArray<EATAttachmentDirection>*> UsedDirectionsOrders;
//~ End Directions
	
//~ Begin Next
protected:

	//UPROPERTY(Category = "Next", EditAnywhere, BlueprintReadOnly)
	//TObjectPtr<class UATSimulationTask_HealthDrain> HealthDrainSimulationTask;
//~ End Next
};
