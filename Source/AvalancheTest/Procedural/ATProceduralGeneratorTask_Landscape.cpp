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
void UATProceduralGeneratorTask_Landscape::Initialize(class UATProceduralGeneratorComponent* InOwnerComponent, int32 InTaskIndex) // UATProceduralGeneratorTask
{
	Super::Initialize(InOwnerComponent, InTaskIndex);

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

void UATProceduralGeneratorTask_Landscape::DoWorkGlobalOnce_SubThread() // UATProceduralGeneratorTask
{
	int32 TreeSeed = OwnerTree->GetTreeSeed();
	FIntVector TreeBoundsSize = OwnerTree->GetBoundsSize();

	HillsBoundsSize2D = FIntPoint(TreeBoundsSize.X, TreeBoundsSize.Y)

	ensure(HillsValues2D.IsEmpty());
	PerlinGenerator->GenUniformGrid2D(HillsValues2D, FIntPoint::ZeroValue, HillsBoundsSize2D, HillsNoiseFrequency, TreeSeed);

	for (int32 SampleArrayIndex2D = 0; SampleArrayIndex2D < HillsValues2D.Num(); ++SampleArrayIndex2D)
	{
		// Normalize
		float NewValue = (HillsValues2D[SampleArrayIndex2D] + 1.0f) * 0.5f;;

		// Power
		NewValue = FMath::Pow(NewValue, HillsHeightPow);

		// Offset
		NewValue = FMath::Lerp(HillsHeightOffset, 1.0f, NewValue);

		HillsValues2D[SampleArrayIndex2D] = NewValue;
	}
}

void UATProceduralGeneratorTask_Landscape::DoWorkForSelectedChunk_SubThread(const AATVoxelChunk* InTargetChunk) // UATProceduralGeneratorTask
{
	ensureReturn(PerChunkData.Contains(InTargetChunk));

	auto& Data = PerChunkData[InTargetChunk];
	Data.TypeDataArray.SetNum(FMath::Cube(InTargetChunk->GetChunkSize()));

	ensureReturn(OwnerTree);
	int32 TreeSeed = OwnerTree->GetTreeSeed();
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

	FIntVector GeneratorStart3D = ChunkBackLeftCornerPoint;
	FIntVector GeneratorSize3D = ChunkSize;

	ensure(Data.CavesNoiseValues3D.IsEmpty());
	PerlinGenerator->GenUniformGrid3D(Data.CavesNoiseValues3D, GeneratorStart3D, GeneratorSize3D, CavesNoiseFrequency, TreeSeed + 10);

	ensure(Data.OresNoiseValues3D.IsEmpty());
	PerlinGenerator->GenUniformGrid3D(Data.OresNoiseValues3D, GeneratorStart3D, GeneratorSize3D, OresNoiseFrequency, TreeSeed + 100);

	FIntVector TreeBoundsSize = OwnerTree->GetBoundsSize();
	int32 ChunkTreeMaxZ = TreeBoundsSize.Z;

	for (int32 LocalX = 0; LocalX < ChunkSize.X; ++LocalX)
	{
		for (int32 LocalY = 0; LocalY < ChunkSize.Y; ++LocalY)
		{
			FIntPoint LocalXY = FIntPoint(LocalX, LocalY);
			FIntPoint GlobalXY = FIntPoint(ChunkBackLeftCornerPoint.X, ChunkBackLeftCornerPoint.Y) + LocalXY;

			FRandomStream RandomStreamXY = FRandomStream(TreeSeed + GetTypeHash(GlobalXY));

			const int32 BottomBedrockWidth = FMath::CeilToInt((float)BedrockWidthVoxelsSidesBottom.Y * FMath::Lerp(0.5f, 1.0f, RandomStreamXY.GetFraction()));
			const float AboveHillsOresWidth = LERP_VECTOR2D(AboveHillsOresWidthMinMax, RandomStreamXY.GetFraction());

			FRandomStream RandomStreamX = FRandomStream(TreeSeed + GlobalXY.X * GlobalXY.X - GlobalXY.X % 43);
			FRandomStream RandomStreamY = FRandomStream(TreeSeed + GlobalXY.Y * GlobalXY.Y - GlobalXY.Y % 24);

			const int32 SideXBedrockWidth = FMath::CeilToInt((float)BedrockWidthVoxelsSidesBottom.X * FMath::Lerp(0.5f, 1.0f, RandomStreamY.GetFraction()));
			const int32 SideYBedrockWidth = FMath::CeilToInt((float)BedrockWidthVoxelsSidesBottom.X * FMath::Lerp(0.5f, 1.0f, RandomStreamX.GetFraction()));

			const FIntPoint LeftBackOffsetXY = GlobalXY;
			const FIntPoint RightFrontOffsetXY = FIntPoint(TreeBoundsSize.X, TreeBoundsSize.Y) - GlobalXY;

			float SampleHillsValue2D = GetHillsValueAtPoint2D(GlobalXY);

			for (int32 LocalZ = 0; LocalZ < ChunkSize.Z; ++LocalZ)
			{
				int32 GlobalZ = GeneratorStart3D.Z + LocalZ;
				float GlobalAlphaZ = float(GlobalZ) / float(ChunkTreeMaxZ);

				FRandomStream RandomStreamZ = FRandomStream(TreeSeed + GlobalZ);

				const bool bBelowHills = GlobalAlphaZ < SampleHillsValue2D;
				const bool bBelowStrongHillsLayer = (SampleHillsValue2D - GlobalAlphaZ) > StrongToWeakWidth;

				FIntVector SampleLocalPoint3D = FIntVector(LocalX, LocalY, LocalZ);
				int32 SampleArrayIndex3D = UATWorldFunctionLibrary::Point3D_To_ArrayIndex3D(SampleLocalPoint3D, ChunkSize);
				float SampleCavesValue3D = (Data.CavesNoiseValues3D[SampleArrayIndex3D] + 1.0f) * 0.5f; // Normalized

				const bool bInsideCave = SampleCavesValue3D < CavesNoiseThreshold;
				const bool bOutsideStrongCavesLayer = SampleCavesValue3D > (CavesNoiseThreshold + StrongToWeakWidth);

				float SampleOresValue3D = (Data.OresNoiseValues3D[SampleArrayIndex3D] + 1.0f) * 0.5f;
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
	}
	bPendingPostWork = true;
}

void UATProceduralGeneratorTask_Landscape::PostWork_GameThread()
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

			if (const UATVoxelTypeData* SampleTypeData = SampleData.TypeDataArray[SampleArrayIndex])
			{
				FIntVector SamplePoint = ChunkBackLeftCornerPoint + UATWorldFunctionLibrary::ArrayIndex3D_To_Point3D(SampleArrayIndex, ChunkSize);
				OwnerTree->SetVoxelAtPoint(SamplePoint, SampleTypeData, true);

				if (OwnerTree->IsThisTickUpdatesTimeBudgetExceeded())
				{
					break;
				}
			}
		}
		if (SampleData.LastProcessedIndex == SampleData.TypeDataArray.Num() - 1) // Accounts for empty array too
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

//~ Begin Data
float UATProceduralGeneratorTask_Landscape::GetHillsValueAtPoint2D(const FIntPoint& InPoint2D) const
{
	int32 SampleIndex2D = UATWorldFunctionLibrary::Point2D_To_ArrayIndex2D(InPoint2D, HillsBoundsSize2D);
	ensureReturn(HillsValues2D.IsValidIndex(SampleIndex2D), 0.0f);
	return HillsValues2D[SampleIndex2D];
}
//~ End Data
