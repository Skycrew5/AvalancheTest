// Scientific Ways

#include "Procedural/ATProceduralGeneratorTask.h"

#include "Procedural/ATProceduralGeneratorComponent.h"

#include "World/ATVoxelTree.h"
#include "World/ATVoxelChunk.h"

UATProceduralGeneratorTask::UATProceduralGeneratorTask()
{
	
}

//~ Begin Initialize
void UATProceduralGeneratorTask::Initialize(UATProceduralGeneratorComponent* InOwnerComponent, int32 InTaskIndex)
{
	OwnerComponent = InOwnerComponent;
	ensureReturn(OwnerComponent);

	OwnerTree = OwnerComponent->GetOwnerTree();
	ensureReturn(OwnerTree);

	check(AsyncTaskPtr == nullptr);
	AsyncTaskPtr = new FAsyncTask<FATProceduralGeneratorTask_AsyncTask>();
	AsyncTaskPtr->GetTask().TargetTask = this;

	int32 NextTaskIndex = InTaskIndex + 1;
	if (OwnerComponent->HasTaskAtIndex(NextTaskIndex))
	{
		NextTask = OwnerComponent->GetTaskAtIndex(NextTaskIndex);
	}
}

void UATProceduralGeneratorTask::DeInitialize()
{
	ensureReturn(AsyncTaskPtr);

	if (!AsyncTaskPtr->TryAbandonTask())
	{
		AsyncTaskPtr->EnsureCompletion();
	}
	delete AsyncTaskPtr;
	AsyncTaskPtr = nullptr;
}
//~ End Initialize

//~ Begin Queue
void UATProceduralGeneratorTask::QueueChunks(const TArray<AATVoxelChunk*>& InChunks)
{
	for (AATVoxelChunk* SampleChunk : InChunks)
	{
		ensureContinue(!QueuedChunksData.DataArray.Contains(SampleChunk));
		QueuedChunksData.DataArray.Add(FChunkWithSquaredDistance(SampleChunk));
	}
	QueuedChunksData.UpdateDistancesAndSort(OwnerTree, true);
}
//~ End Queue

//~ Begin Task
EATProceduralGeneratorTaskPhase UATProceduralGeneratorTask::GetCurrentTaskPhase() const
{
	ensureReturn(AsyncTaskPtr, EATProceduralGeneratorTaskPhase::Idle);

	if (AsyncTaskPtr->IsDone())
	{
		if (bPendingPostWork)
		{
			return EATProceduralGeneratorTaskPhase::PendingPostWork;
		}
		else
		{
			return EATProceduralGeneratorTaskPhase::Idle;
		}
	}
	else
	{
		return EATProceduralGeneratorTaskPhase::DoWork;
	}
}

void UATProceduralGeneratorTask::PreWork_GameThread()
{
	ensureReturn(!bPendingPostWork);

	ensureReturn(AsyncTaskPtr);
	ensureReturn(AsyncTaskPtr->IsDone());

	ensureReturn(SelectedChunks.IsEmpty());
	ensureReturn(OwnerTree);

	while (!OwnerTree->IsThisTickUpdatesTimeBudgetExceeded() && !QueuedChunksData.DataArray.IsEmpty())
	{
		TObjectPtr<class AATVoxelChunk> SampleChunk = QueuedChunksData.DataArray.Pop();

		if (ShouldSelectQueuedChunkForUpdate(SampleChunk))
		{
			SelectedChunks.Add(SampleChunk);
			AllocatePerChunkData(SampleChunk);
		}
	}
	if (SelectedChunks.IsEmpty())
	{
		return;
	}
	AsyncTaskPtr->StartBackgroundTask();
}

void UATProceduralGeneratorTask::FinishPostWorkWithChunk(AATVoxelChunk* InChunk)
{
	if (NextTask)
	{
		PushChunkForNextTask(InChunk);
	}
	else
	{
		HandleLastTaskFinishedForChunk(InChunk);
	}
	ensureReturn(SelectedChunks.Contains(InChunk));
	SelectedChunks.RemoveSingle(InChunk);
}

void UATProceduralGeneratorTask::PushChunkForNextTask(AATVoxelChunk* InChunk)
{
	ensureReturn(InChunk);
	ensureReturn(NextTask);
	NextTask->QueueChunks({ InChunk });
}

void UATProceduralGeneratorTask::HandleLastTaskFinishedForChunk(AATVoxelChunk* InChunk)
{
	ensureReturn(InChunk);
	InChunk->MarkChunkAsSimulationReady();

	RemovePerChunkData(InChunk);
}

void UATProceduralGeneratorTask::FinishPostWork_GameThread()
{
	ensureReturn(bPendingPostWork);
	bPendingPostWork = false;
}
//~ End Task
