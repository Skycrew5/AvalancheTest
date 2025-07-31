// Scientific Ways

#include "Simulation/ATSimulationTask_StabilityWaves.h"

#include "Simulation/ATSimulationComponent.h"
#include "Simulation/ATSimulationTask_Avalanche.h"

#include "World/ATVoxelTree.h"
#include "World/ATVoxelChunk.h"
#include "World/ATVoxelTypeData.h"
#include "World/ATWorldFunctionLibrary.h"

UATSimulationTask_StabilityWaves::UATSimulationTask_StabilityWaves()
{
	EmptyColorValue = 0u;
	DirtyColorValue = 1u;
	SupportedColorValue = 2u;
	UpdatedNeighborColorValue = 3u;

	BelowStabilityConveyMul = 0.98f;
	SideStabilityConveyMul = 0.24f;

	WaveIterationsHardLimit = 128;
	BoundingBoxAdditionalExtent = 64;

	AvalancheStabilityThreshold = 0.25f;
}

//~ Begin Initialize
void UATSimulationTask_StabilityWaves::Initialize(AATVoxelTree* InTargetTree) // UATSimulationTask
{
	Super::Initialize(InTargetTree);

	UATSimulationComponent* SimulationComponent = InTargetTree->GetSimulationComponent();
	ensureReturn(SimulationComponent);

	AvalancheSimulationTask = SimulationComponent->FindTaskInstance<UATSimulationTask_Avalanche>();
	ensureReturn(AvalancheSimulationTask);
}

void UATSimulationTask_StabilityWaves::DeInitialize() // UATSimulationTask
{
	Super::DeInitialize();


}
//~ End Initialize

//~ Begin Task
void UATSimulationTask_StabilityWaves::DoWork_SubThread() // UATSimulationTask
{
	InitDataForPoints(SelectedUpdatePoints);

	for (int32 SampleZ = BoundingBoxMin.Z; SampleZ <= BoundingBoxMax.Z; ++SampleZ)
	{
		TArray<FIntPoint> SupportedPoints2D;

		for (int32 SampleX = BoundingBoxMin.X; SampleX <= BoundingBoxMax.X; ++SampleX)
		{
			for (int32 SampleY = BoundingBoxMin.Y; SampleY <= BoundingBoxMax.Y; ++SampleY)
			{
				FIntVector SamplePoint = FIntVector(SampleX, SampleY, SampleZ);
				int32 SampleDataIndex = UATWorldFunctionLibrary::Point3D_To_ArrayIndex3D(SamplePoint - BoundingBoxMin, BoundingBoxSize);

				if (BoundingBoxDataMap[SamplePoint] == DirtyColorValue)
				{
					if (SampleZ > 0)
					{
						FIntVector BelowPoint = FIntVector(SampleX, SampleY, SampleZ - 1);

						float BelowStability = 0.0f;
						if (IsPointInBoundingBox(BelowPoint))
						{
							int32 BelowDataIndex = UATWorldFunctionLibrary::Point3D_To_ArrayIndex3D(BelowPoint - BoundingBoxMin, BoundingBoxSize);
							BelowStability = BoundingBoxDataMap[BelowPoint].StabilityValue;

							if (BelowStability > 0.0f)
							{
								BoundingBoxColorData[SampleDataIndex] = SupportedColorValue;
								BoundingBoxStabilityData[SampleDataIndex] = BelowStability * BelowStabilityConveyMul;
								SupportedPoints2D.Add(FIntPoint(SampleX, SampleY));
							}
						}
						else
						{
							ensureContinue(TargetTree);
							//BelowStability = TargetTree->GetVoxelInstanceDataAtPoint(BelowPoint, false, true).Stability;

							BoundingBoxColorData[SampleDataIndex] = SupportedColorValue;
							BoundingBoxStabilityData[SampleDataIndex] = 1.0f;
						}
					}
					else
					{
						BoundingBoxColorData[SampleDataIndex] = SupportedColorValue;
						BoundingBoxStabilityData[SampleDataIndex] = 1.0f;
					}
				}
			}
		}
		int32 CurrentIteration = 0;
		while (!SupportedPoints2D.IsEmpty() && CurrentIteration < WaveIterationsHardLimit)
		{
			++CurrentIteration;

			TArray<FIntPoint> Temp_SupportedPoints2D = SupportedPoints2D;
			SupportedPoints2D.Empty();

			for (const FIntPoint& SampleSupportedPoint2D : Temp_SupportedPoints2D)
			{
				FIntVector SampleSupportedPoint3D = FIntVector(SampleSupportedPoint2D.X, SampleSupportedPoint2D.Y, SampleZ);
				int32 SampleSupportedDataIndex = UATWorldFunctionLibrary::Point3D_To_ArrayIndex3D(SampleSupportedPoint3D - BoundingBoxMin, BoundingBoxSize);

				TArray<FIntVector> DirtyNeighbors3D;
				GetDirtyNeighbors(SampleSupportedPoint3D, DirtyNeighbors3D);

				for (const FIntVector& SampleDirtyNeighbor3D : DirtyNeighbors3D)
				{
					int32 SampleDirtyNeighborDataIndex = UATWorldFunctionLibrary::Point3D_To_ArrayIndex3D(SampleDirtyNeighbor3D - BoundingBoxMin, BoundingBoxSize);
					BoundingBoxStabilityData[SampleDirtyNeighborDataIndex] += BoundingBoxStabilityData[SampleSupportedDataIndex] * SideStabilityConveyMul;
				}
			}
			for (const FIntPoint& SampleSupportedPoint2D : Temp_SupportedPoints2D)
			{
				FIntVector SampleSupportedPoint3D = FIntVector(SampleSupportedPoint2D.X, SampleSupportedPoint2D.Y, SampleZ);
				int32 SampleSupportedDataIndex = UATWorldFunctionLibrary::Point3D_To_ArrayIndex3D(SampleSupportedPoint3D - BoundingBoxMin, BoundingBoxSize);

				TArray<FIntVector> DirtyNeighbors3D;
				GetDirtyNeighbors(SampleSupportedPoint3D, DirtyNeighbors3D);

				for (const FIntVector& SampleDirtyNeighbor3D : DirtyNeighbors3D)
				{
					int32 SampleDirtyNeighborDataIndex = UATWorldFunctionLibrary::Point3D_To_ArrayIndex3D(SampleDirtyNeighbor3D - BoundingBoxMin, BoundingBoxSize);
					BoundingBoxColorData[SampleDirtyNeighborDataIndex] = UpdatedNeighborColorValue;

					if (!SupportedPoints2D.Contains(FIntPoint(SampleDirtyNeighbor3D.X, SampleDirtyNeighbor3D.Y)))
					{
						SupportedPoints2D.Add(FIntPoint(SampleDirtyNeighbor3D.X, SampleDirtyNeighbor3D.Y));
					}
				}
			}
		}
	}
	bPendingPostWork = true;
}

