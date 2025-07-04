// Scientific Ways

#include "World/ATVoxelTree.h"

#include "World/ATVoxelChunk.h"
#include "World/ATVoxelTypeData.h"

#if DEBUG_VOXELS
	#pragma optimize("", off)
#endif // DEBUG_VOXELS

AATVoxelTree::AATVoxelTree(const FObjectInitializer& InObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	ChunkClass = AATVoxelChunk::StaticClass();
	TreeSizeInChunks = FIntVector(4, 4, 2);
	ChunkSize = 16;
	VoxelSize = 16.0f;

	MaxUpdatesPerSecond = 10000;
	bEnableVoxelDataUpdatesTick = true;
}

//~ Begin Initialize
void AATVoxelTree::PostInitializeComponents() // AActor
{
	Super::PostInitializeComponents();

	
}

void AATVoxelTree::OnConstruction(const FTransform& InTransform) // AActor
{
	Super::OnConstruction(InTransform);


}

void AATVoxelTree::BeginPlay() // AActor
{
	Super::BeginPlay();

	InitVoxelChunks();
}

void AATVoxelTree::Tick(float InDeltaSeconds) // AActor
{
	Super::Tick(InDeltaSeconds);

	int32 UpdatesLeft = FMath::CeilToInt((float)MaxUpdatesPerSecond * InDeltaSeconds);

	if (bEnableVoxelDataUpdatesTick)
	{
		HandleVoxelDataUpdatesTick(UpdatesLeft);
	}


	TArray<FIntVector> MostRelevantChunkCoords;
	//GetMostRelevantChunksForTick(MostRelevantChunks);
	ChunksMap.GenerateKeyArray(MostRelevantChunkCoords);

	for (const FIntVector& SampleChunkCoords : MostRelevantChunkCoords)
	{
		AATVoxelChunk* SampleChunk = GetVoxelChunkAtCoords(SampleChunkCoords);
		ensureContinue(SampleChunk);
		SampleChunk->HandleUpdates(UpdatesLeft);
	}
}

void AATVoxelTree::EndPlay(const EEndPlayReason::Type InReason) // AActor
{
	Super::EndPlay(InReason);


}
//~ End Initialize

//~ Begin Voxel Chunks
FIntVector AATVoxelTree::GetVoxelChunkCoordsAtPoint(const FIntVector& InPoint) const
{
	return FIntVector(InPoint.X / ChunkSize, InPoint.Y / ChunkSize, InPoint.Z / ChunkSize);
}

void AATVoxelTree::InitVoxelChunks()
{
	UWorld* World = GetWorld();
	ensureReturn(World);

	ensureReturn(!TreeSizeInChunks.IsZero());
	ensureReturn(ChunkSize > 0);
	ensureReturn(VoxelSize > 0.0f);

	for (int32 SampleX = 0; SampleX < TreeSizeInChunks.X; ++SampleX)
	{
		for (int32 SampleY = 0; SampleY < TreeSizeInChunks.Y; ++SampleY)
		{
			for (int32 SampleZ = 0; SampleZ < TreeSizeInChunks.Z; ++SampleZ)
			{
				FIntVector SamplePoint = FIntVector(SampleX, SampleY, SampleZ);

				FTransform SampleTransform = FTransform(FVector(SamplePoint * ChunkSize) * VoxelSize);
				AATVoxelChunk* SampleChunk = World->SpawnActorDeferred<AATVoxelChunk>(
					ChunkClass,
					SampleTransform,
					this,
					nullptr,
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
					ESpawnActorScaleMethod::MultiplyWithRoot
				);
				ensureContinue(SampleChunk);

				ensureContinue(!ChunksMap.Contains(SamplePoint));
				ChunksMap.Add(SamplePoint, SampleChunk);

				SampleChunk->BP_InitChunk(this, SamplePoint);

				if (SamplePoint.Z == 0)
				{
					CreateFoundationAtChunk(SamplePoint);
				}
				SampleChunk->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
				SampleChunk->FinishSpawning(SampleTransform);
			}
		}
	}
}
//~ End Voxel Chunks

