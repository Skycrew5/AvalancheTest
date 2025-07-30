// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "ScWTypes_CommonContainers.h"

#include "ATSimulationTask.generated.h"

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	#define DEV_HANDLE_ASYNC_PENDING_STOP(...) \
		if (DEV_bPendingStop) \
		{ \
			return __VA_ARGS__; \
		}
#else
	#define DEV_HANDLE_ASYNC_PENDING_STOP()
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT

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

	const TArraySetPair<FIntVector>& GetConstQueuedPoints() const { return QueuedPoints; }
	void QueuePoint(const FIntVector& InPoint, const bool bInQueueNeighborsToo);
protected:
	virtual bool ShouldSelectQueuedPointForUpdate(const FIntVector& InPoint) const;

	UPROPERTY(Category = "Queue", EditAnywhere, BlueprintReadWrite)
	int32 QueueNeighborsRadius;

	TArraySetPair<FIntVector> QueuedPoints;
//~ End Queue

//~ Begin Task
public:

	UFUNCTION(Category = "Task", BlueprintCallable)
	EATSimulationTaskPhase GetCurrentTaskPhase() const;

	virtual void PreWork_GameThread();
	void BeginWork_GameThread();
	virtual void DoWork_SubThread() { ensure(false); }
	virtual void PostWork_GameThread() { ensure(false); }

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	TAtomic<bool> DEV_bPendingStop;
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT

protected:
	virtual void OnSelectedUpdatePointAdded(const FIntVector& InPoint) {}
	virtual void OnQueuedPointAdded(const FIntVector& InPoint) {}

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
		TargetTask->DEV_bPendingStop = false;
		TargetTask->DoWork_SubThread();
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FATSimulationTask_AsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	TObjectPtr<class UATSimulationTask> TargetTask;
};
