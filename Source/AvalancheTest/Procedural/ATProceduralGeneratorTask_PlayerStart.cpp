// Scientific Ways

#include "Procedural/ATProceduralGeneratorTask_PlayerStart.h"

#include "Procedural/ATProceduralGeneratorComponent.h"
#include "Procedural/ATProceduralGeneratorTask_Landscape.h"

#include "World/ATVoxelTree.h"
#include "World/ATVoxelChunk.h"
#include "World/ATVoxelTypeData.h"
#include "World/ATWorldFunctionLibrary.h"

#include "FastNoise2/FastNoise2BlueprintLibrary.h"

UATProceduralGeneratorTask_PlayerStart::UATProceduralGeneratorTask_PlayerStart()
{
	PlatformSize = FIntVector(32, 32, 48);
}

//~ Begin Initialize
void UATProceduralGeneratorTask_PlayerStart::Initialize(class UATProceduralGeneratorComponent* InOwnerComponent, int32 InTaskIndex) // UATProceduralGeneratorTask
{
	Super::Initialize(InOwnerComponent, InTaskIndex);

	ensureReturn(PerlinGenerator == nullptr);
	PerlinGenerator = UFastNoise2BlueprintLibrary::MakePerlinGenerator();

	ensureReturn(OwnerComponent);
	LandscapeTask = OwnerComponent->FindTaskInstance<UATProceduralGeneratorTask_Landscape>();
}
//~ End Initialize

//~ Begin Task
void UATProceduralGeneratorTask_PlayerStart::PreWork_GameThread() // UATProceduralGeneratorTask
{
	ensureReturn(PlatformVoxelTypeData);

	Super::PreWork_GameThread();
}

void UATProceduralGeneratorTask_PlayerStart::DoWorkGlobalOnce_SubThread() // UATProceduralGeneratorTask
{
	ensureReturn(OwnerTree);
	int32 TreeSeed = OwnerTree->GetTreeSeed();
	FIntVector BoundsSize = OwnerTree->GetBoundsSize();

	FIntVector PlatformCenter = BoundsSize / 2;

	ensureReturn(LandscapeTask);
	PlatformCenter.Z = FMath::RoundFromZero((float)BoundsSize.Z * LandscapeTask->GetHillsValueAtPoint2D(FIntPoint(PlatformCenter.X, PlatformCenter.Y)));
	PlatformCenter.Z -= PlatformSize.Z / 2 + 4;

	FIntVector PlatformBackLeftCorner = PlatformCenter - PlatformSize / 2;
	FIntVector PlatformFrontRightCorner = PlatformCenter + PlatformSize / 2;

	ensureReturn(PlatformPointsMap.IsEmpty());
	for (int32 SamplePlatformX = PlatformBackLeftCorner.X; SamplePlatformX < PlatformFrontRightCorner.X; ++SamplePlatformX)
	{
		for (int32 SamplePlatformY = PlatformBackLeftCorner.Y; SamplePlatformY < PlatformFrontRightCorner.Y; ++SamplePlatformY)
		{
			for (int32 SamplePlatformZ = PlatformBackLeftCorner.Z; SamplePlatformZ < PlatformFrontRightCorner.Z; ++SamplePlatformZ)
			{
				FIntVector SamplePlatformPoint = FIntVector(SamplePlatformX, SamplePlatformY, SamplePlatformZ);
				PlatformPointsMap.Add(SamplePlatformPoint, PlatformVoxelTypeData);
			}
			for (int32 SampleAbovePlatformZ = PlatformFrontRightCorner.Z; SampleAbovePlatformZ < BoundsSize.Z; ++SampleAbovePlatformZ)
			{
				FIntVector SampleAbovePlatformPoint = FIntVector(SamplePlatformX, SamplePlatformY, SampleAbovePlatformZ);
				PlatformPointsMap.Add(SampleAbovePlatformPoint, nullptr);
			}
		}
	}
}

void UATProceduralGeneratorTask_PlayerStart::DoWorkForSelectedChunk_SubThread(const AATVoxelChunk* InTargetChunk) // UATProceduralGeneratorTask
{
	ensureReturn(InTargetChunk);

	ensureReturn(LandscapeTask);
	auto& Data = PerChunkData[InTargetChunk];
	const auto& LandscapeData = LandscapeTask->GetChunkData(InTargetChunk);
	
	ensureReturn(OwnerTree);
	int32 TreeSeed = OwnerTree->GetTreeSeed();
	FIntVector BoundsSize = OwnerTree->GetBoundsSize();

	int32 ChunkSeed = InTargetChunk->GetChunkSeed();
	FIntVector ChunkCoords = InTargetChunk->GetChunkCoords();

	FIntVector ChunkBackLeftCorner = InTargetChunk->GetChunkBackLeftCornerPoint();
	FIntVector ChunkSize = FIntVector(InTargetChunk->GetChunkSize());

	for (int32 SampleLocalX = 0; SampleLocalX < ChunkSize.X; ++SampleLocalX)
	{
		for (int32 SampleLocalY = 0; SampleLocalY < ChunkSize.Y; ++SampleLocalY)
		{
			for (int32 SampleLocalZ = 0; SampleLocalZ < ChunkSize.Z; ++SampleLocalZ)
			{
				FIntVector SampleLocalPoint = FIntVector(SampleLocalX, SampleLocalY, SampleLocalZ);
				FIntVector SampleGlobalPoint = ChunkBackLeftCorner + SampleLocalPoint;
				
				if (PlatformPointsMap.Contains(SampleGlobalPoint))
				{
					Data.PointsArray.Add(SampleGlobalPoint);
					Data.TypeDataArray.Add(PlatformPointsMap[SampleGlobalPoint]);
				}
			}
		}
	}
	bPendingPostWork = true;
}

void UATProceduralGeneratorTask_PlayerStart::PostWork_GameThread()
{
	ensureReturn(OwnerTree);
	while (!OwnerTree->IsThisTickUpdatesTimeBudgetExceeded() && !SelectedChunks.IsEmpty())
	{
		AATVoxelChunk* SampleChunk = SelectedChunks.Last();
		ensureContinue(SampleChunk);

		FIntVector ChunkBackLeftCornerPoint = SampleChunk->GetChunkBackLeftCornerPoint();
		FIntVector ChunkSize = FIntVector(SampleChunk->GetChunkSize());

		ensureContinue(PerChunkData.Contains(SampleChunk));
		auto& SampleData = PerChunkData[SampleChunk];

		for (int32 SampleArrayIndex = SampleData.LastProcessedIndex + 1; SampleArrayIndex < SampleData.TypeDataArray.Num(); ++SampleArrayIndex)
		{
			SampleData.LastProcessedIndex = SampleArrayIndex;

			const UATVoxelTypeData* SampleTypeData = SampleData.TypeDataArray[SampleArrayIndex];
			const FIntVector SamplePoint = SampleData.PointsArray[SampleArrayIndex];

			if (SampleTypeData)
			{
				OwnerTree->SetVoxelAtPoint(SamplePoint, SampleTypeData, true);
			}
			else
			{
				OwnerTree->BreakVoxelAtPoint(SamplePoint, FVoxelBreakData(true, false));
			}
			if (OwnerTree->IsThisTickUpdatesTimeBudgetExceeded())
			{
				break;
			}
		}
		if (SampleData.LastProcessedIndex == SampleData.TypeDataArray.Num() - 1) // Accounts for empty arrays too
		{
			FinishPostWorkWithChunk(SampleChunk);
		}
	}
	if (SelectedChunks.IsEmpty())
	{
		FinishPostWork_GameThread();
	}
}
//~ End Task