//~ Begin Voxel Getters
FVoxelInstanceData& AATVoxelTree::GetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const bool bInChecked) const
{
	if (const FVoxelInstanceData* SampleData = Point_To_VoxelInstanceData_Map.Find(InPoint))
	{
		return const_cast<FVoxelInstanceData&>(*SampleData);
	}
	else
	{
		ensure(!bInChecked);
		return const_cast<FVoxelInstanceData&>(FVoxelInstanceData::Invalid);
	}
}

int32 AATVoxelTree::GetVoxelNeighborsNumAtPoint(const FIntVector& InPoint) const
{
	int32 OutNum = 0;

	if (HasVoxelInstanceDataAtPoint(InPoint + FIntVector(1, 0, 0))) OutNum += 1;
	if (HasVoxelInstanceDataAtPoint(InPoint + FIntVector(-1, 0, 0))) OutNum += 1;
	if (HasVoxelInstanceDataAtPoint(InPoint + FIntVector(0, 1, 0))) OutNum += 1;
	if (HasVoxelInstanceDataAtPoint(InPoint + FIntVector(0, -1, 0))) OutNum += 1;
	if (HasVoxelInstanceDataAtPoint(InPoint + FIntVector(0, 0, 1))) OutNum += 1;
	if (HasVoxelInstanceDataAtPoint(InPoint + FIntVector(0, 0, -1))) OutNum += 1;
	return OutNum;
}

void AATVoxelTree::GetAllVoxelPointsInRadius(const FIntVector& InCenterPoint, int32 InRadius, TArray<FIntVector>& OutPoints) const
{
	ensure(InRadius > 0);

	for (int32 OffsetX = -InRadius; OffsetX < InRadius + 1; ++OffsetX)
	{
		float Alpha = FMath::Abs((float)OffsetX) / (float)InRadius;
		int32 SliceRadius = FMath::Max(FMath::CeilToInt32((float)InRadius * (1.0f - FMath::Square(Alpha))), 1);

		for (int32 OffsetY = -SliceRadius; OffsetY < SliceRadius + 1; ++OffsetY)
		{
			float Alpha2 = FMath::Abs((float)OffsetY) / (float)SliceRadius;
			int32 SliceRadius2 = FMath::Max(FMath::CeilToInt32((float)SliceRadius * (1.0f - FMath::Square(Alpha2))), 1);

			for (int32 OffsetZ = -SliceRadius2; OffsetZ < SliceRadius2 + 1; ++OffsetZ)
			{
				OutPoints.Add(InCenterPoint + FIntVector(OffsetX, OffsetY, OffsetZ));
			}
		}
	}
}

bool AATVoxelTree::CanBreakVoxelAtPoint(const FIntVector& InPoint) const
{
	if (HasVoxelInstanceDataAtPoint(InPoint))
	{
		FVoxelInstanceData& Data = GetVoxelInstanceDataAtPoint(InPoint);
		ensureReturn(Data.IsTypeDataValid(), false);
		return !Data.TypeData->bIsFoundation;
	}
	return false;
}

bool AATVoxelTree::IsPointInsideTree(const FIntVector& InPoint) const
{
	return (InPoint.X >= 0 && InPoint.X < BoundsSize.X)
		&& (InPoint.Y >= 0 && InPoint.Y < BoundsSize.Y)
		&& (InPoint.Z >= 0 && InPoint.Z < BoundsSize.Z);
}
//~ End Voxel Getters

//~ Begin Voxel Setters
bool AATVoxelTree::SetVoxelAtPoint(const FIntVector& InPoint, const UATVoxelTypeData* InTypeData, const bool bInForced)
{
	ensureReturn(InTypeData, false);

	if (bInForced)
	{
		BreakVoxelAtPoint(InPoint, true);
	}
	else if (HasVoxelInstanceDataAtPoint(InPoint))
	{
		return false;
	}
	if (AATVoxelChunk* SampleChunk = GetVoxelChunkAtPoint(InPoint))
	{
		ensure(!Point_To_VoxelInstanceData_Map.Contains(InPoint));
		FVoxelInstanceData& NewInstanceData = Point_To_VoxelInstanceData_Map.Add(InPoint, InTypeData->BP_InitializeInstanceData(this, InPoint));
		
		SampleChunk->HandleSetVoxelInstanceDataAtPoint(InPoint, NewInstanceData);

		QueueRecursiveStabilityUpdate(InPoint);
		return true;
	}
	//ensureReturn(false, false);
	return false;
}

