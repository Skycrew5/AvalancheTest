// Scientific Ways

#include "Procedural/ATProceduralGeneratorTask_Landscape.h"

#include "World/ATVoxelTree.h"
#include "World/ATVoxelChunk.h"
#include "World/ATVoxelTypeData.h"
#include "World/ATWorldFunctionLibrary.h"

#include "FastNoise2/FastNoise2BlueprintLibrary.h"

UATProceduralGeneratorTask_Landscape::UATProceduralGeneratorTask_Landscape()
{
	DefaultToWeakWidth = 0.02f;

	HillsNoiseFrequency = 0.01f;
	HillsHeightOffset = 0.5f;
	HillsHeightPow = 3.0f;

	CavesNoiseFrequency = 0.01f;
	CavesThreshold = 0.4f;

	OresNoiseFrequency = 0.05f;
	OresThreshold = 0.25f;
}

//~ Begin Initialize
void UATProceduralGeneratorTask_Landscape::Initialize(AATVoxelTree* InTargetTree) // UATProceduralGeneratorTask
{
	Super::Initialize(InTargetTree);

	ensureReturn(PerlinGenerator == nullptr);
	PerlinGenerator = UFastNoise2BlueprintLibrary::MakePerlinGenerator();

	//ensureReturn(CellularDistanceGenerator == nullptr);
	//CellularDistanceGenerator = UFastNoise2BlueprintLibrary::MakeCellularDistanceGenerator(PerlinGenerator);
}
//~ End Initialize

//~ Begin Task
void UATProceduralGeneratorTask_Landscape::PreWork_GameThread() // UATProceduralGeneratorTask
{
	ensureReturn(DefaultVoxelTypeData);
	ensureReturn(WeakVoxelTypeData);

	ensureReturn(FoundationVoxelTypeData);
	ensureReturn(FoundationVoxelTypeData->bIsUnbreakable);
	ensureReturn(FoundationVoxelTypeData->bHasInfiniteStability);

	ensureReturn(!OresVoxelTypeDataArray.IsEmpty());

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

	/*for (int32 SampleArrayIndex = 0; SampleArrayIndex < GeneratorValues3D.Num(); ++SampleArrayIndex)
	{
		float SampleGeneratorValue = GeneratorValues3D[SampleArrayIndex];

		if (SampleGeneratorValue > 0.0f)
		//if (FMath::RandBool())
		{
			if (SampleGeneratorValue > 0.1f)
			{
				Data.TypeDataArray[SampleArrayIndex] = WeakVoxelTypeData;
			}
			else
			{
				Data.TypeDataArray[SampleArrayIndex] = DefaultVoxelTypeData;
			}
		}
	}*/
	TArray<float> HillsValues2D;
	FIntPoint GeneratorStart2D = FIntPoint(ChunkBackLeftCornerPoint.X, ChunkBackLeftCornerPoint.Y);
	FIntPoint GeneratorSize2D = FIntPoint(ChunkSize.X, ChunkSize.Y);
	PerlinGenerator->GenUniformGrid2D(HillsValues2D, GeneratorStart2D, GeneratorSize2D, HillsNoiseFrequency, TreeSeed);

	TArray<float> CavesValues3D;
	FIntVector GeneratorStart3D = ChunkBackLeftCornerPoint;
	FIntVector GeneratorSize3D = ChunkSize;
	PerlinGenerator->GenUniformGrid3D(CavesValues3D, GeneratorStart3D, GeneratorSize3D, CavesNoiseFrequency, TreeSeed + 10);

	TArray<float> OresValues3D;
	PerlinGenerator->GenUniformGrid3D(OresValues3D, GeneratorStart3D, GeneratorSize3D, OresNoiseFrequency, TreeSeed + 100);

	int32 ChunkTreeMaxZ = TargetTree->GetBoundsSize().Z;

	for (int32 SampleArrayIndex2D = 0; SampleArrayIndex2D < HillsValues2D.Num(); ++SampleArrayIndex2D)
	{
		float SampleHillsValue2D = HillsValues2D[SampleArrayIndex2D];

		// Normalize
		SampleHillsValue2D = (SampleHillsValue2D + 1.0f) * 0.5f;

		// Power
		SampleHillsValue2D = FMath::Pow(SampleHillsValue2D, HillsHeightPow);

		// Offset
		SampleHillsValue2D = FMath::Lerp(HillsHeightOffset, 1.0f, SampleHillsValue2D);

		for (int32 LocalZ = 0; LocalZ < ChunkSize.Z; ++LocalZ)
		{
			int32 GlobalZ = GeneratorStart3D.Z + LocalZ;
			float GlobalAlphaZ = float(GlobalZ) / float(ChunkTreeMaxZ);

			if (GlobalAlphaZ < SampleHillsValue2D) // If below hills
			{
				int32 SampleArrayIndex3D = UATWorldFunctionLibrary::ArrayIndex2D_To_ArrayIndex3D(SampleArrayIndex2D, LocalZ, ChunkSize);

				if (SampleHillsValue2D - GlobalAlphaZ > DefaultToWeakWidth) // If below default voxels layer
				{
					float SampleCavesValue3D = CavesValues3D[SampleArrayIndex3D];

					// Normalize
					SampleCavesValue3D = (SampleCavesValue3D + 1.0f) * 0.5f;

					if (SampleCavesValue3D > CavesThreshold) // If outside cave
					{
						float SampleOresValue3D = OresValues3D[SampleArrayIndex3D];

						// Normalize
						SampleOresValue3D = (SampleOresValue3D + 1.0f) * 0.5f;

						if (SampleOresValue3D < OresThreshold) // If inside ores
						{
							float BelowHillsAlpha = (SampleHillsValue2D - GlobalAlphaZ) / SampleHillsValue2D;
							int32 SampleOreTypeDataIndex = FMath::RoundToInt(BelowHillsAlpha * (float)OresVoxelTypeDataArray.Num()) - 1;

							// Clamp
							SampleOreTypeDataIndex = FMath::Clamp(SampleOreTypeDataIndex, 0, OresVoxelTypeDataArray.Num() - 1);
							Data.TypeDataArray[SampleArrayIndex3D] = OresVoxelTypeDataArray[SampleOreTypeDataIndex];
						}
						else
						{
							if (SampleCavesValue3D > CavesThreshold + DefaultToWeakWidth) // If outside default layer
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
				
				int32 SampleArrayIndex3D = UATWorldFunctionLibrary::Point3D_To_ArrayIndex3D(SamplePoint, ChunkSize);
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
				FIntVector SamplePoint = ChunkBackLeftCornerPoint + UATWorldFunctionLibrary::ArrayIndex3D_To_Point3D(SampleArrayIndex, ChunkSize);
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
