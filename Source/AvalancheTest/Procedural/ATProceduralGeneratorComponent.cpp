// Scientific Ways

#include "Procedural/ATProceduralGeneratorComponent.h"

#include "Framework/ATSaveGame_VoxelTree.h"

#include "Procedural/ATProceduralGeneratorTask.h"
#include "Procedural/ATProceduralGeneratorTask_Landscape.h"
#include "Procedural/ATProceduralGeneratorTask_PlayerStart.h"

#include "World/ATVoxelTree.h"

UATProceduralGeneratorComponent::UATProceduralGeneratorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f;

	TaskArray = {
		CreateDefaultSubobject<UATProceduralGeneratorTask_Landscape>(TEXT("ProceduralGeneratorTask_Landscape")),
		CreateDefaultSubobject<UATProceduralGeneratorTask_PlayerStart>(TEXT("ProceduralGeneratorTask_PlayerStart")),
	};
	bEnableProceduralTasks = true;

	VoxelTreeDataSaveSlot = TEXT("VoxelTreeData/DefaultSlot");
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

	UATSaveGame_VoxelTree::LoadSlot(OwnerTree, VoxelTreeDataSaveSlot);

	for (int32 SampleIndex = 0; SampleIndex < TaskArray.Num(); ++SampleIndex)
	{
		UATProceduralGeneratorTask* SampleTask = TaskArray[SampleIndex];

		ensureContinue(SampleTask);
		SampleTask->Initialize(this, SampleIndex);
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
	if (bPendingSaveGeneratedData)
	{
		if (CurrentTaskIndex == (TaskArray.Num() - 1) && bMoveToNextTask) // Just finished the last task
		{
			FinishGenerateVoxelTreeData();
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

//~ Begin Editor
void UATProceduralGeneratorComponent::GenerateVoxelTreeData()
{
	ensureReturn(OwnerTree);
	const auto& ChunksMap = OwnerTree->GetChunksMap();

	TArray<AATVoxelChunk*> ChunksToQueue;
	ChunksMap.GenerateValueArray(ChunksToQueue);

	QueueChunksForTaskAtIndex(ChunksToQueue, 0);

	bTickInEditor = true;
}

void UATProceduralGeneratorComponent::FinishGenerateVoxelTreeData()
{
	bTickInEditor = false;
	bPendingSaveGeneratedData = false;

	UATSaveGame_VoxelTree::SaveSlot(OwnerTree, VoxelTreeDataSaveSlot);
}
//~ End Editor
