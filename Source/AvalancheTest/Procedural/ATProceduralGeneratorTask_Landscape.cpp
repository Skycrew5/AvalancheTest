// Scientific Ways

#include "Procedural/ATProceduralGeneratorTask_Landscape.h"

#include "World/ATVoxelTree.h"
#include "World/ATVoxelChunk.h"
#include "World/ATVoxelTypeData.h"
#include "World/ATWorldFunctionLibrary.h"

#include "FastNoise2/FastNoise2BlueprintLibrary.h"

UATProceduralGeneratorTask_Landscape::UATProceduralGeneratorTask_Landscape()
{
	HillsNoiseFrequency = 0.01f;

	DefaultToWeakThreshold = 0.02f;
}

//~ Begin Initialize
void UATProceduralGeneratorTask_Landscape::Initialize(AATVoxelTree* InTargetTree) // UATProceduralGeneratorTask
{
	Super::Initialize(InTargetTree);

	ensureReturn(PerlinGenerator == nullptr);
	PerlinGenerator = UFastNoise2BlueprintLibrary::MakePerlinGenerator();
}
//~ End Initialize

//~ Begin Task
void UATProceduralGeneratorTask_Landscape::PreWork_GameThread() // UATProceduralGeneratorTask
{
	ensureReturn(DefaultVoxelTypeData);
	ensureReturn(WeakVoxelTypeData);

	ensureReturn(FoundationVoxelTypeData);
	ensureReturn(FoundationVoxelTypeData->bIsFoundation);

	Super::PreWork_GameThread();
}

void UATProceduralGeneratorTask_Landscape::DoWorkForSelectedChunk_SubThread(const AATVoxelChunk* InTargetChunk) // UATProceduralGeneratorTask
{
	ensureReturn(PerChunkData.Contains(InTargetChunk));

	FTaskChunkData& Data = PerChunkData[InTargetChunk];
	Data.TypeDataArray.SetNum(FMath::Cube(InTargetChunk->GetChunkSize()));

	ensureReturn(InTargetChunk->GetOwnerTree());
	int32 TreeSeed = InTargetChunk->GetOwnerTree()->GetTreeSeed();
	int32 ChunkSeed = InTargetChunk->GetChunkSeed();

	FIntVector ChunkCoords = InTargetChunk->GetChunkCoords();
	FIntVector ChunkBackLeftCornerPoint = InTargetChunk->GetChunkBackLeftCornerPoint();
	FIntVector ChunkSize = FIntVector(InTargetChunk->GetChunkSize());

	TArray<float> PerlinValues3D;
	FIntVector PerlinStart3D = ChunkBackLeftCornerPoint;
	FIntVector PerlinSize3D = ChunkSize;
	/*PerlinGenerator->GenUniformGrid3D(PerlinValues3D, PerlinStart3D, PerlinSize3D, 0.01f, TreeSeed);

	for (int32 SampleArrayIndex = 0; SampleArrayIndex < PerlinValues3D.Num(); ++SampleArrayIndex)
	{
		float SamplePerlinValue = PerlinValues3D[SampleArrayIndex];

		if (SamplePerlinValue > 0.0f)
		//if (FMath::RandBool())
		{
			if (SamplePerlinValue > 0.1f)
			{
				Data.TypeDataArray[SampleArrayIndex] = WeakVoxelTypeData;
			}
			else
			{
				Data.TypeDataArray[SampleArrayIndex] = DefaultVoxelTypeData;
			}
		}
	}*/
	TArray<float> PerlinValues2D;
	FIntPoint PerlinStart2D = FIntPoint(ChunkBackLeftCornerPoint.X, ChunkBackLeftCornerPoint.Y);
	FIntPoint PerlinSize2D = FIntPoint(ChunkSize.X, ChunkSize.Y);
	PerlinGenerator->GenUniformGrid2D(PerlinValues2D, PerlinStart2D, PerlinSize2D, HillsNoiseFrequency, TreeSeed);

	int32 ChunkTreeMaxZ = TargetTree->GetBoundsSize().Z;

	for (int32 SampleArrayIndex2D = 0; SampleArrayIndex2D < PerlinValues2D.Num(); ++SampleArrayIndex2D)
	{
		float SamplePerlinValue = PerlinValues2D[SampleArrayIndex2D];
		float SampleNormalizedPerlinValue = (SamplePerlinValue + 1.0f) * 0.5f;

		SampleNormalizedPerlinValue = FMath::Pow(SampleNormalizedPerlinValue, TargetTree->PerlinPow);

		for (int32 LocalZ = 0; LocalZ < ChunkSize.Z; ++LocalZ)
		{
			int32 GlobalZ = PerlinStart3D.Z + LocalZ;
			float GlobalAlphaZ = float(GlobalZ) / float(ChunkTreeMaxZ);

			if (SampleNormalizedPerlinValue > GlobalAlphaZ)
			{
				int32 SampleArrayIndex3D = UATWorldFunctionLibrary::ArrayIndex2D_To_Point3D(SampleArrayIndex2D, LocalZ, ChunkSize);

				if (SampleNormalizedPerlinValue - GlobalAlphaZ > DefaultToWeakThreshold)
				{
					Data.TypeDataArray[SampleArrayIndex3D] = WeakVoxelTypeData;
				}
				else
				{
					Data.TypeDataArray[SampleArrayIndex3D] = DefaultVoxelTypeData;
				}
			}
		}
	}
	if (ChunkCoords.Z == 0)
	{
		for (int32 SampleX = 0; SampleX < ChunkSize.X; ++SampleX)
		{
			for (int32 SampleY = 0; SampleY < ChunkSize.Y; ++SampleY)
			{
				FIntVector SamplePoint = FIntVector(SampleX, SampleY, 0);
				
				int32 SampleArrayIndex3D = UATWorldFunctionLibrary::Point_To_ArrayIndex(SamplePoint, ChunkSize);
				Data.TypeDataArray[SampleArrayIndex3D] = FoundationVoxelTypeData;
			}
		}
	}
	bPendingPostWork = true;
}

