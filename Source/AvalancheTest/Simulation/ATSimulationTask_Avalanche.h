// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "Simulation/ATSimulationTask.h"

#include "ATSimulationTask_Avalanche.generated.h"

/**
 *
 */
UCLASS(ClassGroup = ("Simulations"), meta = (DisplayName = "[AT] Simulation Task (Avalanche)", BlueprintSpawnableComponent))
class AVALANCHETEST_API UATSimulationTask_Avalanche : public UATSimulationTask
{
	GENERATED_BODY()

public:
	UATSimulationTask_Avalanche();
	
//~ Begin Task
public:
	virtual void DoWork_SubThread() override; // UATSimulationTask
	virtual void PostWork_GameThread() override; // UATSimulationTask
protected:
	virtual void OnSelectedUpdatePointAdded(const FIntVector& InPoint) override; // UATSimulationTask
	//virtual void OnQueuedPointAdded(const FIntVector& InPoint) override { PointsAvalancheCounterMap.Remove(InPoint); } // UATSimulationTask
//~ End Task

//~ Begin Avalanche
protected:

	UPROPERTY(Category = "Avalanche", EditAnywhere, BlueprintReadWrite)
	float AvalancheCooldown;

	UPROPERTY(Category = "Avalanche", EditAnywhere, BlueprintReadWrite)
	float InstantAvalancheStabilityThreshold;

	UPROPERTY(Category = "Avalanche", EditAnywhere, BlueprintReadWrite)
	float InstantAvalanchePerWorkProbability;

	UPROPERTY(Transient)
	double NextAvalancheTime;

	UPROPERTY(Transient)
	int32 PendingAvalanchePointsNum;

	UPROPERTY(Transient)
	TMap<FIntVector, int32> PointsAvalancheCounterMap;
//~ Begin Avalanche
};
