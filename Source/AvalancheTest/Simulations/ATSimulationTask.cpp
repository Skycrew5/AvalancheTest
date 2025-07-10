// Scientific Ways

#include "Simulations/ATSimulationTask.h"

#include "World/ATVoxelTree.h"

UATSimulationTask::UATSimulationTask()
{
	QueueNeighborsRadius = 6;
	
}

//~ Begin Initialize
void UATSimulationTask::Initialize(AATVoxelTree* InTargetTree)
{
	TargetTree = InTargetTree;

	check(SimulationAsyncTaskPtr == nullptr);
	SimulationAsyncTaskPtr = new FAsyncTask<FATSimulationTask_AsyncTask>();
	SimulationAsyncTaskPtr->GetTask().TargetTask = this;
}

void UATSimulationTask::DeInitialize()
{
	ensureReturn(SimulationAsyncTaskPtr);

	if (!SimulationAsyncTaskPtr->TryAbandonTask())
	{
		SimulationAsyncTaskPtr->EnsureCompletion();
	}
	delete SimulationAsyncTaskPtr;
	SimulationAsyncTaskPtr = nullptr;
}
//~ End Initialize

//~ Begin Queue
void UATSimulationTask::QueuePoint(const FIntVector& InPoint, const bool bInQueueNeighborsToo)
{
	QueuedPoints.Add(InPoint);
	ResetCacheAtPoint(InPoint);

	if (bInQueueNeighborsToo)
	{
		TArray<FIntVector> PointsInRadius;
		TargetTree->GetAllVoxelPointsInRadius(InPoint, QueueNeighborsRadius, PointsInRadius);

		for (const FIntVector& SamplePoint : PointsInRadius)
		{
			QueuedPoints.Add(SamplePoint);
			ResetCacheAtPoint(SamplePoint);
		}
	}
}

bool UATSimulationTask::ShouldSelectQueuedPointForUpdate(const FIntVector& InPoint) const
{
	ensureReturn(TargetTree, false);
	return TargetTree->HasVoxelInstanceDataAtPoint(InPoint, true);
}
//~ End Queue

//~ Begin Task
EATSimulationTaskPhase UATSimulationTask::GetCurrentTaskPhase() const
{
	ensureReturn(SimulationAsyncTaskPtr, EATSimulationTaskPhase::Idle);

	if (SimulationAsyncTaskPtr->IsDone())
	{
		if (bPendingPostWork)
		{
			return EATSimulationTaskPhase::PendingPostWork;
		}
		else
		{
			return EATSimulationTaskPhase::Idle;
		}
	}
	else
	{
		return EATSimulationTaskPhase::DoWork;
	}
}

void UATSimulationTask::PreWork_GameThread(int32& InOutUpdatesLeft)
{
	ensure(!bPendingPostWork);

	ensureReturn(SimulationAsyncTaskPtr);
	ensureReturn(SimulationAsyncTaskPtr->IsDone());

	ensure(SelectedUpdatePoints.IsEmpty());
	while (InOutUpdatesLeft > 0 && !QueuedPoints.IsEmpty())
	{
		InOutUpdatesLeft -= 1;
		FIntVector SamplePoint = QueuedPoints.Pop();

		if (ShouldSelectQueuedPointForUpdate(SamplePoint))
		{
			SelectedUpdatePoints.Add(SamplePoint);
			AllocateCacheAtPoint(SamplePoint);
		}
	}
	if (SelectedUpdatePoints.IsEmpty())
	{
		return;
	}
	SimulationAsyncTaskPtr->StartBackgroundTask();
}

void UATSimulationTask::FinishPostWork_GameThread()
{
	ensure(bPendingPostWork);
	bPendingPostWork = false;
}
//~ End Task
