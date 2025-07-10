// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "ATSimulationComponent.generated.h"

/**
 *
 */
UCLASS(ClassGroup = ("Simulations"), meta = (DisplayName = "[AT] Simulation Component", BlueprintSpawnableComponent))
class AVALANCHETEST_API UATSimulationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UATSimulationComponent();

//~ Begin ActorComponent
protected:
	virtual void OnRegister() override; // UActorComponent
	virtual void BeginPlay() override; // UActorComponent
	virtual void TickComponent(float InDeltaSeconds, enum ELevelTick InTickType, FActorComponentTickFunction* InThisTickFunction) override; // UActorComponent
	virtual void EndPlay(const EEndPlayReason::Type InReason) override; // UActorComponent
//~ End ActorComponent

//~ Begin Owner
protected:

	UPROPERTY(Category = "Owner", BlueprintReadOnly)
	TObjectPtr<class AATVoxelTree> OwnerTree;
//~ End Owner

//~ Begin Update
public:

	UFUNCTION(Category = "Update", BlueprintCallable)
	void ForceTickUpdateNextFrame();

protected:
	void HandleTickUpdate_FromForceTickUpdate();
	void HandleTickUpdate(float InDeltaSeconds);

	UPROPERTY(Category = "Update", EditAnywhere, BlueprintReadOnly)
	int32 MaxUpdatesPerSecond;

	UPROPERTY(Transient)
	FTimerHandle ForceTickUpdateNextFrameTimerHandle;
//~ End Update

//~ Begin Queue
public:
	void QueuePointForTaskAtIndex(int32 InTaskIndex, const FIntVector& InPoint, const bool bInQueueNeighborPointsToo = true);
//~ End Queue

//~ Begin Tasks
public:

	UFUNCTION(Category = "Tasks", BlueprintCallable)
	bool IsCurrentTaskActive() const;

	UFUNCTION(Category = "Tasks", BlueprintCallable, meta = (DeterminesOutputType = "InClass"))
	class UATSimulationTask* FindTaskInstanceByClass(TSubclassOf<UATSimulationTask> InClass) const;

	template<class TaskClass>
	TaskClass* FindTaskInstance() const { return Cast<TaskClass>(FindTaskInstanceByClass(TaskClass::StaticClass())); }

	void HandleSimulationTasks(int32& InOutUpdatesLeft);
protected:
	void IncrementCurrentTaskIndex();

	UPROPERTY(Category = "Tasks", EditAnywhere, BlueprintReadOnly)
	bool bEnableSimulationTasks;

	UPROPERTY(Category = "Tasks", EditAnywhere, BlueprintReadOnly, Instanced)
	TArray<TObjectPtr<class UATSimulationTask>> TaskArray;

	UPROPERTY(Category = "Tasks", BlueprintReadOnly)
	int32 CurrentTaskIndex;
//~ End Tasks
};
