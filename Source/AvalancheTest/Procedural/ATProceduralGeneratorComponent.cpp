// Scientific Ways

#include "Procedural/ATProceduralGeneratorComponent.h"

#include "Procedural/ATProceduralGeneratorTask.h"
#include "Procedural/ATProceduralGeneratorTask_Landscape.h"

#include "World/ATVoxelTree.h"

UATProceduralGeneratorComponent::UATProceduralGeneratorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f;

	TaskArray = {
		CreateDefaultSubobject<UATProceduralGeneratorTask_Landscape>(TEXT("ProceduralGeneratorTask_Landscape")),
	};
	bEnableProceduralTasks = true;
}

//~ Begin ActorComponent
void UATProceduralGeneratorComponent::OnRegister() // UActorComponent
{
	Super::OnRegister();

	OwnerTree = GetOwner<AATVoxelTree>();
	ensure(OwnerTree);
}

void UATProceduralGeneratorComponent::BeginPlay() // UActorComponent
{
	Super::BeginPlay();

	for (UATProceduralGeneratorTask* SampleTask : TaskArray)
	{
		SampleTask->Initialize(OwnerTree);
	}
}

void UATProceduralGeneratorComponent::TickComponent(float InDeltaSeconds, enum ELevelTick InTickType, FActorComponentTickFunction* InThisTickFunction) // UActorComponent
{
	Super::TickComponent(InDeltaSeconds, InTickType, InThisTickFunction);

	HandleTickUpdate(InDeltaSeconds);
}

void UATProceduralGeneratorComponent::EndPlay(const EEndPlayReason::Type InReason) // UActorComponent
{
	Super::EndPlay(InReason);

	for (UATProceduralGeneratorTask* SampleTask : TaskArray)
	{
		SampleTask->DeInitialize();
	}
}
//~ End ActorComponent

//~ Begin Update
void UATProceduralGeneratorComponent::ForceTickUpdateNextFrame()
{
	UWorld* World = GetWorld();
	ensureReturn(World);

	FTimerManager& TimerManager = World->GetTimerManager();

	if (TimerManager.IsTimerActive(ForceTickUpdateNextFrameTimerHandle))
	{

	}
	else
	{
		ForceTickUpdateNextFrameTimerHandle = TimerManager.SetTimerForNextTick(this, &UATProceduralGeneratorComponent::HandleTickUpdate_FromForceTickUpdate);
	}
}

void UATProceduralGeneratorComponent::HandleTickUpdate_FromForceTickUpdate()
{
	UWorld* World = GetWorld();
	ensureReturn(World);

	HandleTickUpdate(World->GetDeltaSeconds());
}

void UATProceduralGeneratorComponent::HandleTickUpdate(float InDeltaSeconds)
{
	HandleProceduralTasks();
}
//~ End Update

//~ Begin Queue
int32 UATProceduralGeneratorComponent::GetTotalQueuedChunksNum() const
{
	int32 OutNum = 0;

	for (UATProceduralGeneratorTask* SampleTask : TaskArray)
	{
		ensureContinue(SampleTask);
		OutNum += SampleTask->GetQueuedChunksNum();
	}
	return OutNum;
}

void UATProceduralGeneratorComponent::QueueChunksForTaskAtIndex(const TArray<AATVoxelChunk*>& InChunks, int32 InTaskIndex)
{
	ensureReturn(TaskArray.IsValidIndex(InTaskIndex));

	UATProceduralGeneratorTask* TargetTask = TaskArray[InTaskIndex];
	ensureReturn(TargetTask);

	TargetTask->QueueChunks(InChunks);
}
//~ End Queue

// Begin Tasks
bool UATProceduralGeneratorComponent::IsCurrentTaskActive() const
{
	ensureReturn(TaskArray.IsValidIndex(CurrentTaskIndex), false);

	UATProceduralGeneratorTask* CurrentTask = TaskArray[CurrentTaskIndex];
	ensureReturn(CurrentTask, false);
	return CurrentTask->GetCurrentTaskPhase() == EATProceduralGeneratorTaskPhase::DoWork;
}

UATProceduralGeneratorTask* UATProceduralGeneratorComponent::FindTaskInstanceByClass(TSubclassOf<UATProceduralGeneratorTask> InClass) const
{
	for (UATProceduralGeneratorTask* SampleTask : TaskArray)
	{
		if (SampleTask->IsA(InClass))
		{
			return SampleTask;
		}
	}
	return nullptr;
}

void UATProceduralGeneratorComponent::HandleProceduralTasks()
{
	if (!bEnableProceduralTasks)
	{
		return;
	}
	ensureReturn(TaskArray.IsValidIndex(CurrentTaskIndex));

	UATProceduralGeneratorTask* CurrentTask = TaskArray[CurrentTaskIndex];
	ensureReturn(CurrentTask);

	bool bMoveToNextTask = false;

	switch (CurrentTask->GetCurrentTaskPhase())
	{
		case EATProceduralGeneratorTaskPhase::Idle:
		{
			CurrentTask->PreWork_GameThread();
			//ensure(CurrentTask->GetCurrentTaskPhase() == EATProceduralGeneratorTaskPhase::DoWork);

			bMoveToNextTask = CurrentTask->GetCurrentTaskPhase() != EATProceduralGeneratorTaskPhase::DoWork;
			break;
		}
		case EATProceduralGeneratorTaskPhase::DoWork:
		{
			break;
		}
		case EATProceduralGeneratorTaskPhase::PendingPostWork:
		{
			CurrentTask->PostWork_GameThread();
			bMoveToNextTask = CurrentTask->GetCurrentTaskPhase() != EATProceduralGeneratorTaskPhase::PendingPostWork;
			break;
		}
	}
	if (bMoveToNextTask)
	{
		IncrementCurrentTaskIndex();
	}
}

void UATProceduralGeneratorComponent::IncrementCurrentTaskIndex()
{
	ensureReturn(!TaskArray.IsEmpty());
	CurrentTaskIndex = (CurrentTaskIndex + 1) % TaskArray.Num();
}
// End Tasks
