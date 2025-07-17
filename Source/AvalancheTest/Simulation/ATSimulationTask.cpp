// Scientific Ways

#include "Simulation/ATSimulationTask.h"

#include "World/ATVoxelTree.h"

UATSimulationTask::UATSimulationTask()
{
	QueueNeighborsRadius = 2;
	
}

//~ Begin Initialize
void UATSimulationTask::Initialize(AATVoxelTree* InTargetTree)
{
	TargetTree = InTargetTree;

	check(AsyncTaskPtr == nullptr);
	AsyncTaskPtr = new FAsyncTask<FATSimulationTask_AsyncTask>();
	AsyncTaskPtr->GetTask().TargetTask = this;
}

void UATSimulationTask::DeInitialize()
{
	ensureReturn(AsyncTaskPtr);

	if (!AsyncTaskPtr->TryAbandonTask())
	{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
		DEV_bPendingStop = true;
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT

		AsyncTaskPtr->EnsureCompletion();
	}
	delete AsyncTaskPtr;
	AsyncTaskPtr = nullptr;
}
//~ End Initialize

//~ Begin Queue
void UATSimulationTask::QueuePoint(const FIntVector& InPoint, const bool bInQueueNeighborsToo)
{
	QueuedPoints.Add(InPoint);
	OnQueuedPointAdded(InPoint);

	if (bInQueueNeighborsToo && QueueNeighborsRadius > 0)
	{
		TArray<FIntVector> PointsInRadius;
		TargetTree->GetAllVoxelPointsInRadius(InPoint, QueueNeighborsRadius, PointsInRadius);

		for (const FIntVector& SamplePoint : PointsInRadius)
		{
			QueuePoint(SamplePoint, false);
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
	ensureReturn(AsyncTaskPtr, EATSimulationTaskPhase::Idle);

	if (AsyncTaskPtr->IsDone())
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

void UATSimulationTask::PreWork_GameThread()
{
	ensureReturn(!bPendingPostWork);

	ensureReturn(AsyncTaskPtr);
	ensureReturn(AsyncTaskPtr->IsDone());

	ensureReturn(SelectedUpdatePoints.IsEmpty());
	ensureReturn(TargetTree);

	while (!TargetTree->IsThisTickUpdatesTimeBudgetExceeded() && !QueuedPoints.IsEmpty())
	{
		FIntVector SamplePoint = QueuedPoints.Pop();

		if (ShouldSelectQueuedPointForUpdate(SamplePoint))
		{
			SelectedUpdatePoints.Add(SamplePoint);
			OnSelectedUpdatePointAdded(SamplePoint);
		}
	}
	if (SelectedUpdatePoints.IsEmpty())
	{
		return;
	}
	AsyncTaskPtr->StartBackgroundTask();
}

void UATSimulationTask::FinishPostWork_GameThread()
{
	ensureReturn(bPendingPostWork);
	bPendingPostWork = false;
}
//~ End Task
