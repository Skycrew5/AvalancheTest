// Scientific Ways

#include "Simulation/ATSimulationTask_StabilityRecursive.h"

#include "Simulation/ATSimulationComponent.h"
#include "Simulation/ATSimulationTask_Avalanche.h"

#include "World/ATVoxelTree.h"
#include "World/ATVoxelChunk.h"
#include "World/ATVoxelTypeData.h"

UATSimulationTask_StabilityRecursive::UATSimulationTask_StabilityRecursive()
{
	bEnablePointsCache = false;
	EarlySkipCheckDepth = 32u;
	MaxRecursionLevel = 48u;

	QueueFeedbackRadius = 2;

	DirectionsOrder1 = {
		EATAttachmentDirection::Bottom,
		EATAttachmentDirection::Right,
		EATAttachmentDirection::Left,
		EATAttachmentDirection::Front,
		EATAttachmentDirection::Back,
		EATAttachmentDirection::Top
	};
	DirectionsOrder2 = {
		EATAttachmentDirection::Top,
		EATAttachmentDirection::Back,
		EATAttachmentDirection::Front,
		EATAttachmentDirection::Left,
		EATAttachmentDirection::Right,
		EATAttachmentDirection::Bottom
	};
	DirectionsOrder3 = {
		EATAttachmentDirection::Left,
		EATAttachmentDirection::Right,
		EATAttachmentDirection::Front,
		EATAttachmentDirection::Back,
		EATAttachmentDirection::Top,
		EATAttachmentDirection::Bottom
	};
	DirectionsOrder4 = {
		EATAttachmentDirection::Right,
		EATAttachmentDirection::Left,
		EATAttachmentDirection::Back,
		EATAttachmentDirection::Front,
		EATAttachmentDirection::Top,
		EATAttachmentDirection::Bottom
	};
	DirectionsOrder5 = {
		EATAttachmentDirection::Front,
		EATAttachmentDirection::Back,
		EATAttachmentDirection::Bottom,
		EATAttachmentDirection::Top,
		EATAttachmentDirection::Left,
		EATAttachmentDirection::Right
	};
	DirectionsOrder6 = {
		EATAttachmentDirection::Back,
		EATAttachmentDirection::Front,
		EATAttachmentDirection::Top,
		EATAttachmentDirection::Bottom,
		EATAttachmentDirection::Right,
		EATAttachmentDirection::Left
	};
	DirectionsOrder_OnlyBottom = {
		EATAttachmentDirection::Bottom
	};
	UsedDirectionsOrders = {
		&DirectionsOrder1,
		&DirectionsOrder2,
		&DirectionsOrder3,
		&DirectionsOrder4,
		&DirectionsOrder5,
		&DirectionsOrder6
	};

	AvalancheValueThreshold = 0.25f;
}

//~ Begin Initialize
void UATSimulationTask_StabilityRecursive::Initialize(AATVoxelTree* InTargetTree) // UATSimulationTask
{
	Super::Initialize(InTargetTree);

	UATSimulationComponent* SimulationComponent = InTargetTree->GetSimulationComponent();
	ensureReturn(SimulationComponent);

	AvalancheSimulationTask = SimulationComponent->FindTaskInstance<UATSimulationTask_Avalanche>();
	ensureReturn(AvalancheSimulationTask);
}

void UATSimulationTask_StabilityRecursive::DeInitialize() // UATSimulationTask
{
	Super::DeInitialize();


}
//~ End Initialize
	
//~ Begin Queue
bool UATSimulationTask_StabilityRecursive::ShouldSelectQueuedPointForUpdate(const FIntVector& InPoint) const // UATSimulationTask
{
	return Super::ShouldSelectQueuedPointForUpdate(InPoint);
}

void UATSimulationTask_StabilityRecursive::QueuePointsFeedback(const FIntVector& InPoint)
{
	ensureReturn(AvalancheSimulationTask);

	TArray<FIntVector> PointsInRadius;
	TargetTree->GetAllVoxelPointsInRadius(InPoint, QueueFeedbackRadius, PointsInRadius);

	for (const FIntVector& SamplePoint : PointsInRadius)
	{
		if (!AvalancheSimulationTask->IsPointQueued(SamplePoint))
		{
			FeedbackSelectedPointsSet.Add(SamplePoint);
		}
	}
}

void UATSimulationTask_StabilityRecursive::ApplyFeedbackPoints()
{
	ensureReturn(AvalancheSimulationTask);

	for (const FIntVector& SamplePoint : FeedbackSelectedPointsSet)
	{
		if (!AvalancheSimulationTask->IsPointQueued(SamplePoint))
		{
			QueuePoint(SamplePoint, false);
		}
	}
	FeedbackSelectedPointsSet.Empty();
}
//~ End Queue

