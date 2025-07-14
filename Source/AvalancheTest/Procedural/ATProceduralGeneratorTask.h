// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "ATProceduralGeneratorTask.generated.h"

UENUM(BlueprintType, meta = (DisplayName = "[AT] Procedural Generator Task Phase"))
enum class EATProceduralGeneratorTaskPhase : uint8
{
	Idle,
	DoWork,
	PendingPostWork,
};

/**
 *
 */
UCLASS(Abstract, DefaultToInstanced, ClassGroup = ("Procedural"), meta = (DisplayName = "[AT] Procedural Generator Task", BlueprintSpawnableComponent))
class AVALANCHETEST_API UATProceduralGeneratorTask : public UObject
{
	GENERATED_BODY()

public:

	UATProceduralGeneratorTask();

//~ Begin Initialize
public:
	virtual void Initialize(class AATVoxelTree* InTargetTree);
	virtual void DeInitialize();
//~ End Initialize
	
//~ Begin Target
protected:

	UPROPERTY(Category = "Target", BlueprintReadOnly)
	TObjectPtr<class AATVoxelTree> TargetTree;
//~ End Target
	
//~ Begin Queue
public:

	UFUNCTION(Category = "Queue", BlueprintCallable)
	int32 GetQueuedChunksNum() { return QueuedChunks.Num(); }

	UFUNCTION(Category = "Queue", BlueprintCallable)
	void QueueChunk(class AATVoxelChunk* InChunk);

protected:
	virtual bool ShouldSelectQueuedChunkForUpdate(class AATVoxelChunk* InChunk) const { return true; }

	UPROPERTY(Category = "Queue", BlueprintReadOnly)
	TArray<TObjectPtr<class AATVoxelChunk>> QueuedChunks;
//~ End Queue

//~ Begin Task
public:

	UFUNCTION(Category = "Task", BlueprintCallable)
	EATProceduralGeneratorTaskPhase GetCurrentTaskPhase() const;

	UFUNCTION(Category = "Task", BlueprintCallable)
	const TArray<class AATVoxelChunk*>& GetSelectedChunks() const { return SelectedChunks; }

	virtual void PreWork_GameThread();
	virtual void DoWorkForSelectedChunk_SubThread(const class AATVoxelChunk* InTargetChunk) { ensure(false); }
	virtual void PostWork_GameThread() { ensure(false); }
protected:
	virtual void AllocatePerChunkData(class AATVoxelChunk* InChunk) {}
	virtual void RemovePerChunkData(class AATVoxelChunk* InChunk) {}

	void FinishPostWork_GameThread();

	UPROPERTY(Category = "Task", BlueprintReadOnly)
	TArray<TObjectPtr<class AATVoxelChunk>> SelectedChunks;

	UPROPERTY(Transient)
	bool bPendingPostWork;

	FAsyncTask<class FATProceduralGeneratorTask_AsyncTask>* AsyncTaskPtr;
//~ End Task
};

class FATProceduralGeneratorTask_AsyncTask : public FNonAbandonableTask
{
public:

	void DoWork()
	{
		ensureReturn(TargetTask);
		ParallelFor(TargetTask->GetSelectedChunks().Num(), [this](int32 InIndex)
		{
			const auto& SelectedChunks = TargetTask->GetSelectedChunks();
			ensureReturn(SelectedChunks.IsValidIndex(InIndex));
			TargetTask->DoWorkForSelectedChunk_SubThread(SelectedChunks[InIndex]);
		});
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FATProceduralGeneratorTask_AsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	TObjectPtr<class UATProceduralGeneratorTask> TargetTask;
};
