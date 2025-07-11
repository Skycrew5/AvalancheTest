// Scientific Ways

#include "Procedural/ATProceduralGeneratorTask.h"

#include "World/ATVoxelTree.h"

UATProceduralGeneratorTask::UATProceduralGeneratorTask()
{
	
}

//~ Begin Initialize
void UATProceduralGeneratorTask::Initialize(AATVoxelTree* InTargetTree)
{
	ensureReturn(InTargetTree);
	TargetTree = InTargetTree;

	check(AsyncTaskPtr == nullptr);
	AsyncTaskPtr = new FAsyncTask<FATProceduralGeneratorTask_AsyncTask>();
	AsyncTaskPtr->GetTask().TargetTask = this;
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
void UATProceduralGeneratorTask::QueueChunk(AATVoxelChunk* InChunk)
{
	ensureReturn(!QueuedChunks.Contains(InChunk));
	QueuedChunks.Add(InChunk);
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
	ensureReturn(TargetTree);

	while (!TargetTree->IsThisTickUpdatesTimeBudgetExceeded() && !QueuedChunks.IsEmpty())
	{
		TObjectPtr<class AATVoxelChunk> SampleChunk = QueuedChunks.Pop();

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

void UATProceduralGeneratorTask::FinishPostWork_GameThread()
{
	ensureReturn(bPendingPostWork);
	bPendingPostWork = false;
}
//~ End Task
