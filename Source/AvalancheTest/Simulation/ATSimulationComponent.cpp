// Scientific Ways

#include "Simulation/ATSimulationComponent.h"

#include "Simulation/ATSimulationTask.h"
#include "Simulation/ATSimulationTask_Avalanche.h"
#include "Simulation/ATSimulationTask_StabilityWaves.h"
//#include "Simulation/ATSimulationTask_StabilityRecursive.h"

#include "World/ATVoxelTree.h"

UATSimulationComponent::UATSimulationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f;

	TaskArray = {
		CreateDefaultSubobject<UATSimulationTask_StabilityWaves>(TEXT("SimulationTask_StabilityWaves")),
		//CreateDefaultSubobject<UATSimulationTask_StabilityRecursive>(TEXT("SimulationTask_StabilityRecursive")),
		CreateDefaultSubobject<UATSimulationTask_Avalanche>(TEXT("SimulationTask_Avalanche"))
	};
	bEnableSimulationTasks = true;
	MaxUpdatesPerSecond = 10000;
}

//~ Begin ActorComponent
void UATSimulationComponent::OnRegister() // UActorComponent
{
	Super::OnRegister();

	OwnerTree = GetOwner<AATVoxelTree>();
	ensure(OwnerTree);
}

void UATSimulationComponent::BeginPlay() // UActorComponent
{
	Super::BeginPlay();

	for (UATSimulationTask* SampleTask : TaskArray)
	{
		SampleTask->Initialize(OwnerTree);
	}
}

void UATSimulationComponent::TickComponent(float InDeltaSeconds, enum ELevelTick InTickType, FActorComponentTickFunction* InThisTickFunction) // UActorComponent
{
	Super::TickComponent(InDeltaSeconds, InTickType, InThisTickFunction);

	HandleTickUpdate(InDeltaSeconds);

	SIZE_T TasksQueuedPointsAllocatedSize = 0;
	for (UATSimulationTask* SampleTask : TaskArray)
	{
		const auto& SampleQueuedPoints = SampleTask->GetConstQueuedPoints();
		TasksQueuedPointsAllocatedSize += SampleQueuedPoints.GetAllocatedSize();
	}
	//UE_LOG(LogVoxels, Log, TEXT("TasksQueuedPointsAllocatedSize %i"), TasksQueuedPointsAllocatedSize);
	//SET_FLOAT_STAT(STAT_SimulationTasks_QueuedPoints, SIZE_T_TO_MB(TasksQueuedPointsAllocatedSize));
	SET_MEMORY_STAT(STAT_SimulationTasks_QueuedPoints, TasksQueuedPointsAllocatedSize);
}

void UATSimulationComponent::EndPlay(const EEndPlayReason::Type InReason) // UActorComponent
{
	Super::EndPlay(InReason);

	for (UATSimulationTask* SampleTask : TaskArray)
	{
		SampleTask->DeInitialize();
	}
}
//~ End ActorComponent

//~ Begin Update
void UATSimulationComponent::ForceTickUpdateNextFrame()
{
	UWorld* World = GetWorld();
	ensureReturn(World);

	FTimerManager& TimerManager = World->GetTimerManager();

	if (TimerManager.IsTimerActive(ForceTickUpdateNextFrameTimerHandle))
	{

	}
	else
	{
		ForceTickUpdateNextFrameTimerHandle = TimerManager.SetTimerForNextTick(this, &UATSimulationComponent::HandleTickUpdate_FromForceTickUpdate);
	}
}

void UATSimulationComponent::HandleTickUpdate_FromForceTickUpdate()
{
	UWorld* World = GetWorld();
	ensureReturn(World);

	HandleTickUpdate(World->GetDeltaSeconds());
}

void UATSimulationComponent::HandleTickUpdate(float InDeltaSeconds)
{
	HandleSimulationTasks();
}
//~ End Update

//~ Begin Queue
void UATSimulationComponent::QueuePointForTaskAtIndex(int32 InTaskIndex, const FIntVector& InPoint, const bool bInQueueNeighborPointsToo)
{
	ensureReturn(TaskArray.IsValidIndex(InTaskIndex));

	UATSimulationTask* TargetTask = TaskArray[InTaskIndex];
	ensureReturn(TargetTask);

	TargetTask->QueuePoint(InPoint, bInQueueNeighborPointsToo);
}
//~ End Queue

// Begin Tasks
bool UATSimulationComponent::IsCurrentTaskActive() const
{
	ensureReturn(TaskArray.IsValidIndex(CurrentTaskIndex), false);

	UATSimulationTask* CurrentTask = TaskArray[CurrentTaskIndex];
	ensureReturn(CurrentTask, false);
	return CurrentTask->GetCurrentTaskPhase() == EATSimulationTaskPhase::DoWork;
}

UATSimulationTask* UATSimulationComponent::FindTaskInstanceByClass(TSubclassOf<UATSimulationTask> InClass) const
{
	for (UATSimulationTask* SampleTask : TaskArray)
	{
		ensureContinue(SampleTask);

		if (SampleTask->IsA(InClass))
		{
			return SampleTask;
		}
	}
	return nullptr;
}

void UATSimulationComponent::HandleSimulationTasks()
{
	if (!bEnableSimulationTasks)
	{
		return;
	}
	ensureReturn(TaskArray.IsValidIndex(CurrentTaskIndex));

	UATSimulationTask* CurrentTask = TaskArray[CurrentTaskIndex];
	ensureReturn(CurrentTask);

	bool bMoveToNextTask = false;

	switch (CurrentTask->GetCurrentTaskPhase())
	{
		case EATSimulationTaskPhase::Idle:
		{
			CurrentTask->PreWork_GameThread();
			bMoveToNextTask = CurrentTask->GetCurrentTaskPhase() != EATSimulationTaskPhase::DoWork;
			break;
		}
		case EATSimulationTaskPhase::DoWork:
		{
			break;
		}
		case EATSimulationTaskPhase::PendingPostWork:
		{
			CurrentTask->PostWork_GameThread();
			bMoveToNextTask = CurrentTask->GetCurrentTaskPhase() != EATSimulationTaskPhase::PendingPostWork;
			break;
		}
	}
	if (CurrentTask->GetCurrentTaskPhase() == EATSimulationTaskPhase::Idle)
	{
		OwnerTree->ApplyQueued_Point_To_VoxelInstanceData_Map();
	}
	if (bMoveToNextTask)
	{
		IncrementCurrentTaskIndex();
	}
}

void UATSimulationComponent::IncrementCurrentTaskIndex()
{
	ensureReturn(!TaskArray.IsEmpty());
	CurrentTaskIndex = (CurrentTaskIndex + 1) % TaskArray.Num();
}
// End Tasks