bool AATVoxelTree::SetVoxelsAtPoints(const TArray<FIntVector>& InPoints, const UATVoxelTypeData* InTypeData, const bool bInForced)
{
	bool bAnySet = false;

	for (const FIntVector& SamplePoint : InPoints)
	{
		bAnySet |= SetVoxelAtPoint(SamplePoint, InTypeData, bInForced);
	}
	return bAnySet;
}

bool AATVoxelTree::BreakVoxelAtPoint(const FIntVector& InPoint, const bool bInForced, const bool bInNotify)
{
	if (!bInForced && !CanBreakVoxelAtPoint(InPoint))
	{
		return false;
	}
	if (AATVoxelChunk* SampleChunk = GetVoxelChunkAtPoint(InPoint))
	{
		if (HasVoxelInstanceDataAtPoint(InPoint))
		{
			SampleChunk->HandleBreakVoxelAtPoint(InPoint, bInNotify);
		}
		Point_To_VoxelInstanceData_Map.Remove(InPoint);
		QueueRecursiveStabilityUpdate(InPoint);
		return true;
	}
	return false;
}

bool AATVoxelTree::BreakVoxelsAtPoints(const TArray<FIntVector>& InPoints, const bool bInForced, const bool bInNotify)
{
	bool bAnyBroken = false;

	for (const FIntVector& SamplePoint : InPoints)
	{
		bAnyBroken |= BreakVoxelAtPoint(SamplePoint, bInForced, bInNotify);
	}
	return bAnyBroken;
}

void AATVoxelTree::FillChunkWithVoxels(const FIntVector& InChunkCoords, const UATVoxelTypeData* InTypeData, const bool bInForced)
{
	FIntVector ChunkOffset = InChunkCoords * ChunkSize;
	TArray<FIntVector> VoxelPoints;

	for (int32 SampleX = 0; SampleX < ChunkSize; ++SampleX)
	{
		for (int32 SampleY = 0; SampleY < ChunkSize; ++SampleY)
		{
			for (int32 SampleZ = 0; SampleZ < ChunkSize; ++SampleZ)
			{
				VoxelPoints.Add(ChunkOffset + FIntVector(SampleX, SampleY, SampleZ));
			}
		}
	}
	SetVoxelsAtPoints(VoxelPoints, InTypeData, bInForced);
}

void AATVoxelTree::BreakAllVoxelsAtChunk(const FIntVector& InChunkCoords, const bool bInForced)
{
	FIntVector ChunkOffset = InChunkCoords * ChunkSize;
	TArray<FIntVector> VoxelPoints;

	for (int32 SampleX = 0; SampleX < ChunkSize; ++SampleX)
	{
		for (int32 SampleY = 0; SampleY < ChunkSize; ++SampleY)
		{
			for (int32 SampleZ = 0; SampleZ < ChunkSize; ++SampleZ)
			{
				VoxelPoints.Add(ChunkOffset + FIntVector(SampleX, SampleY, SampleZ));
			}
		}
	}
	BreakVoxelsAtPoints(VoxelPoints, bInForced);
}

void AATVoxelTree::CreateFoundationAtChunk(const FIntVector& InChunkCoords)
{
	ensureReturn(FoundationVoxelTypeData);
	ensureReturn(FoundationVoxelTypeData->bIsFoundation);

	FIntVector ChunkOffset = InChunkCoords * ChunkSize;
	TArray<FIntVector> VoxelPoints;

	for (int32 SampleX = 0; SampleX < ChunkSize; ++SampleX)
	{
		for (int32 SampleY = 0; SampleY < ChunkSize; ++SampleY)
		{
			VoxelPoints.Add(ChunkOffset + FIntVector(SampleX, SampleY, 0));
		}
	}
	SetVoxelsAtPoints(VoxelPoints, FoundationVoxelTypeData, true);
}
//~ End Voxel Setters

