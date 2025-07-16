// Scientific Ways

#include "Procedural/ATProceduralGeneratorTask_Landscape.h"

#include "World/ATVoxelTree.h"
#include "World/ATVoxelChunk.h"
#include "World/ATVoxelTypeData.h"
#include "World/ATWorldFunctionLibrary.h"

#include "FastNoise2/FastNoise2BlueprintLibrary.h"

UATProceduralGeneratorTask_Landscape::UATProceduralGeneratorTask_Landscape()
{
	StrongToWeakWidth = 0.02f;
	AboveHillsOresWidthMinMax = FVector2D(-0.01f, 0.02f);

	HillsNoiseFrequency = 0.01f;
	HillsHeightOffset = 0.5f;
	HillsHeightPow = 3.0f;

	CavesNoiseFrequency = 0.04f;
	CavesNoiseThreshold = 0.25f;

	OresNoiseFrequency = 0.05f;
	OresNoiseThreshold = 0.25f;

	BedrockNoiseFrequency = 0.01f;
	BedrockNoiseThreshold = 0.25f;
	BedrockWidthVoxelsSidesBottom = FIntPoint(16, 8);
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
	ensureReturn(StrongVoxelTypeData);
	ensureReturn(WeakVoxelTypeData);

	ensureReturn(BedrockVoxelTypeData);
	ensureReturn(BedrockVoxelTypeData->bIsUnbreakable);
	ensureReturn(BedrockVoxelTypeData->bHasInfiniteStability);

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
				Data.TypeDataArray[SampleArrayIndex] = StrongVoxelTypeData;
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

	FIntVector TreeBounds = TargetTree->GetBoundsSize();
	int32 ChunkTreeMaxZ = TreeBounds.Z;

	for (int32 SampleArrayIndex2D = 0; SampleArrayIndex2D < HillsValues2D.Num(); ++SampleArrayIndex2D)
	{
		FIntPoint LocalXY = UATWorldFunctionLibrary::ArrayIndex2D_To_Point2D(SampleArrayIndex2D, GeneratorSize2D);
		FIntPoint GlobalXY = GeneratorStart2D + LocalXY;

		FRandomStream RandomStreamXY = FRandomStream(TreeSeed + GetTypeHash(GlobalXY));

		const int32 BottomBedrockWidth = FMath::CeilToInt((float)BedrockWidthVoxelsSidesBottom.Y * FMath::Lerp(0.5f, 1.0f, RandomStreamXY.GetFraction()));
		const float AboveHillsOresWidth = LERP_VECTOR2D(AboveHillsOresWidthMinMax, RandomStreamXY.GetFraction());

		FRandomStream RandomStreamX = FRandomStream(TreeSeed + GlobalXY.X * GlobalXY.X - GlobalXY.X % 43);
		FRandomStream RandomStreamY = FRandomStream(TreeSeed + GlobalXY.Y * GlobalXY.Y - GlobalXY.Y % 24);

		const int32 SideXBedrockWidth = FMath::CeilToInt((float)BedrockWidthVoxelsSidesBottom.X * FMath::Lerp(0.5f, 1.0f, RandomStreamY.GetFraction()));
		const int32 SideYBedrockWidth = FMath::CeilToInt((float)BedrockWidthVoxelsSidesBottom.X * FMath::Lerp(0.5f, 1.0f, RandomStreamX.GetFraction()));

		const FIntPoint LeftBackOffsetXY = GlobalXY;
		const FIntPoint RightFrontOffsetXY = FIntPoint(TreeBounds.X, TreeBounds.Y) - GlobalXY;

		float SampleHillsValue2D = (HillsValues2D[SampleArrayIndex2D] + 1.0f) * 0.5f; // Normalized

		// Power
		SampleHillsValue2D = FMath::Pow(SampleHillsValue2D, HillsHeightPow);

		// Offset
		SampleHillsValue2D = FMath::Lerp(HillsHeightOffset, 1.0f, SampleHillsValue2D);

		for (int32 LocalZ = 0; LocalZ < ChunkSize.Z; ++LocalZ)
		{
			int32 GlobalZ = GeneratorStart3D.Z + LocalZ;
			float GlobalAlphaZ = float(GlobalZ) / float(ChunkTreeMaxZ);

			FRandomStream RandomStreamZ = FRandomStream(TreeSeed + GlobalZ);

			const bool bBelowHills = GlobalAlphaZ < SampleHillsValue2D;
			const bool bBelowStrongHillsLayer = (SampleHillsValue2D - GlobalAlphaZ) > StrongToWeakWidth;

			int32 SampleArrayIndex3D = UATWorldFunctionLibrary::ArrayIndex2D_To_ArrayIndex3D(SampleArrayIndex2D, LocalZ, ChunkSize);
			float SampleCavesValue3D = (CavesValues3D[SampleArrayIndex3D] + 1.0f) * 0.5f; // Normalized

			const bool bInsideCave = SampleCavesValue3D < CavesNoiseThreshold;
			const bool bOutsideStrongCavesLayer = SampleCavesValue3D > (CavesNoiseThreshold + StrongToWeakWidth);

			float SampleOresValue3D = (OresValues3D[SampleArrayIndex3D] + 1.0f) * 0.5f; 
			const bool bInsideOres = SampleOresValue3D < OresNoiseThreshold;

			const bool bInsideOresAboveHills = bInsideOres && (GlobalAlphaZ < (SampleHillsValue2D + AboveHillsOresWidth));

			const bool bInsideBottomBedrock = GlobalZ < BottomBedrockWidth;
			const bool bInsideSideXBedrock = bBelowHills && ((LeftBackOffsetXY.X < SideXBedrockWidth) || (RightFrontOffsetXY.X < SideXBedrockWidth));
			const bool bInsideSideYBedrock = bBelowHills && ((LeftBackOffsetXY.Y < SideYBedrockWidth) || (RightFrontOffsetXY.Y < SideYBedrockWidth));

			if (bInsideBottomBedrock || bInsideSideXBedrock || bInsideSideYBedrock)
			{
				Data.TypeDataArray[SampleArrayIndex3D] = BedrockVoxelTypeData;
			}
			else if (bInsideOresAboveHills)
			{
				goto ForceSetOre;
			}
			else if (bBelowHills)
			{
				if (bInsideCave)
				{
					Data.TypeDataArray[SampleArrayIndex3D] = nullptr;
				}
				else
				{
					if (bInsideOres)
					{
					ForceSetOre:
						float BelowHillsAlpha = (SampleHillsValue2D - GlobalAlphaZ) / SampleHillsValue2D;
						int32 SampleOreTypeDataIndex = FMath::RoundToInt(BelowHillsAlpha * (float)OresVoxelTypeDataArray.Num()) - 1;

						// Clamp
						SampleOreTypeDataIndex = FMath::Clamp(SampleOreTypeDataIndex, 0, OresVoxelTypeDataArray.Num() - 1);
						Data.TypeDataArray[SampleArrayIndex3D] = OresVoxelTypeDataArray[SampleOreTypeDataIndex];
					}
					else
					{
						if (bBelowStrongHillsLayer && bOutsideStrongCavesLayer)
						{
							Data.TypeDataArray[SampleArrayIndex3D] = WeakVoxelTypeData;
						}
						else
						{
							Data.TypeDataArray[SampleArrayIndex3D] = StrongVoxelTypeData;
						}
					}
				}
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
