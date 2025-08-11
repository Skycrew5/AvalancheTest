// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "World/ATTypes_World.h"

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
	virtual void Initialize(class UATProceduralGeneratorComponent* InOwnerComponent, int32 InTaskIndex);
	virtual void DeInitialize();
//~ End Initialize
	
//~ Begin Owner
protected:

	UPROPERTY(Category = "Owner", BlueprintReadOnly)
	TObjectPtr<class UATProceduralGeneratorComponent> OwnerComponent;

	UPROPERTY(Category = "Owner", BlueprintReadOnly)
	TObjectPtr<class AATVoxelTree> OwnerTree;
//~ End Owner
	
//~ Begin Queue
public:

	UFUNCTION(Category = "Queue", BlueprintCallable)
	int32 GetQueuedChunksNum() { return QueuedChunksData.DataArray.Num(); }

	UFUNCTION(Category = "Queue", BlueprintCallable)
	void QueueChunks(const TArray<class AATVoxelChunk*>& InChunks);

protected:
	virtual bool ShouldSelectQueuedChunkForUpdate(class AATVoxelChunk* InChunk) const { return true; }

	UPROPERTY(Transient)
	FSortedChunksBySquaredDistance QueuedChunksData;
//~ End Queue

//~ Begin Task
public:

	UFUNCTION(Category = "Task", BlueprintCallable)
	EATProceduralGeneratorTaskPhase GetCurrentTaskPhase() const;

	UFUNCTION(Category = "Task", BlueprintCallable)
	const TArray<class AATVoxelChunk*>& GetSelectedChunks() const { return SelectedChunks; }

	UFUNCTION(Category = "Task", BlueprintCallable)
	bool WasDoWorkGlobalOnceCompleted() const { return bDoWorkGlobalOnceCompleted; }

	virtual void PreWork_GameThread();
	void BeginWork_GameThread();

	virtual void DoWorkGlobalOnce_SubThread() { }
	void MarkDoWorkGlobalOnceAsCompleted() { bDoWorkGlobalOnceCompleted = true; }

	virtual void DoWorkForSelectedChunk_SubThread(const class AATVoxelChunk* InTargetChunk) { ensure(false); }
	virtual void PostWork_GameThread() { ensure(false); }
protected:
	virtual void AllocatePerChunkData(class AATVoxelChunk* InChunk) {}
	virtual void RemovePerChunkData(class AATVoxelChunk* InChunk) {}

	void FinishPostWorkWithChunk(class AATVoxelChunk* InChunk);

	virtual void PushChunkForNextTask(class AATVoxelChunk* InChunk);
	virtual void HandleLastTaskFinishedForChunk(class AATVoxelChunk* InChunk);

	void FinishPostWork_GameThread();

	UPROPERTY(Category = "Task", BlueprintReadOnly)
	TArray<TObjectPtr<class AATVoxelChunk>> SelectedChunks;

	UPROPERTY(Transient)
	bool bDoWorkGlobalOnceCompleted;

	UPROPERTY(Transient)
	bool bPendingPostWork;

	//UPROPERTY(Transient)
	FAsyncTask<class FATProceduralGeneratorTask_AsyncTask>* AsyncTaskPtr;
//~ End Task

//~ Begin Next
protected:

	UPROPERTY(Transient)
	TObjectPtr<class UATProceduralGeneratorTask> NextTask;
//~ End Next
};

class FATProceduralGeneratorTask_AsyncTask : public FNonAbandonableTask
{
public:

	void DoWork()
	{
		ensureReturn(TargetTask);

		if (!TargetTask->WasDoWorkGlobalOnceCompleted())
		{
			TargetTask->DoWorkGlobalOnce_SubThread();
			TargetTask->MarkDoWorkGlobalOnceAsCompleted();
		}
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