void UATSimulationTask_StabilityWaves::PostWork_GameThread()
{
	ensureReturn(TargetTree);

	for (int32 SampleX = BoundingBoxMin.X; SampleX <= BoundingBoxMax.X; ++SampleX)
	{
		for (int32 SampleY = BoundingBoxMin.Y; SampleY <= BoundingBoxMax.Y; ++SampleY)
		{
			for (int32 SampleZ = BoundingBoxMin.Z; SampleZ <= BoundingBoxMax.Z; ++SampleZ)
			{
				FIntVector SamplePoint = FIntVector(SampleX, SampleY, SampleZ);
				if (!TargetTree->HasVoxelInstanceDataAtPoint(SamplePoint, false))
				{
					continue;
				}
				FVoxelInstanceData& SampleData = TargetTree->GetVoxelInstanceDataAtPoint(SamplePoint, false);
				float PrevStability = SampleData.Stability;

				int32 SampleDataIndex = UATWorldFunctionLibrary::Point3D_To_ArrayIndex3D(SamplePoint - BoundingBoxMin, BoundingBoxSize);
				SampleData.Stability = BoundingBoxStabilityData[SampleDataIndex];

				AATVoxelChunk* SampleChunk = TargetTree->GetVoxelChunkAtPoint(SamplePoint);
				ensureContinue(SampleChunk);

				//SampleChunk->HandleSetVoxelStabilityAtPoint(SamplePoint, SampleData.Stability);
				SampleChunk->HandleSetVoxelInstanceDataAtPoint(SamplePoint, SampleData);

				if (SampleData.Stability <= AvalancheStabilityThreshold)
				{
					ensureContinue(AvalancheSimulationTask);
					AvalancheSimulationTask->QueuePoint(SamplePoint, false);
				}
			}
		}
	}
	SelectedUpdatePoints.Empty();

	if (SelectedUpdatePoints.IsEmpty())
	{
		FinishPostWork_GameThread();
	}
}
//~ End Task

