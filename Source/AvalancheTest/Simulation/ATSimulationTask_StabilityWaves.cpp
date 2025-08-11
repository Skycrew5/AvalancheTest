// Scientific Ways

#include "Simulation/ATSimulationTask_StabilityWaves.h"

#include "Simulation/ATSimulationComponent.h"
#include "Simulation/ATSimulationTask_Avalanche.h"

#include "World/ATVoxelTree.h"
#include "World/ATVoxelChunk.h"
#include "World/ATVoxelTypeData.h"

UATSimulationTask_StabilityWaves::UATSimulationTask_StabilityWaves()
{
	EmptyColorValue = 0u;
	DirtyColorValue = 1u;
	SupportedColorValue = 2u;
	UpdatedNeighborColorValue = 3u;

	BelowStabilityConveyMul = 1.0f;
	SideStabilityConveyMul = 0.24f;

	WaveIterationsHardLimit = 128;
	BoundingBoxAdditionalExtent = 8;

	AvalancheMassThreshold = 4.0f;
	AvalancheValueThreshold = 0.25f;
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

	BoundingBoxDataMap.GenerateKeyArray(SelectedUpdatePoints);

	for (int32 SampleZ = BoundingBoxMin.Z; SampleZ <= BoundingBoxMax.Z; ++SampleZ)
	{
		TArray<FIntPoint> SupportedPoints2D;

		for (int32 SampleX = BoundingBoxMin.X; SampleX <= BoundingBoxMax.X; ++SampleX)
		{
			for (int32 SampleY = BoundingBoxMin.Y; SampleY <= BoundingBoxMax.Y; ++SampleY)
			{
				FIntVector SamplePoint = FIntVector(SampleX, SampleY, SampleZ);

				if (!BoundingBoxDataMap.Contains(SamplePoint))
				{
					continue;
				}
				if (BoundingBoxDataMap[SamplePoint].ColorValue == DirtyColorValue)
				{
					if (SampleZ > 0)
					{
						FIntVector BelowPoint = FIntVector(SampleX, SampleY, SampleZ - 1);

						float BelowStability = 0.0f;
						if (IsPointInBoundingBox(BelowPoint))
						{
							if (BoundingBoxDataMap.Contains(BelowPoint))
							{
								BelowStability = BoundingBoxDataMap[BelowPoint].StabilityValue;

								if (BelowStability > 0.0f)
								{
									BoundingBoxDataMap[SamplePoint].ColorValue = SupportedColorValue;
									BoundingBoxDataMap[SamplePoint].StabilityValue = BelowStability * BelowStabilityConveyMul;
									SupportedPoints2D.Add(FIntPoint(SampleX, SampleY));
								}
							}
						}
						else
						{
							ensureContinue(TargetTree);
							//BelowStability = TargetTree->GetVoxelInstanceDataAtPoint(BelowPoint, false, true).AvalancheValue;

							BoundingBoxDataMap[SamplePoint].ColorValue = SupportedColorValue;
							BoundingBoxDataMap[SamplePoint].StabilityValue = 1.0f;
						}
					}
					else
					{
						BoundingBoxDataMap[SamplePoint].ColorValue = SupportedColorValue;
						BoundingBoxDataMap[SamplePoint].StabilityValue = 1.0f;
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

				TArray<FIntVector> DirtyNeighbors3D;
				GetDirtyNeighbors(SampleSupportedPoint3D, DirtyNeighbors3D);

				for (const FIntVector& SampleDirtyNeighbor3D : DirtyNeighbors3D)
				{
					BoundingBoxDataMap[SampleDirtyNeighbor3D].StabilityValue += BoundingBoxDataMap[SampleSupportedPoint3D].StabilityValue * SideStabilityConveyMul;
				}
			}
			for (const FIntPoint& SampleSupportedPoint2D : Temp_SupportedPoints2D)
			{
				FIntVector SampleSupportedPoint3D = FIntVector(SampleSupportedPoint2D.X, SampleSupportedPoint2D.Y, SampleZ);

				TArray<FIntVector> DirtyNeighbors3D;
				GetDirtyNeighbors(SampleSupportedPoint3D, DirtyNeighbors3D);

				for (const FIntVector& SampleDirtyNeighbor3D : DirtyNeighbors3D)
				{
					BoundingBoxDataMap[SampleDirtyNeighbor3D].ColorValue = UpdatedNeighborColorValue;

					if (!SupportedPoints2D.Contains(FIntPoint(SampleDirtyNeighbor3D.X, SampleDirtyNeighbor3D.Y)))
					{
						SupportedPoints2D.Add(FIntPoint(SampleDirtyNeighbor3D.X, SampleDirtyNeighbor3D.Y));
					}
				}
			}
		}
		UE_LOG(LogTemp, Verbose, TEXT("StabilityWaves: Z = %d, Iterations = %d"), SampleZ, CurrentIteration);
	}
	for (int32 SampleZ = BoundingBoxMin.Z; SampleZ <= BoundingBoxMax.Z; ++SampleZ)
	{
		struct FStabilityPointData
		{
			float StabilityValue;
			FIntPoint PointXY;

			bool operator<(const FStabilityPointData& InOther) const { return StabilityValue < InOther.StabilityValue; }
		};
		TArray<FStabilityPointData> StabilitySortedPointsXY;

		for (int32 SampleX = BoundingBoxMin.X; SampleX <= BoundingBoxMax.X; ++SampleX)
		{
			for (int32 SampleY = BoundingBoxMin.Y; SampleY <= BoundingBoxMax.Y; ++SampleY)
			{
				FIntVector SamplePoint = FIntVector(SampleX, SampleY, SampleZ);

				if (!BoundingBoxDataMap.Contains(SamplePoint))
				{
					continue;
				}
				BoundingBoxDataMap[SamplePoint].MassValue = 1.0f;
				StabilitySortedPointsXY.Add(FStabilityPointData(BoundingBoxDataMap[SamplePoint].StabilityValue, FIntPoint(SampleX, SampleY)));
			}
		}
		StabilitySortedPointsXY.Sort();

		for (const FStabilityPointData& SampleStabilityPointXY : StabilitySortedPointsXY) // For each point in XY plane, we will distribute its mass to neighbors
		{
			const FIntVector& SamplePoint = FIntVector(SampleStabilityPointXY.PointXY.X, SampleStabilityPointXY.PointXY.Y, SampleZ);

			TArray<FIntVector> AttachmentNeighbors3D;
			float SampleTotalStability = GetAttachmentNeighbors(SamplePoint, AttachmentNeighbors3D);

			for (const FIntVector& SampleAttachmentNeighbor3D : AttachmentNeighbors3D)
			{
				float SampleMassFraction = BoundingBoxDataMap[SampleAttachmentNeighbor3D].StabilityValue / SampleTotalStability;
				BoundingBoxDataMap[SampleAttachmentNeighbor3D].MassValue += BoundingBoxDataMap[SamplePoint].MassValue * SampleMassFraction;
			}
		}
		for (const FStabilityPointData& SampleStabilityPointXY : StabilitySortedPointsXY)
		{
			const FIntVector& SamplePoint = FIntVector(SampleStabilityPointXY.PointXY.X, SampleStabilityPointXY.PointXY.Y, SampleZ);

			float AvalancheThresholdMul = 1.0f;

			/*if (SamplePoint.Z == BoundingBoxMin.Z || BoundingBoxDataMap.Contains(SamplePoint + FIntVector(0, 0, -1))) // Supported voxel
			{
				if (BoundingBoxDataMap.Contains(SamplePoint + FIntVector(-1, 0, 0)) &&
					BoundingBoxDataMap.Contains(SamplePoint + FIntVector(1, 0, 0)) &&
					BoundingBoxDataMap.Contains(SamplePoint + FIntVector(0, -1, 0)) &&
					BoundingBoxDataMap.Contains(SamplePoint + FIntVector(0, 1, 0))) // Fully supported voxel
				{
					AvalancheThresholdMul = 100.0f;
				}
				else // Partially supported voxel
				{
					AvalancheThresholdMul = 30.0f;
				}
			}
			else // Loose voxel
			{
				if (BoundingBoxDataMap.Contains(SamplePoint + FIntVector(-1, 0, 0)) ||
					BoundingBoxDataMap.Contains(SamplePoint + FIntVector(1, 0, 0)) ||
					BoundingBoxDataMap.Contains(SamplePoint + FIntVector(0, -1, 0)) ||
					BoundingBoxDataMap.Contains(SamplePoint + FIntVector(0, 1, 0))) // Somehow supported voxel
				{
					AvalancheThresholdMul = 2.0f;
				}
			}*/
			BoundingBoxDataMap[SamplePoint].AvalancheValue = BoundingBoxDataMap[SamplePoint].MassValue / (AvalancheMassThreshold * AvalancheThresholdMul);
		}
	}
	bPendingPostWork = true;
}

void UATSimulationTask_StabilityWaves::PostWork_GameThread()
{
	ensureReturn(TargetTree);
	while (!TargetTree->IsThisTickUpdatesTimeBudgetExceeded() && !SelectedUpdatePoints.IsEmpty())
	{
		FIntVector SamplePoint = SelectedUpdatePoints.Pop();

		FVoxelInstanceData& SampleData = TargetTree->GetVoxelInstanceDataAtPoint(SamplePoint, false);
		float PrevStability = SampleData.AvalancheValue;

		ensureContinue(BoundingBoxDataMap.Contains(SamplePoint));
		SampleData.AvalancheValue = BoundingBoxDataMap[SamplePoint].AvalancheValue;

		AATVoxelChunk* SampleChunk = TargetTree->GetVoxelChunkAtPoint(SamplePoint);
		ensureContinue(SampleChunk);

		//SampleChunk->HandleSetVoxelStabilityAtPoint(SamplePoint, SampleData.AvalancheValue);
		SampleChunk->HandleSetVoxelInstanceDataAtPoint(SamplePoint, SampleData);

		if (SampleData.AvalancheValue <= AvalancheValueThreshold)
		{
			ensureContinue(AvalancheSimulationTask);
			AvalancheSimulationTask->QueuePoint(SamplePoint, false);
		}
	}
	if (SelectedUpdatePoints.IsEmpty())
	{
		BoundingBoxDataMap.Empty();
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
	ensureReturn(TargetTree);

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

		ensure(BoundingBoxDataMap.IsEmpty());

		//ParallelFor(BoundingBoxSize.X, [&](int32 SampleLocalX)
		for (int32 SampleX = BoundingBoxMin.X; SampleX <= BoundingBoxMax.X; ++SampleX)
		{
			for (int32 SampleY = BoundingBoxMin.Y; SampleY <= BoundingBoxMax.Y; ++SampleY)
			{
				for (int32 SampleZ = BoundingBoxMin.Z; SampleZ <= BoundingBoxMax.Z; ++SampleZ)
				{
					FIntVector SamplePoint = FIntVector(SampleX, SampleY, SampleZ);
					//const FVoxelInstanceData& SampleData = TargetTree->GetVoxelInstanceDataAtPoint(SamplePoint, false, true);

					//if (SampleData.AvalancheValue > 0.0f)
					if (TargetTree->HasVoxelInstanceDataAtPoint(SamplePoint, true))
					{
						BoundingBoxDataMap.Add(SamplePoint, FBoundingBoxData(DirtyColorValue, 0.0f, 1.0f, 0.0f));
					}
				}
			}
			//}, EParallelForFlags::ForceSingleThread);
		}//);
	}
}

void UATSimulationTask_StabilityWaves::GetDirtyNeighbors(const FIntVector& InPoint, TArray<FIntVector>& OutDirtyNeighbors) const
{
	ensureReturn(OutDirtyNeighbors.IsEmpty());

	static const TArray<FIntVector> NeighborsOffsets = {
		FIntVector(1, 0, 0), FIntVector(-1, 0, 0),
		FIntVector(0, 1, 0), FIntVector(0, -1, 0)
		};
	
	for (const FIntVector& SampleOffset : NeighborsOffsets)
	{
		const FIntVector SampleNeighborPoint = InPoint + SampleOffset;

		if (BoundingBoxDataMap.Contains(SampleNeighborPoint))
		{
			if (BoundingBoxDataMap[SampleNeighborPoint].ColorValue == DirtyColorValue)
			{
				OutDirtyNeighbors.Add(SampleNeighborPoint);
			}
		}
	}
}

float UATSimulationTask_StabilityWaves::GetAttachmentNeighbors(const FIntVector& InPoint, TArray<FIntVector>& OutAttachmentNeighbors) const
{
	ensureReturn(OutAttachmentNeighbors.IsEmpty(), 0.0f);

	static const TArray<FIntVector> NeighborsOffsets = {
		FIntVector(1, 0, 0), FIntVector(-1, 0, 0),
		FIntVector(0, 1, 0), FIntVector(0, -1, 0),
		FIntVector(0, 0, -1)
	};

	float OutTotalStability = 0.0f;
	for (const FIntVector& SampleOffset : NeighborsOffsets)
	{
		const FIntVector SampleNeighborPoint = InPoint + SampleOffset;

		if (BoundingBoxDataMap.Contains(SampleNeighborPoint))
		{
			if (BoundingBoxDataMap[SampleNeighborPoint].StabilityValue > BoundingBoxDataMap[InPoint].StabilityValue)
			{
				OutAttachmentNeighbors.Add(SampleNeighborPoint);
				OutTotalStability += BoundingBoxDataMap[SampleNeighborPoint].StabilityValue;
			}
		}
	}
	return OutTotalStability;
}
//~ End Utils
