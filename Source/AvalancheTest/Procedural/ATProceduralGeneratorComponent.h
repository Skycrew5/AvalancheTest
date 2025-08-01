// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "ATProceduralGeneratorComponent.generated.h"

/**
 *
 */
UCLASS(ClassGroup = ("Procedural"), meta = (DisplayName = "[AT] Procedural Generator Component", BlueprintSpawnableComponent))
class AVALANCHETEST_API UATProceduralGeneratorComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UATProceduralGeneratorComponent();

//~ Begin ActorComponent
protected:
	virtual void OnRegister() override; // UActorComponent
	virtual void BeginPlay() override; // UActorComponent
	virtual void TickComponent(float InDeltaSeconds, enum ELevelTick InTickType, FActorComponentTickFunction* InThisTickFunction) override; // UActorComponent
	virtual void EndPlay(const EEndPlayReason::Type InReason) override; // UActorComponent
//~ End ActorComponent

//~ Begin Owner
public:

	UFUNCTION(Category = "Owner", BlueprintCallable)
	class AATVoxelTree* GetOwnerTree() const { return OwnerTree; }

protected:

	UPROPERTY(Transient)
	TObjectPtr<class AATVoxelTree> OwnerTree;
//~ End Owner

//~ Begin Update
public:

	UFUNCTION(Category = "Update", BlueprintCallable)
	void ForceTickUpdateNextFrame();

protected:
	void HandleTickUpdate_FromForceTickUpdate();
	void HandleTickUpdate(float InDeltaSeconds);

	UPROPERTY(Transient)
	FTimerHandle ForceTickUpdateNextFrameTimerHandle;
//~ End Update
	
//~ Begin Queue
public:

	UFUNCTION(Category = "Queue", BlueprintCallable)
	int32 GetTotalQueuedChunksNum() const;

	void QueueChunksForTaskAtIndex(const TArray<class AATVoxelChunk*>& InChunks, int32 InTaskIndex);
//~ End Queue

//~ Begin Tasks
public:

	UFUNCTION(Category = "Tasks", BlueprintCallable)
	bool IsCurrentTaskActive() const;

	UFUNCTION(Category = "Tasks", BlueprintCallable)
	bool HasTaskAtIndex(int32 InTaskIndex) const { return TaskArray.IsValidIndex(InTaskIndex); }

	UFUNCTION(Category = "Tasks", BlueprintCallable)
	class UATProceduralGeneratorTask* GetTaskAtIndex(int32 InTaskIndex) const { ensureReturn(TaskArray.IsValidIndex(InTaskIndex), nullptr); return TaskArray[InTaskIndex]; }

	UFUNCTION(Category = "Tasks", BlueprintCallable, meta = (DeterminesOutputType = "InClass"))
	class UATProceduralGeneratorTask* FindTaskInstanceByClass(TSubclassOf<UATProceduralGeneratorTask> InClass) const;

	template<class TaskClass>
	TaskClass* FindTaskInstance() const { return Cast<TaskClass>(FindTaskInstanceByClass(TaskClass::StaticClass())); }

	void HandleProceduralTasks();
protected:
	void IncrementCurrentTaskIndex();

	UPROPERTY(Category = "Tasks", EditAnywhere, BlueprintReadOnly)
	bool bEnableProceduralTasks;

	UPROPERTY(Category = "Tasks", EditAnywhere, BlueprintReadOnly, Instanced)
	TArray<TObjectPtr<class UATProceduralGeneratorTask>> TaskArray;

	UPROPERTY(Category = "Tasks", BlueprintReadOnly)
	int32 CurrentTaskIndex;
//~ End Tasks

//~ Begin Editor
public:

	UFUNCTION(Category = "Editor", CallInEditor)
	void GenerateVoxelTreeData();

	UPROPERTY(Category = "Editor", EditAnywhere, BlueprintReadWrite)
	FString VoxelTreeDataSaveSlot;

	UPROPERTY(Category = "Editor", BlueprintReadWrite)
	bool bPendingSaveGeneratedData;

protected:
	void FinishGenerateVoxelTreeData();
//~ End Editor
};
