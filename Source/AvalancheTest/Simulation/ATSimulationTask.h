// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "ScWTypes_Containers.h"

#include "ATSimulationTask.generated.h"

UENUM(BlueprintType, meta = (DisplayName = "[AT] Simulation Task Phase"))
enum class EATSimulationTaskPhase : uint8
{
	Idle,
	DoWork,
	PendingPostWork,
};

/**
 *
 */
UCLASS(Abstract, DefaultToInstanced, ClassGroup = ("Simulations"), meta = (DisplayName = "[AT] Simulation Task", BlueprintSpawnableComponent))
class AVALANCHETEST_API UATSimulationTask : public UObject
{
	GENERATED_BODY()

public:
	UATSimulationTask();

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
	void QueuePoint(const FIntVector& InPoint, const bool bInQueueNeighborsToo = true);
protected:
	virtual bool ShouldSelectQueuedPointForUpdate(const FIntVector& InPoint) const;

	UPROPERTY(Category = "Queue", EditAnywhere, BlueprintReadWrite)
	int32 QueueNeighborsRadius = 6;

	TArraySetPair<FIntVector> QueuedPoints;
//~ End Queue

//~ Begin Task
public:

	UFUNCTION(Category = "Task", BlueprintCallable)
	EATSimulationTaskPhase GetCurrentTaskPhase() const;

	virtual void PreWork_GameThread();
	virtual void DoWork_SubThread() { ensure(false); }
	virtual void PostWork_GameThread() { ensure(false); }
protected:
	virtual void AllocateCacheAtPoint(const FIntVector& InPoint) {}
	virtual void ResetCacheAtPoint(const FIntVector& InPoint) {}

	void FinishPostWork_GameThread();

	UPROPERTY(Transient)
	bool bPendingPostWork;

	UPROPERTY(Transient)
	TArray<FIntVector> SelectedUpdatePoints;

	FAsyncTask<class FATSimulationTask_AsyncTask>* AsyncTaskPtr;
//~ End Task
};

class FATSimulationTask_AsyncTask : public FNonAbandonableTask
{
public:

	void DoWork()
	{
		ensureReturn(TargetTask);
		TargetTask->DoWork_SubThread();
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FATSimulationTask_AsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	TObjectPtr<class UATSimulationTask> TargetTask;
};
