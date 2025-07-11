// Scientific Ways

#include "Simulation/ATSimulationTask_StabilityRecursive.h"

#include "Simulation/ATSimulationComponent.h"
#include "Simulation/ATSimulationTask_HealthDrain.h"

#include "World/ATVoxelTree.h"
#include "World/ATVoxelChunk.h"
#include "World/ATVoxelTypeData.h"

UATSimulationTask_StabilityRecursive::UATSimulationTask_StabilityRecursive()
{
	MaxRecursionLevel = 48u;

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
}

//~ Begin Initialize
void UATSimulationTask_StabilityRecursive::Initialize(AATVoxelTree* InTargetTree) // UATSimulationTask
{
	Super::Initialize(InTargetTree);

	UATSimulationComponent* SimulationComponent = InTargetTree->GetSimulationComponent();
	ensureReturn(SimulationComponent);

	//HealthDrainSimulationTask = SimulationComponent->FindTaskInstance<UATSimulationTask_HealthDrain>();
	//ensureReturn(HealthDrainSimulationTask);
}

void UATSimulationTask_StabilityRecursive::DeInitialize() // UATSimulationTask
{
	Super::DeInitialize();


}
//~ End Initialize
	
//~ Begin Task
void UATSimulationTask_StabilityRecursive::DoWork_SubThread() // UATSimulationTask
{
	ParallelFor(SelectedUpdatePoints.Num(), [&](int32 InIndex)
	{
		const FIntVector& SamplePoint = SelectedUpdatePoints[InIndex];

		if (TargetTree->HasVoxelInstanceDataAtPoint(SamplePoint, true))
		{
			const FVoxelInstanceData& SampleData = TargetTree->GetVoxelInstanceDataAtPoint(SamplePoint, false, true);
			FRecursivePointCache NewCache = FRecursivePointCache(false, 0.0f);

			for (uint8 SampleOrderIndex = 0; SampleOrderIndex < UsedDirectionsOrders.Num(); ++SampleOrderIndex)
			{
				if (NewCache.Stability < 1.0f)
				{
					FRecursiveThreadData ThreadData = FRecursiveThreadData(*UsedDirectionsOrders[SampleOrderIndex]);
					float OrderStability = DoWork_SubThread_GetStabilityFromAllNeighbors(SamplePoint, ThreadData);
					if (OrderStability > NewCache.Stability)
					{
						NewCache = FRecursivePointCache(false, OrderStability, ThreadData.ThisOrderUpdatedPoints);
					}
				}
				else
				{
					break;
				}
			}
			PointsCache[SamplePoint] = NewCache;
		}
		//}, EParallelForFlags::ForceSingleThread);
	});
	bPendingPostWork = true;
}

float UATSimulationTask_StabilityRecursive::DoWork_SubThread_GetStabilityFromAllNeighbors(const FIntVector& InTargetPoint, FRecursiveThreadData& InThreadData, EATAttachmentDirection InNeighborDirection, uint8 InCurrentRecursionLevel)
{
	++InCurrentRecursionLevel;

	if (InCurrentRecursionLevel > MaxRecursionLevel)
	{
		return 1.0f * EATAttachmentDirection_Utils::BaseAttachmentStrengthMuls[InNeighborDirection];
	}
	FIntVector SamplePoint = InTargetPoint + EATAttachmentDirection_Utils::IntOffsets[InNeighborDirection];

	if (FRecursivePointCache* PointCachePtr = PointsCache.Find(SamplePoint))
	{
		if (PointCachePtr->bIsThreadSafe && !PointCachePtr->Intersects(InThreadData.ThisOrderUpdatedPoints))
		{
			const FVoxelInstanceData& SampleData = TargetTree->GetVoxelInstanceDataAtPoint(SamplePoint, false, true);
			ensureReturn(SampleData.IsTypeDataValid(), 0.0f);

			return PointCachePtr->Stability * SampleData.TypeData->GetStabilityAttachmentMulForDirection(InNeighborDirection);
		}
	}
	if (InThreadData.ThisOrderUpdatedPoints.Contains(SamplePoint))
	{
		return 0.0f;
	}
	const FVoxelInstanceData& SampleData = TargetTree->GetVoxelInstanceDataAtPoint(SamplePoint, false, true);
	if (SampleData.IsTypeDataValid())
	{
		if (SampleData.TypeData->bIsFoundation)
		{
			return 1.0f * SampleData.TypeData->GetStabilityAttachmentMulForDirection(InNeighborDirection);
		}
		else
		{
			InThreadData.ThisOrderUpdatedPoints.Add(SamplePoint);
			float AccumulatedStability = 0.0f;

			if (InCurrentRecursionLevel + 1 == MaxRecursionLevel && InThreadData.DirectionsOrderPtr != &DirectionsOrder_OnlyBottom)
			{
				InThreadData.DirectionsOrderPtr = &DirectionsOrder_OnlyBottom;
				InCurrentRecursionLevel = 0u;
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
		FIntVector SamplePoint = SelectedUpdatePoints.Pop();
		FVoxelInstanceData& SampleData = TargetTree->GetVoxelInstanceDataAtPoint(SamplePoint, false);

		ensureContinue(PointsCache.Contains(SamplePoint));
		SampleData.Stability = PointsCache[SamplePoint].Stability;

		if (SampleData.Stability > 0.25f)
		{
			AATVoxelChunk* SampleChunk = TargetTree->GetVoxelChunkAtPoint(SamplePoint);
			ensureContinue(SampleChunk);

			//SampleChunk->HandleSetVoxelStabilityAtPoint(SamplePoint, SampleData.Stability);
			SampleChunk->HandleSetVoxelInstanceDataAtPoint(SamplePoint, SampleData);
		}
		else
		{
			TargetTree->BreakVoxelAtPoint(SamplePoint, true, true); // Will update Chunk too
		}
		//ensureContinue(HealthDrainSimulationTask);
		//HealthDrainSimulationTask->QueuePoint(SamplePoint);

		PointsCache[SamplePoint].bIsThreadSafe = true;
	}
	if (SelectedUpdatePoints.IsEmpty())
	{
		FinishPostWork_GameThread();
	}
}
//~ End Task
