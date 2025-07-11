// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "Simulation/ATSimulationTask.h"

#include "ATSimulationTask_HealthDrain.generated.h"

/**
 *
 */
UCLASS(ClassGroup = ("Simulations"), meta = (DisplayName = "[AT] Simulation Task (HealthDrain)", BlueprintSpawnableComponent))
class AVALANCHETEST_API UATSimulationTask_HealthDrain : public UATSimulationTask
{
	GENERATED_BODY()

public:
	UATSimulationTask_HealthDrain();
	
//~ Begin Queue
protected:
	virtual bool ShouldSelectQueuedPointForUpdate(const FIntVector& InPoint) const override; // UATSimulationTask
//~ End Queue
	
//~ Begin Task
public:
	virtual void DoWork_SubThread() override; // UATSimulationTask
	virtual void PostWork_GameThread() override; // UATSimulationTask
//~ End Task

//~ Begin Health
protected:

	UPROPERTY(Category = "Health", EditAnywhere, BlueprintReadOnly)
	float HealthDrainSpeedMul;
//~ Begin Health
};