//~ Begin Task
void UATSimulationTask_StabilityRecursive::OnSelectedUpdatePointAdded(const FIntVector& InPoint) // UATSimulationTask
{
	PointsCache.Add(InPoint);

	/*ensureReturn(TargetTree);
	ensureReturn(TargetTree->HasVoxelInstanceDataAtPoint(InPoint, true));
	FVoxelInstanceData& SampleData = TargetTree->GetVoxelInstanceDataAtPoint(InPoint, false, true);
	SampleData.AvalancheValue = 0.0f;*/
}

void UATSimulationTask_StabilityRecursive::DoWork_SubThread() // UATSimulationTask
{
	UpdatedSelectedPointsStabilities.SetNum(SelectedUpdatePoints.Num());

	ParallelFor(SelectedUpdatePoints.Num(), [&](int32 InIndex)
	{
		DEV_HANDLE_ASYNC_PENDING_STOP();

		const FIntVector& SamplePoint = SelectedUpdatePoints[InIndex];

		if (TargetTree->HasVoxelInstanceDataAtPoint(SamplePoint, true))
		{
			FVoxelInstanceData& SampleData = TargetTree->GetVoxelInstanceDataAtPoint(SamplePoint, false, true);

			FRecursivePointCache NewCache = FRecursivePointCache(false, 0.0f);
			float NewStability = 0.0f;

			const FIntVector& BottomPoint = SamplePoint + FIntVector(0, 0, -1);
			FVoxelInstanceData& BottomData = TargetTree->GetVoxelInstanceDataAtPoint(BottomPoint, false, true);

			if (DoWork_SubThread_PointEarlySkip(SamplePoint))
			{
				NewStability = 1.0f;
			}
			else
			{
				for (uint8 SampleOrderIndex = 0; SampleOrderIndex < UsedDirectionsOrders.Num(); ++SampleOrderIndex)
				{
					DEV_HANDLE_ASYNC_PENDING_STOP();

					if (NewStability < 1.0f)
					{
						FRecursiveThreadData ThreadData = FRecursiveThreadData(*UsedDirectionsOrders[SampleOrderIndex]);
						float OrderStability = DoWork_SubThread_GetStabilityFromAllNeighbors(SamplePoint, ThreadData);

						if (OrderStability > NewStability)
						{
							if (bEnablePointsCache)
							{
								NewCache = FRecursivePointCache(false, OrderStability, ThreadData.ThisOrderUpdatedPoints);
							}
							NewStability = OrderStability;
						}
					}
					else
					{
						break;
					}
				}
			}
			if (bEnablePointsCache)
			{
				PointsCache[SamplePoint] = NewCache;
			}
			UpdatedSelectedPointsStabilities[InIndex] = NewStability;
		}
		//}, EParallelForFlags::ForceSingleThread);
	});
	
	bPendingPostWork = true;
}

bool UATSimulationTask_StabilityRecursive::DoWork_SubThread_PointEarlySkip(const FIntVector& InTargetPoint)
{
	if (EarlySkipCheckDepth < 1)
	{
		return false;
	}
	const FVoxelInstanceData& TargetData = TargetTree->GetVoxelInstanceDataAtPoint(InTargetPoint, false, true);
	float CompoundStability = TargetData.AvalancheValue;

	for (int32 SampleDepthZ = 0; SampleDepthZ < EarlySkipCheckDepth; ++SampleDepthZ)
	{
		FIntVector SamplePoint = InTargetPoint + FIntVector(0, 0, -SampleDepthZ);

		if (TargetTree->HasVoxelInstanceDataAtPoint(SamplePoint, true))
		{
			const FVoxelInstanceData& SampleData = TargetTree->GetVoxelInstanceDataAtPoint(SamplePoint, false, true);
			if (SampleData.IsTypeDataValid())
			{
				if (SampleData.TypeData->bHasInfiniteStability)
				{
					return true;
				}
				else
				{
					CompoundStability *= SampleData.AvalancheValue * SampleData.TypeData->GetStabilityAttachmentMulForDirection(EATAttachmentDirection::Bottom);

					if (CompoundStability <= AvalancheValueThreshold)
					{
						return false;
					}
				}
			}
		}
		else
		{
			return false;
		}
	}
	return true;
}

