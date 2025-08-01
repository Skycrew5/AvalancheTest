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
	
//~ Begin Queue
protected:
	virtual bool ShouldSelectQueuedPointForUpdate(const FIntVector& InPoint) const override; // UATSimulationTask
	void QueuePointsFeedback(const FIntVector& InPoint);
	void ApplyFeedbackPoints();

	UPROPERTY(Category = "Queue", EditAnywhere, BlueprintReadWrite)
	int32 QueueFeedbackRadius;

	UPROPERTY(Transient)
	TSet<FIntVector> FeedbackSelectedPointsSet;
//~ End Queue

//~ Begin Task
protected:
	virtual void OnSelectedUpdatePointAdded(const FIntVector& InPoint) override; // UATSimulationTask
	virtual void OnQueuedPointAdded(const FIntVector& InPoint) override { PointsCache.Remove(InPoint); } // UATSimulationTask
public:
	virtual void DoWork_SubThread() override; // UATSimulationTask
	virtual void PostWork_GameThread() override; // UATSimulationTask
protected:

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

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadWrite)
	bool bEnablePointsCache;

	UPROPERTY(Category = "Task", EditAnywhere, BlueprintReadWrite)
	uint8 MaxRecursionLevel;

	//UPROPERTY(Transient)
	TMap<FIntVector, FRecursivePointCache> PointsCache;

	UPROPERTY(Transient)
	TArray<float> UpdatedSelectedPointsStabilities;
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

//~ Begin Avalanche
protected:

	UPROPERTY(Category = "Avalanche", EditAnywhere, BlueprintReadWrite)
	float AvalancheStabilityThreshold;

	UPROPERTY(Transient)
	TObjectPtr<class UATSimulationTask_Avalanche> AvalancheSimulationTask;
//~ End Avalanche
};