void UATProceduralGeneratorTask_Landscape::PostWork_GameThread()
{
	ensureReturn(TargetTree);
	while (!TargetTree->IsThisTickUpdatesTimeBudgetExceeded() && !SelectedChunks.IsEmpty())
	{
		AATVoxelChunk* SampleChunk = SelectedChunks.Last();
		ensureContinue(SampleChunk);

		FIntVector ChunkBackLeftCornerPoint = SampleChunk->GetChunkBackLeftCornerPoint();
		FIntVector ChunkSize = FIntVector(SampleChunk->GetChunkSize());

		ensureContinue(PerChunkData.Contains(SampleChunk));
		FTaskChunkData& SampleData = PerChunkData[SampleChunk];

		for (int32 SampleArrayIndex = SampleData.LastProcessedIndex + 1; SampleArrayIndex < SampleData.TypeDataArray.Num(); ++SampleArrayIndex)
		{
			SampleData.LastProcessedIndex = SampleArrayIndex;

			if (const UATVoxelTypeData* SampleTypeData = SampleData.TypeDataArray[SampleArrayIndex])
			{
				FIntVector SamplePoint = ChunkBackLeftCornerPoint + UATWorldFunctionLibrary::ArrayIndex_To_Point(SampleArrayIndex, ChunkSize);
				TargetTree->SetVoxelAtPoint(SamplePoint, SampleTypeData, true);

				if (TargetTree->IsThisTickUpdatesTimeBudgetExceeded())
				{
					break;
				}
			}
		}
		if (SampleData.LastProcessedIndex == SampleData.TypeDataArray.Num() - 1)
		{
			PerChunkData.Remove(SampleChunk);
			SelectedChunks.Remove(SampleChunk);

			SampleChunk->MarkChunkAsSimulationReady();
		}
	}
	if (SelectedChunks.IsEmpty())
	{
		FinishPostWork_GameThread();
	}
}
//~ End Task