float UATSimulationTask_StabilityRecursive::DoWork_SubThread_GetStabilityFromAllNeighbors(const FIntVector& InTargetPoint, FRecursiveThreadData& InThreadData, EATAttachmentDirection InNeighborDirection, uint8 InCurrentRecursionLevel)
{
	DEV_HANDLE_ASYNC_PENDING_STOP(0.0f);

	++InCurrentRecursionLevel;

	if (InCurrentRecursionLevel > MaxRecursionLevel)
	{
		return 1.0f * FATVoxelUtils::BaseAttachmentStrengthMuls[InNeighborDirection];
	}
	FIntVector SamplePoint = InTargetPoint + FATVoxelUtils::IntOffsets[InNeighborDirection];

	if (FRecursivePointCache* PointCachePtr = PointsCache.Find(SamplePoint))
	{
		if (PointCachePtr->bIsThreadSafe && !PointCachePtr->Intersects(InThreadData.ThisOrderUpdatedPoints))
		{
			const FVoxelInstanceData& SampleData = TargetTree->GetVoxelInstanceDataAtPoint(SamplePoint, false, true);
			ensureReturn(SampleData.IsTypeDataValid(), 0.0f);

			return PointCachePtr->AvalancheValue * SampleData.TypeData->GetStabilityAttachmentMulForDirection(InNeighborDirection);
		}
	}
	if (InThreadData.ThisOrderUpdatedPoints.Contains(SamplePoint))
	{
		return 0.0f;
	}
	const FVoxelInstanceData& SampleData = TargetTree->GetVoxelInstanceDataAtPoint(SamplePoint, false, true);
	if (SampleData.IsTypeDataValid())
	{
		if (SampleData.TypeData->bHasInfiniteStability)
		{
			return 1.0f * SampleData.TypeData->GetStabilityAttachmentMulForDirection(InNeighborDirection);
		}
		else
		{
			InThreadData.ThisOrderUpdatedPoints.Add(SamplePoint);
			float AccumulatedStability = 0.0f;

			if (SampleData.TypeData->StraightSheddingDepth > 0 && InCurrentRecursionLevel + 1 == MaxRecursionLevel && InThreadData.DirectionsOrderPtr != &DirectionsOrder_OnlyBottom)
			{
				InThreadData.DirectionsOrderPtr = &DirectionsOrder_OnlyBottom;
				InCurrentRecursionLevel = MaxRecursionLevel - SampleData.TypeData->StraightSheddingDepth;
			}
			for (EATAttachmentDirection SampleDirection : (*InThreadData.DirectionsOrderPtr))
			{
				if (AccumulatedStability < 1.0f)
				{
					AccumulatedStability += DoWork_SubThread_GetStabilityFromAllNeighbors(SamplePoint, InThreadData, SampleDirection, InCurrentRecursionLevel);
				}
				else
				{
					break;
				}
			}
			float OutStability = FMath::Min(AccumulatedStability * SampleData.TypeData->GetStabilityAttachmentMulForDirection(InNeighborDirection), 1.0f);
			return OutStability;
		}
	}
	InThreadData.ThisOrderUpdatedPoints.Add(SamplePoint);
	return TargetTree->IsPointInsideSimulationReadyChunk(SamplePoint) ? 0.0f : 1.0f;
}

void UATSimulationTask_StabilityRecursive::PostWork_GameThread()
{
	ensureReturn(TargetTree);
	while (!TargetTree->IsThisTickUpdatesTimeBudgetExceeded() && !SelectedUpdatePoints.IsEmpty())
	{
		ensureContinue(SelectedUpdatePoints.Num() == UpdatedSelectedPointsStabilities.Num());

		FIntVector SamplePoint = SelectedUpdatePoints.Pop();
		FVoxelInstanceData& SampleData = TargetTree->GetVoxelInstanceDataAtPoint(SamplePoint, false);

		float PrevAvalancheValue = SampleData.AvalancheValue;
		SampleData.AvalancheValue = UpdatedSelectedPointsStabilities.Pop();

		AATVoxelChunk* SampleChunk = TargetTree->GetVoxelChunkAtPoint(SamplePoint);
		ensureContinue(SampleChunk);

		//SampleChunk->HandleSetVoxelStabilityAtPoint(SamplePoint, SampleData.AvalancheValue);
		SampleChunk->HandleSetVoxelInstanceDataAtPoint(SamplePoint, SampleData);

		if (SampleData.AvalancheValue <= AvalancheValueThreshold)
		{
			ensureContinue(AvalancheSimulationTask);
			AvalancheSimulationTask->QueuePoint(SamplePoint, false);

			if (PrevAvalancheValue > AvalancheValueThreshold)
			{
				QueuePointsFeedback(SamplePoint);
			}
		}
		if (bEnablePointsCache)
		{
			PointsCache[SamplePoint].bIsThreadSafe = true;
		}
	}
	if (bEnablePointsCache)
	{
		SET_MEMORY_STAT(STAT_SimulationTasks_StabilityRecursiveCache, PointsCache.GetAllocatedSize());
		for (const auto& SampleKeyValue : PointsCache)
		{
			INC_MEMORY_STAT_BY(STAT_SimulationTasks_StabilityRecursiveCache, SampleKeyValue.Value.FinalUpdatedPoints.GetAllocatedSize());
		}
	}
	if (SelectedUpdatePoints.IsEmpty())
	{
		ApplyFeedbackPoints();
		FinishPostWork_GameThread();
	}
}
//~ End Task
