// Scientific Ways

#include "Simulations/ATSimulationTask_HealthDrain.h"

#include "World/ATVoxelTree.h"
#include "World/ATVoxelChunk.h"

UATSimulationTask_HealthDrain::UATSimulationTask_HealthDrain()
{
	HealthDrainSpeedMul = 1000.0f;
	
}

//~ Begin Queue
bool UATSimulationTask_HealthDrain::ShouldSelectQueuedPointForUpdate(const FIntVector& InPoint) const // UATSimulationTask
{
	ensureReturn(TargetTree, false);
	if (!TargetTree->HasVoxelInstanceDataAtPoint(InPoint))
	{
		return false;
	}
	FVoxelInstanceData& SampleData = TargetTree->GetVoxelInstanceDataAtPoint(InPoint, false);
	return SampleData.Stability < 0.25f;
}
//~ End Queue

//~ Begin Task
void UATSimulationTask_HealthDrain::DoWork_SubThread() // UATSimulationTask
{
	//float DeltaTime = GetWorld()->DeltaTimeSeconds;
	float DeltaTime = 1.0f / 30.0f;

	ParallelFor(SelectedUpdatePoints.Num(), [&](int32 InIndex)
	{
		const FIntVector& SamplePoint = SelectedUpdatePoints[InIndex];
		FVoxelInstanceData& SampleData = TargetTree->GetVoxelInstanceDataAtPoint(SamplePoint, false, true);

		//ensure(SampleData.Stability < 0.25f);

		float HealthDrainSpeed = FMath::Max(FMath::Square(0.25f - SampleData.Stability), 0.1f);
		SampleData.Health -= HealthDrainSpeed * HealthDrainSpeedMul * DeltaTime;
	});
	bPendingPostWork = true;
}

void UATSimulationTask_HealthDrain::PostWork_GameThread(int32& InOutUpdatesLeft)
{
	while (InOutUpdatesLeft > 0 && !SelectedUpdatePoints.IsEmpty())
	{
		InOutUpdatesLeft -= 1;
		FIntVector SamplePoint = SelectedUpdatePoints.Pop();

		FVoxelInstanceData& SampleData = TargetTree->GetVoxelInstanceDataAtPoint(SamplePoint, false);

		if (SampleData.Health > 0.0f)
		{
			QueuePoint(SamplePoint);

			AATVoxelChunk* SampleChunk = TargetTree->GetVoxelChunkAtPoint(SamplePoint);
			ensureContinue(SampleChunk);

			//SampleChunk->HandleSetVoxelHealthAtPoint(SamplePoint, SampleData.Health);
			SampleChunk->HandleSetVoxelInstanceDataAtPoint(SamplePoint, SampleData);
		}
		else
		{
			TargetTree->BreakVoxelAtPoint(SamplePoint, true, true); // Will update Chunk too
		}
	}
	if (SelectedUpdatePoints.IsEmpty())
	{
		FinishPostWork_GameThread();
	}
}
//~ End Task