//~ Begin Voxel Data
void AATVoxelTree::QueueFullUpdateAtChunk(const FIntVector& InChunkCoords)
{
	FIntVector ChunkOffset = InChunkCoords * ChunkSize;
	TArray<FIntVector> VoxelPoints;

	for (int32 SampleX = 0; SampleX < ChunkSize; ++SampleX)
	{
		for (int32 SampleY = 0; SampleY < ChunkSize; ++SampleY)
		{
			for (int32 SampleZ = 0; SampleZ < ChunkSize; ++SampleZ)
			{
				QueueRecursiveStabilityUpdate(ChunkOffset + FIntVector(SampleX, SampleY, SampleZ));
			}
		}
	}
}

void AATVoxelTree::HandleVoxelDataUpdatesTick(int32& InOutUpdatesLeft)
{
	//InOutMaxUpdates -= UpdatePendingAttachmentData(InOutMaxUpdates);
	//InOutMaxUpdates -= UpdatePendingStabilityData(InOutMaxUpdates);
	UpdateStabilityRecursive(InOutUpdatesLeft);
	UpdateHealth(InOutUpdatesLeft);
}

static TArray<EATAttachmentDirection> DirectionsOrder1 = { EATAttachmentDirection::Bottom, EATAttachmentDirection::Right, EATAttachmentDirection::Left, EATAttachmentDirection::Front, EATAttachmentDirection::Back, EATAttachmentDirection::Top };
static TArray<EATAttachmentDirection> DirectionsOrder2 = { EATAttachmentDirection::Top, EATAttachmentDirection::Back, EATAttachmentDirection::Front, EATAttachmentDirection::Left, EATAttachmentDirection::Right, EATAttachmentDirection::Bottom };
static TArray<EATAttachmentDirection> DirectionsOrder3 = { EATAttachmentDirection::Left, EATAttachmentDirection::Right, EATAttachmentDirection::Front, EATAttachmentDirection::Back, EATAttachmentDirection::Top, EATAttachmentDirection::Bottom };
static TArray<EATAttachmentDirection> DirectionsOrder4 = { EATAttachmentDirection::Right, EATAttachmentDirection::Left, EATAttachmentDirection::Back, EATAttachmentDirection::Front, EATAttachmentDirection::Top, EATAttachmentDirection::Bottom };
static TArray<EATAttachmentDirection> DirectionsOrder5 = { EATAttachmentDirection::Front, EATAttachmentDirection::Back, EATAttachmentDirection::Bottom, EATAttachmentDirection::Top, EATAttachmentDirection::Left, EATAttachmentDirection::Right };
static TArray<EATAttachmentDirection> DirectionsOrder6 = { EATAttachmentDirection::Back, EATAttachmentDirection::Front, EATAttachmentDirection::Top, EATAttachmentDirection::Bottom, EATAttachmentDirection::Right, EATAttachmentDirection::Left };

static TArray<EATAttachmentDirection> DirectionsOrder_OnlyBottom = { EATAttachmentDirection::Bottom };

static TArray<TArray<EATAttachmentDirection>*> UsedDirectionsOrders = { &DirectionsOrder1, &DirectionsOrder2, &DirectionsOrder3, &DirectionsOrder4, &DirectionsOrder5, &DirectionsOrder6 };

static uint8 MaxRecursionLevel = 32u;