//~ Begin Utils
bool UATSimulationTask_StabilityWaves::IsPointInBoundingBox(const FIntVector& InPoint) const
{
	return InPoint.X >= BoundingBoxMin.X && InPoint.X <= BoundingBoxMax.X
		&& InPoint.Y >= BoundingBoxMin.Y && InPoint.Y <= BoundingBoxMax.Y
		&& InPoint.Z >= BoundingBoxMin.Z && InPoint.Z <= BoundingBoxMax.Z;
}

void UATSimulationTask_StabilityWaves::InitDataForPoints(const TArray<FIntVector>& InPoints)
{
	// Bounding box
	{
		ensureReturn(!InPoints.IsEmpty());

		FIntVector NewMin = InPoints[0];
		FIntVector NewMax = InPoints[0];

		for (const FIntVector& SamplePoint : InPoints)
		{
			NewMin = FScWMath::MinIntVector(NewMin, SamplePoint);
			NewMax = FScWMath::MaxIntVector(NewMax, SamplePoint);
		}
		ensureReturn(TargetTree);
		const FIntVector& TreeBoundsMax = TargetTree->GetBoundsSize() - FIntVector(1, 1, 1);

		BoundingBoxMin = FScWMath::ClampIntVector(NewMin - FIntVector(BoundingBoxAdditionalExtent), FIntVector::ZeroValue, TreeBoundsMax);
		BoundingBoxMax = FScWMath::ClampIntVector(NewMax + FIntVector(BoundingBoxAdditionalExtent), FIntVector::ZeroValue, TreeBoundsMax);

		BoundingBoxSize = BoundingBoxMax - BoundingBoxMin + FIntVector(1, 1, 1);
		ensureReturn(BoundingBoxSize.X > 0 && BoundingBoxSize.Y > 0 && BoundingBoxSize.Z > 0);
	}
	// Data arrays
	{
		int32 NewDataArraySize = BoundingBoxSize.X * BoundingBoxSize.Y * BoundingBoxSize.Z;
		ensureReturn(NewDataArraySize > 0);

		BoundingBoxColorData.SetNum(NewDataArraySize);
		BoundingBoxStabilityData.SetNum(NewDataArraySize);

		ParallelFor(BoundingBoxSize.X, [&](int32 SampleLocalX)
		{
			for (int32 SampleLocalY = 0; SampleLocalY < BoundingBoxSize.Y; ++SampleLocalY)
			{
				for (int32 SampleLocalZ = 0; SampleLocalZ < BoundingBoxSize.Z; ++SampleLocalZ)
				{
					FIntVector SampleLocalPoint = FIntVector(SampleLocalX, SampleLocalY, SampleLocalZ);
					int32 SampleDataIndex = UATWorldFunctionLibrary::Point3D_To_ArrayIndex3D(SampleLocalPoint, BoundingBoxSize);

					if (TargetTree->HasVoxelInstanceDataAtPoint(BoundingBoxMin + SampleLocalPoint, true))
					{
						BoundingBoxColorData[SampleDataIndex] = DirtyColorValue;
					}
					else
					{
						BoundingBoxColorData[SampleDataIndex] = EmptyColorValue;
					}
					BoundingBoxStabilityData[SampleDataIndex] = 0.0f;
				}
			}
			//}, EParallelForFlags::ForceSingleThread);
		});
	}
}

void UATSimulationTask_StabilityWaves::GetDirtyNeighbors(const FIntVector& InPoint, TArray<FIntVector>& OutDirtyNeighbors)
{
	ensureReturn(OutDirtyNeighbors.IsEmpty());

	static const TArray<FIntVector> NeighborsOffsets = {
		FIntVector(1, 0, 0), FIntVector(-1, 0, 0),
		FIntVector(0, 1, 0), FIntVector(0, -1, 0)
		};
	
	for (const FIntVector& SampleOffset : NeighborsOffsets)
	{
		const FIntVector SampleNeighborPoint = InPoint + SampleOffset;

		if (IsPointInBoundingBox(SampleNeighborPoint))
		{
			const int32 SampleNeighborDataIndex = UATWorldFunctionLibrary::Point3D_To_ArrayIndex3D(SampleNeighborPoint - BoundingBoxMin, BoundingBoxSize);

			if (BoundingBoxColorData[SampleNeighborDataIndex] == DirtyColorValue)
			{
				OutDirtyNeighbors.Add(SampleNeighborPoint);
			}
		}
	}
}
//~ End Utils
