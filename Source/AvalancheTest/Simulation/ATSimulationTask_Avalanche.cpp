// Scientific Ways

#include "Simulation/ATSimulationTask_Avalanche.h"

#include "World/ATVoxelTree.h"
#include "World/ATVoxelChunk.h"

UATSimulationTask_Avalanche::UATSimulationTask_Avalanche()
{
	QueueNeighborsRadius = 0;

	AvalancheCooldown = 5.0;

	InstantAvalancheStabilityThreshold = 0.01f;
	InstantAvalanchePerWorkProbability = 0.001f;
}

//~ Begin Task
void UATSimulationTask_Avalanche::DoWork_SubThread() // UATSimulationTask
{
	//float DeltaTime = GetWorld()->DeltaTimeSeconds;
	float DeltaTime = 1.0f / 30.0f;

	ParallelFor(SelectedUpdatePoints.Num(), [&](int32 InIndex)
	{
		DEV_HANDLE_ASYNC_PENDING_STOP();

		const FIntVector& SamplePoint = SelectedUpdatePoints[InIndex];
		FVoxelInstanceData& SampleData = TargetTree->GetVoxelInstanceDataAtPoint(SamplePoint, false, true);

		ensureReturn(PointsAvalancheCounterMap.Contains(SamplePoint));

		if (SampleData.Stability <= InstantAvalancheStabilityThreshold || InstantAvalanchePerWorkProbability >= FMath::FRand())
		{
			PointsAvalancheCounterMap[SamplePoint] = INT_MAX;
		}
		else
		{
			++PointsAvalancheCounterMap[SamplePoint];
		}
	});
	bPendingPostWork = true;
}

void UATSimulationTask_Avalanche::PostWork_GameThread()
{
	UWorld* World = GetWorld();
	ensureReturn(World);

	double CurrentTime = World->GetTimeSeconds();
	if (CurrentTime >= NextAvalancheTime)
	{
		PendingAvalanchePointsNum = PointsAvalancheCounterMap.Num();
		NextAvalancheTime = CurrentTime + AvalancheCooldown;
	}
	ensureReturn(TargetTree);
	while (!TargetTree->IsThisTickUpdatesTimeBudgetExceeded() && !SelectedUpdatePoints.IsEmpty())
	{
		FIntVector SamplePoint = SelectedUpdatePoints.Pop();
		FVoxelInstanceData& SampleData = TargetTree->GetVoxelInstanceDataAtPoint(SamplePoint, false);

		ensureContinue(PointsAvalancheCounterMap.Contains(SamplePoint));
		int32 SampleCounter = PointsAvalancheCounterMap[SamplePoint];

		if (PendingAvalanchePointsNum > 0 || SampleCounter == INT_MAX)
		{
			PointsAvalancheCounterMap.Remove(SamplePoint);
			--PendingAvalanchePointsNum;

			TargetTree->BreakVoxelAtPoint(SamplePoint, FVoxelBreakData(true, true)); // Will update Chunk too
		}
		else
		{
			QueuePoint(SamplePoint, false);

			AATVoxelChunk* SampleChunk = TargetTree->GetVoxelChunkAtPoint(SamplePoint);
			ensureContinue(SampleChunk);

			//SampleChunk->HandleSetVoxelHealthAtPoint(SamplePoint, SampleData.Health);
			SampleChunk->HandleSetVoxelInstanceDataAtPoint(SamplePoint, SampleData);
		}
	}
	if (SelectedUpdatePoints.IsEmpty())
	{
		FinishPostWork_GameThread();
	}
}

void UATSimulationTask_Avalanche::OnSelectedUpdatePointAdded(const FIntVector& InPoint) // UATSimulationTask
{
	if (PointsAvalancheCounterMap.Contains(InPoint))
	{

	}
	else
	{
		PointsAvalancheCounterMap.Add(InPoint, 0);
	}
}
//~ End Task