void AATVoxelTree::UpdateStabilityRecursive(int32& InOutUpdatesLeft)
{
	/*if (!QueuedRecursiveStabilityUpdatePoints.IsEmpty())
	{
		VoxelComponent->RegenerateCompoundData();
	}*/
	TArraySetPair<FIntVector> SelectedUpdatePoints;
	QueuedRecursiveStabilityUpdatePoints.AddHeadTo(InOutUpdatesLeft, SelectedUpdatePoints, true);

	ParallelFor(SelectedUpdatePoints.Num(), [&](int32 InIndex)
	{
		const FIntVector& SamplePoint = SelectedUpdatePoints.GetConstArray()[InIndex];

		if (HasVoxelInstanceDataAtPoint(SamplePoint))
		{
			FVoxelInstanceData& SampleData = GetVoxelInstanceDataAtPoint(SamplePoint, false);
			SampleData.Stability = 0.0f;

			ensure(!UpdateStabilityRecursive_PointsCache.Contains(SamplePoint));
			FRecursivePointCache NewCache = FRecursivePointCache(SampleData.Stability);

			for (uint8 SampleOrderIndex = 0; SampleOrderIndex < UsedDirectionsOrders.Num(); ++SampleOrderIndex)
			{
				if (SampleData.Stability < 1.0f)
				{
					FRecursiveThreadData ThreadData = FRecursiveThreadData(*UsedDirectionsOrders[SampleOrderIndex]);
					float OrderStability = UpdateStabilityRecursive_GetStabilityFromAllNeighbors(SamplePoint, ThreadData);
					if (OrderStability > SampleData.Stability)
					{
						SampleData.Stability = OrderStability;
						NewCache = FRecursivePointCache(SampleData.Stability, ThreadData.ThisOrderUpdatedPoints);
					}
				}
				else
				{
					break;
				}
			}
			UpdateStabilityRecursive_PointsCache.Add(SamplePoint, NewCache);
		}
	}, EParallelForFlags::ForceSingleThread);
	//});

	for (const FIntVector& SamplePoint : SelectedUpdatePoints.GetConstArray())
	{
		if (!HasVoxelInstanceDataAtPoint(SamplePoint))
		{
			continue;
		}
		FVoxelInstanceData& SampleData = GetVoxelInstanceDataAtPoint(SamplePoint, false);

		if (SampleData.Stability < 0.25f)
		{
			InDangerGroupHealthUpdatePoints.Add(SamplePoint);
		}
		AATVoxelChunk* SampleChunk = GetVoxelChunkAtPoint(SamplePoint);
		ensureContinue(SampleChunk);

		//SampleChunk->HandleSetVoxelStabilityAtPoint(SamplePoint, SampleData.Stability);
		SampleChunk->HandleSetVoxelInstanceDataAtPoint(SamplePoint, SampleData);
	}
	InOutUpdatesLeft -= SelectedUpdatePoints.Num();
	//UpdateStabilityRecursive_CachedPointStabilities.Empty();
}

float AATVoxelTree::UpdateStabilityRecursive_GetStabilityFromAllNeighbors(const FIntVector& InTargetPoint, FRecursiveThreadData& InThreadData, EATAttachmentDirection InNeighborDirection, uint8 InCurrentRecursionLevel)
{
	++InCurrentRecursionLevel;

	if (InCurrentRecursionLevel > MaxRecursionLevel)
	{
		return 1.0f * EATAttachmentDirection_Utils::AttachmentMuls[InNeighborDirection];
	}
	FIntVector SamplePoint = InTargetPoint + EATAttachmentDirection_Utils::IntOffsets[InNeighborDirection];

	if (FRecursivePointCache* PointCachePtr = UpdateStabilityRecursive_PointsCache.Find(SamplePoint))
	{
		if (!PointCachePtr->Intersects(InThreadData.ThisOrderUpdatedPoints))
		{
			return PointCachePtr->Stability * EATAttachmentDirection_Utils::AttachmentMuls[InNeighborDirection];
		}
	}
	if (InThreadData.ThisOrderUpdatedPoints.Contains(SamplePoint))
	{
		return 0.0f;
	}
	FVoxelInstanceData& SampleData = GetVoxelInstanceDataAtPoint(SamplePoint, false);
	if (SampleData.IsTypeDataValid())
	{
		if (SampleData.TypeData->bIsFoundation)
		{
			return 1.0f * EATAttachmentDirection_Utils::AttachmentMuls[InNeighborDirection];
		}
		else
		{
			InThreadData.ThisOrderUpdatedPoints.Add(SamplePoint);
			float AccumulatedStability = 0.0f;

			if (InCurrentRecursionLevel + 1 == MaxRecursionLevel && InThreadData.DirectionsOrderPtr != &DirectionsOrder_OnlyBottom)
			{
				InThreadData.DirectionsOrderPtr = &DirectionsOrder_OnlyBottom;
				InCurrentRecursionLevel = 0u;
			}
			for (EATAttachmentDirection SampleDirection : (*InThreadData.DirectionsOrderPtr))
			{
				if (AccumulatedStability < 1.0f)
				{
					AccumulatedStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(SamplePoint, InThreadData, SampleDirection, InCurrentRecursionLevel);
				}
				else
				{
					break;
				}
			}
			float OutStability = FMath::Min(AccumulatedStability * EATAttachmentDirection_Utils::AttachmentMuls[InNeighborDirection], 1.0f);
			return OutStability;
		}
	}
	InThreadData.ThisOrderUpdatedPoints.Add(SamplePoint);
	return 0.0f;
}

void AATVoxelTree::UpdateHealth(int32& InOutUpdatesLeft)
{
	//TArraySetPair<FIntVector> SelectedUpdatePoints;
	//InDangerGroupHealthUpdatePoints.AddHeadTo(InOutUpdatesLeft, SelectedUpdatePoints, true);

	float DeltaTime = GetWorld()->DeltaTimeSeconds;

	ParallelFor(InDangerGroupHealthUpdatePoints.Num(), [&](int32 InIndex)
	{
		const FIntVector& SamplePoint = InDangerGroupHealthUpdatePoints.GetConstArray()[InIndex];
		FVoxelInstanceData& SampleData = GetVoxelInstanceDataAtPoint(SamplePoint, false);

		//ensure(SampleData.Stability < 0.25f);

		float HealthDrainMul = FMath::Square(FMath::Max(0.25f - SampleData.Stability, 0.1f) * 48.0f);
		SampleData.Health -= HealthDrainMul * DeltaTime;
	});

	TArraySetPair<FIntVector> BrokenPoints;
	for (const FIntVector& SamplePoint : InDangerGroupHealthUpdatePoints.GetConstArray())
	{
		FVoxelInstanceData& SampleData = GetVoxelInstanceDataAtPoint(SamplePoint, false);

		if (SampleData.Health > 0.0f)
		{
			AATVoxelChunk* SampleChunk = GetVoxelChunkAtPoint(SamplePoint);
			ensureContinue(SampleChunk);

			//SampleChunk->HandleSetVoxelHealthAtPoint(SamplePoint, SampleData.Health);
			SampleChunk->HandleSetVoxelInstanceDataAtPoint(SamplePoint, SampleData);
		}
		else
		{
			BrokenPoints.Add(SamplePoint);
			BreakVoxelAtPoint(SamplePoint, true, true); // Will update Chunk too
		}
	}
	InDangerGroupHealthUpdatePoints.RemoveFromOther(BrokenPoints);
	//InOutUpdatesLeft -= SelectedUpdatePoints.Num();
}

void AATVoxelTree::QueueRecursiveStabilityUpdate(const FIntVector& InChunkPoint, const bool bInQueueNeighborsToo)
{
	QueuedRecursiveStabilityUpdatePoints.Add(InChunkPoint);
	UpdateStabilityRecursive_PointsCache.Remove(InChunkPoint);

	if (bInQueueNeighborsToo)
	{
		TArray<FIntVector> PointsInRadius;
		GetAllVoxelPointsInRadius(InChunkPoint, 6, PointsInRadius);

		for (const FIntVector& SamplePoint : PointsInRadius)
		{
			QueuedRecursiveStabilityUpdatePoints.Add(SamplePoint);
			UpdateStabilityRecursive_PointsCache.Remove(SamplePoint);
		}
	}
}
//~ End Voxel Data

#if DEBUG_VOXELS
	#pragma optimize("", on)
#endif // DEBUG_VOXELS
