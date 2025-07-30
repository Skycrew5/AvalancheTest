// Scientific Ways

#include "World/ATVoxelTree.h"

#include "Framework/ATGameState.h"

#include "Procedural/ATProceduralGeneratorComponent.h"

#include "Simulation/ATSimulationComponent.h"

#include "World/ATVoxelChunk.h"
#include "World/ATVoxelTypeData.h"
#include "World/ATWorldFunctionLibrary.h"

#if DEBUG_VOXELS
	#pragma optimize("", off)
#endif // DEBUG_VOXELS

AATVoxelTree::AATVoxelTree(const FObjectInitializer& InObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent")));
	SimulationComponent = CreateDefaultSubobject<UATSimulationComponent>(TEXT("SimulationComponent"));
	ProceduralGeneratorComponent = CreateDefaultSubobject<UATProceduralGeneratorComponent>(TEXT("ProceduralGeneratorComponent"));

	ChunkClass = AATVoxelChunk::StaticClass();
	TreeSizeInChunks = FIntVector(4, 4, 2);
	ChunkSize = 16;
	VoxelSize = 16.0f;
	ChunksUpdateMaxSquareExtent = 4;

	TreeSeed = 1337;

	TickUpdatesTimeBudgetMs = 10.000;
	TickUpdatesTimeBudgetMs_PerQueuedChunkAdditive = 0.0;
	TickUpdatesTimeBudgetMs_PerSkipSimulationPointQueueAdditive = 0.001;
}

//~ Begin Actor
void AATVoxelTree::PostInitializeComponents() // AActor
{
	Super::PostInitializeComponents();

	BoundsSize = TreeSizeInChunks * ChunkSize;
}

void AATVoxelTree::OnConstruction(const FTransform& InTransform) // AActor
{
	Super::OnConstruction(InTransform);


}

void AATVoxelTree::BeginPlay() // AActor
{
	Super::BeginPlay();

	if (AATGameState* GameState = AATGameState::TryGetATGameState(this))
	{
		GameState->SetMainVoxelTree(this);
	}
	//InitVoxelChunksInSquare(FIntPoint(TreeSizeInChunks.X, TreeSizeInChunks.Y) / 2, TreeSizeInChunks.GetAbsMax());
}

void AATVoxelTree::Tick(float InDeltaSeconds) // AActor
{
	Super::Tick(InDeltaSeconds);

	HandleTickUpdate(InDeltaSeconds);

	GetWorldTimerManager().SetTimer(UpdateSortedChunkArrayTimerHandle, this, &AATVoxelTree::UpdateSortedChunkArray, 1.337f);

	SET_MEMORY_STAT(STAT_VoxelData_Queued_Point_To_VoxelInstanceData_Map, Queued_Point_To_VoxelInstanceData_Map.GetAllocatedSize());
	SET_MEMORY_STAT(STAT_VoxelData_Queued_PointsSkipSimulationQueue_Set, Queued_PointsSkipSimulationQueue_Set.GetAllocatedSize());
	SET_MEMORY_STAT(STAT_VoxelData_Point_To_VoxelInstanceData_Map, Point_To_VoxelInstanceData_Map.GetAllocatedSize());
}

void AATVoxelTree::EndPlay(const EEndPlayReason::Type InReason) // AActor
{
	Super::EndPlay(InReason);


}
//~ End Actor

int32 FloorDiv(int32 A, int32 B)
{
	return (A >= 0) ? (A / B) : ((A - B + 1) / B);
}

//~ Begin Voxel Chunks
bool AATVoxelTree::IsChunkCoordsInsideTree(const FIntVector& InChunkCoords) const
{
	return (InChunkCoords.X >= 0 && InChunkCoords.X < TreeSizeInChunks.X)
		&& (InChunkCoords.Y >= 0 && InChunkCoords.Y < TreeSizeInChunks.Y)
		&& (InChunkCoords.Z >= 0 && InChunkCoords.Z < TreeSizeInChunks.Z);
}

bool AATVoxelTree::IsChunkCoordsOnTreeSide(const FIntVector& InChunkCoords, const bool bInIgnoreBottom) const
{
	return (InChunkCoords.X == 0 || InChunkCoords.X == (TreeSizeInChunks.X - 1))
		|| (InChunkCoords.Y == 0 || InChunkCoords.Y == (TreeSizeInChunks.Y - 1))
		|| (bInIgnoreBottom ? false : (InChunkCoords.Z == 0 || InChunkCoords.Z == TreeSizeInChunks.Z - 1));
}

FIntVector AATVoxelTree::GetVoxelChunkCoordsAtPoint(const FIntVector& InPoint) const
{
	//return FIntVector(InPoint.X / ChunkSize, InPoint.Y / ChunkSize, InPoint.Z / ChunkSize);
	return FIntVector(FloorDiv(InPoint.X, ChunkSize), FloorDiv(InPoint.Y, ChunkSize), FloorDiv(InPoint.Z, ChunkSize));
}

void AATVoxelTree::RegisterChunksUpdateReferenceActor(const AActor* InActor)
{
	ensureReturn(!ChunksUpdateReferenceActors.Contains(InActor));
	ChunksUpdateReferenceActors.Add(InActor);
}

void AATVoxelTree::UnRegisterChunksUpdateReferenceActor(const AActor* InActor)
{
	ensureReturn(ChunksUpdateReferenceActors.Contains(InActor));
	ChunksUpdateReferenceActors.Remove(InActor);
}

void AATVoxelTree::HandleChunkUpdates()
{
	ensureReturn(SimulationComponent);
	if (!SimulationComponent->IsCurrentTaskActive())
	{
		TArray<AATVoxelChunk*> NewChunks;

		for (const AActor* UpdateReferenceActor : ChunksUpdateReferenceActors)
		{
			FIntPoint UpdatePointXY = UATWorldFunctionLibrary::WorldLocation_To_PointXY(UpdateReferenceActor->GetActorLocation(), GetVoxelSize());
			InitVoxelChunksInSquare(UpdatePointXY / GetChunkSize(), ChunksUpdateMaxSquareExtent, NewChunks);

			if (IsThisTickUpdatesTimeBudgetExceeded())
			{
				break;
			}
		}
		if (NewChunks.IsEmpty())
		{

		}
		else
		{
			UpdateSortedChunkArray();
			ProceduralGeneratorComponent->QueueChunksForTaskAtIndex(NewChunks, 0);
		}
	}
	if (IsThisTickUpdatesTimeBudgetExceeded())
	{
		return;
	}
	SET_MEMORY_STAT(STAT_VoxelComponents_Point_To_MeshIndex_Map, 0);

	for (FChunkWithSquaredDistance& SampleData : SortedChunksData.DataArray)
	{
		AATVoxelChunk* SampleChunk = SampleData.Chunk;
		ensureContinue(SampleChunk);
		SampleChunk->HandleUpdates();

		if (IsThisTickUpdatesTimeBudgetExceeded())
		{
			//break;
			return;
		}
	}
}

void AATVoxelTree::UpdateSortedChunkArray()
{
	SortedChunksData.UpdateDistancesAndSort(this, false);
}

void AATVoxelTree::InitVoxelChunksInSquare(const FIntPoint& InSquareCenterXY, const int32 InSquareExtentXY, TArray<AATVoxelChunk*>& OutNewChunks)
{
	if (InSquareExtentXY < 1)
	{
		return;
	}
	UWorld* World = GetWorld();
	ensureReturn(World);

	ensureReturn(!TreeSizeInChunks.IsZero());
	ensureReturn(ChunkSize > 0);
	ensureReturn(VoxelSize > 0.0f);

	FIntPoint BackLeftCorner = FIntPoint(FMath::Max(InSquareCenterXY.X - InSquareExtentXY, 0), FMath::Max(InSquareCenterXY.Y - InSquareExtentXY, 0));
	FIntPoint FrontRightCorner = FIntPoint(FMath::Min(InSquareCenterXY.X + InSquareExtentXY, TreeSizeInChunks.X), FMath::Min(InSquareCenterXY.Y + InSquareExtentXY, TreeSizeInChunks.Y));

	ensure(OutNewChunks.IsEmpty());

	for (int32 SampleX = BackLeftCorner.X; SampleX < FrontRightCorner.X; ++SampleX)
	{
		for (int32 SampleY = BackLeftCorner.Y; SampleY < FrontRightCorner.Y; ++SampleY)
		{
			for (int32 SampleZ = 0; SampleZ < TreeSizeInChunks.Z; ++SampleZ)
			{
				FIntVector SamplePoint = FIntVector(SampleX, SampleY, SampleZ);

				if (ChunksMap.Contains(SamplePoint))
				{
					continue;
				}
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
				SortedChunksData.DataArray.Add(FChunkWithSquaredDistance(SampleChunk, 0));

				SampleChunk->BP_InitChunk(this, SamplePoint);

				SampleChunk->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
				SampleChunk->FinishSpawning(SampleTransform);

				OutNewChunks.Add(SampleChunk);

				if (IsThisTickUpdatesTimeBudgetExceeded())
				{
					goto LeaveLoop;
				}
			}
		}
	}
LeaveLoop:
	return;
}
//~ End Voxel Chunks

//~ Begin Voxel Getters
bool AATVoxelTree::HasVoxelInstanceDataAtPoint(const FIntVector& InPoint, const bool bInIgnoreQueued) const
{
	if (!bInIgnoreQueued)
	{
		check(IsInGameThread());

		if (Queued_Point_To_VoxelInstanceData_Map.Contains(InPoint))
		{
			return Queued_Point_To_VoxelInstanceData_Map[InPoint].IsTypeDataValid();
		}
	}
	return Point_To_VoxelInstanceData_Map.Contains(InPoint);
}

FVoxelInstanceData& AATVoxelTree::GetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const bool bInChecked, const bool bInIgnoreQueued) const
{
	if (!bInIgnoreQueued)
	{
		check(IsInGameThread());

		if (const FVoxelInstanceData* SampleQueuedData = Queued_Point_To_VoxelInstanceData_Map.Find(InPoint))
		{
			return const_cast<FVoxelInstanceData&>(*SampleQueuedData);
		}
	}
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

int32 AATVoxelTree::GetVoxelNeighborsNumAtPoint(const FIntVector& InPoint, const bool bInIgnoreQueued) const
{
	int32 OutNum = 0;

	if (HasVoxelInstanceDataAtPoint(InPoint + FIntVector(1, 0, 0), bInIgnoreQueued)) OutNum += 1;
	if (HasVoxelInstanceDataAtPoint(InPoint + FIntVector(-1, 0, 0), bInIgnoreQueued)) OutNum += 1;
	if (HasVoxelInstanceDataAtPoint(InPoint + FIntVector(0, 1, 0), bInIgnoreQueued)) OutNum += 1;
	if (HasVoxelInstanceDataAtPoint(InPoint + FIntVector(0, -1, 0), bInIgnoreQueued)) OutNum += 1;
	if (HasVoxelInstanceDataAtPoint(InPoint + FIntVector(0, 0, 1), bInIgnoreQueued)) OutNum += 1;
	if (HasVoxelInstanceDataAtPoint(InPoint + FIntVector(0, 0, -1), bInIgnoreQueued)) OutNum += 1;
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

bool AATVoxelTree::CanBreakVoxelAtPoint(const FIntVector& InPoint, const bool bInIgnoreQueued) const
{
	if (HasVoxelInstanceDataAtPoint(InPoint, bInIgnoreQueued))
	{
		FVoxelInstanceData& Data = GetVoxelInstanceDataAtPoint(InPoint, bInIgnoreQueued);
		ensureReturn(Data.IsTypeDataValid(), false);
		return !Data.TypeData->bIsUnbreakable;
	}
	return false;
}

bool AATVoxelTree::IsPointInsideTree(const FIntVector& InPoint) const
{
	return (InPoint.X >= 0 && InPoint.X < BoundsSize.X)
		&& (InPoint.Y >= 0 && InPoint.Y < BoundsSize.Y)
		&& (InPoint.Z >= 0 && InPoint.Z < BoundsSize.Z);
}

bool AATVoxelTree::IsPointInsideSimulationReadyChunk(const FIntVector& InPoint) const
{
	if (const AATVoxelChunk* SampleChunk = ChunksMap.FindRef(GetVoxelChunkCoordsAtPoint(InPoint), nullptr))
	{
		return SampleChunk->IsChunkSimulationReady();
	}
	return false;
}
//~ End Voxel Getters

//~ Begin Voxel Setters
bool AATVoxelTree::SetVoxelAtPoint(const FIntVector& InPoint, const UATVoxelTypeData* InTypeData, const bool bInForced)
{
	ensureReturn(InTypeData, false);

	if (bInForced)
	{
		BreakVoxelAtPoint(InPoint, FVoxelBreakData(true, false));
	}
	else if (HasVoxelInstanceDataAtPoint(InPoint))
	{
		return false;
	}
	if (AATVoxelChunk* SampleChunk = GetVoxelChunkAtPoint(InPoint))
	{
		ensure(!HasVoxelInstanceDataAtPoint(InPoint));

		FVoxelInstanceData NewInstanceData = Queued_Point_To_VoxelInstanceData_Map.Add(InPoint, InTypeData->BP_InitializeInstanceData(this, InPoint));
		SampleChunk->HandleSetVoxelInstanceDataAtPoint(InPoint, NewInstanceData);

		if (!SampleChunk->IsChunkSimulationReady())
		{
			Queued_PointsSkipSimulationQueue_Set.Add(InPoint);
		}
		return true;
	}
	//ensure(false);
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

bool AATVoxelTree::BreakVoxelAtPoint(const FIntVector& InPoint, const FVoxelBreakData& InBreakData)
{
	if (!InBreakData.bForced && !CanBreakVoxelAtPoint(InPoint))
	{
		return false;
	}
	if (AATVoxelChunk* SampleChunk = GetVoxelChunkAtPoint(InPoint))
	{
		if (HasVoxelInstanceDataAtPoint(InPoint))
		{
			FVoxelInstanceData& SampleData = GetVoxelInstanceDataAtPoint(InPoint);
			SampleChunk->HandleBreakVoxelAtPoint(InPoint, InBreakData);

			Queued_Point_To_VoxelInstanceData_Map.Add(InPoint, FVoxelInstanceData::Invalid);

			if (InBreakData.bNotify)
			{
				FBrokenVoxelsData& SampleBrokenVoxelsData = Queued_VoxelTypeData_To_BrokenVoxelsData_Map.FindOrAdd(SampleData.TypeData, FBrokenVoxelsData({}, InBreakData));

				ensureIf(!SampleBrokenVoxelsData.Points.Contains(InPoint))
				{
					SampleBrokenVoxelsData.Points.Add(InPoint);
				}
			}
			return true;
		}
	}
	return false;
}

bool AATVoxelTree::BreakVoxelsAtPoints(const TArray<FIntVector>& InPoints, const FVoxelBreakData& InBreakData)
{
	bool bAnyBroken = false;

	for (const FIntVector& SamplePoint : InPoints)
	{
		bAnyBroken |= BreakVoxelAtPoint(SamplePoint, InBreakData);
	}
	return bAnyBroken;
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

void AATVoxelTree::HandleBrokenVoxelsUpdates()
{
	if (Queued_VoxelTypeData_To_BrokenVoxelsData_Map.IsEmpty())
	{
		Queued_VoxelTypeData_To_BrokenVoxelsData_Map.Compact();
	}
	else
	{
		TArray<const UATVoxelTypeData*> QueuedVoxelTypeDataArray;
		Queued_VoxelTypeData_To_BrokenVoxelsData_Map.GenerateKeyArray(QueuedVoxelTypeDataArray);

		for (const UATVoxelTypeData* SampleVoxelTypeData : QueuedVoxelTypeDataArray)
		{
			const FBrokenVoxelsData& SampleBrokenVoxelsData = Queued_VoxelTypeData_To_BrokenVoxelsData_Map[SampleVoxelTypeData];
			OnBreakVoxelsAtPoints.Broadcast(SampleVoxelTypeData, SampleBrokenVoxelsData.Points, SampleBrokenVoxelsData.BreakData);

			Queued_VoxelTypeData_To_BrokenVoxelsData_Map.Remove(SampleVoxelTypeData);

			//if (IsThisTickUpdatesTimeBudgetExceeded())
			{
			//	break;
			}
		}
	}
}
//~ End Voxel Setters

//~ Begin Voxel Data
bool AATVoxelTree::IsThisTickUpdatesTimeBudgetExceeded() const
{
	return FPlatformTime::Cycles64() > ThisTickUpdatesTimeBudget_CyclesThreshold;
}

void AATVoxelTree::SetThisTickUpdatesTimeBudget(double InTimeMs)
{
	ThisTickUpdatesTimeBudget_CyclesThreshold = FPlatformTime::Cycles64() + FPlatformTime::SecondsToCycles64(InTimeMs * 0.001);
}

void AATVoxelTree::ForceTickUpdateNextFrame()
{
	FTimerManager& TimerManager = GetWorldTimerManager();

	if (TimerManager.IsTimerActive(ForceTickUpdateNextFrameTimerHandle))
	{

	}
	else
	{
		ForceTickUpdateNextFrameTimerHandle = TimerManager.SetTimerForNextTick(this, &AATVoxelTree::HandleTickUpdate_FromForceTickUpdate);
	}
}

void AATVoxelTree::HandleTickUpdate_FromForceTickUpdate()
{
	UWorld* World = GetWorld();
	ensureReturn(World);

	HandleTickUpdate(World->GetDeltaSeconds());
}

void AATVoxelTree::HandleTickUpdate(float InDeltaSeconds)
{
	ensureReturn(ProceduralGeneratorComponent);

	double ThisTickUpdatesTimeBudgetMs = TickUpdatesTimeBudgetMs;
	ThisTickUpdatesTimeBudgetMs += (double)ProceduralGeneratorComponent->GetTotalQueuedChunksNum() * TickUpdatesTimeBudgetMs_PerQueuedChunkAdditive;
	ThisTickUpdatesTimeBudgetMs += (double)Queued_PointsSkipSimulationQueue_Set.Num() * TickUpdatesTimeBudgetMs_PerSkipSimulationPointQueueAdditive;

	if (CVarVoxelTree_ForceSyncUpdates.GetValueOnGameThread() > 0)
	{
		ThisTickUpdatesTimeBudgetMs = 10000.0; // Help synchronous updates with 10 seconds budget
	}
	SetThisTickUpdatesTimeBudget(ThisTickUpdatesTimeBudgetMs);

	HandleBrokenVoxelsUpdates();
	HandleChunkUpdates();
}

void AATVoxelTree::ApplyQueued_Point_To_VoxelInstanceData_Map()
{
	if (Queued_Point_To_VoxelInstanceData_Map.IsEmpty())
	{
		Queued_Point_To_VoxelInstanceData_Map.Compact();
	}
	else
	{
		TArray<FIntVector> QueuedPointArray;
		Queued_Point_To_VoxelInstanceData_Map.GenerateKeyArray(QueuedPointArray);

		for (const FIntVector& SamplePoint : QueuedPointArray)
		{
			HandleQueuedVoxelInstanceData(SamplePoint);

			if (IsThisTickUpdatesTimeBudgetExceeded())
			{
				break;
			}
		}
	}
}

void AATVoxelTree::HandleQueuedVoxelInstanceData(const FIntVector& InPoint)
{
	ensureReturn(SimulationComponent);
	ensureReturn(!SimulationComponent->IsCurrentTaskActive());

	ensureReturn(Queued_Point_To_VoxelInstanceData_Map.Contains(InPoint));
	const FVoxelInstanceData& QueuedVoxelInstanceData = Queued_Point_To_VoxelInstanceData_Map[InPoint];

	if (QueuedVoxelInstanceData.IsTypeDataValid())
	{
		FVoxelInstanceData& AddedVoxelInstanceData = Point_To_VoxelInstanceData_Map.Add(InPoint, QueuedVoxelInstanceData);
	}
	else
	{
		Point_To_VoxelInstanceData_Map.Remove(InPoint);
	}
	ensure(Queued_PointsSkipSimulationQueue_Set.Num() <= Queued_Point_To_VoxelInstanceData_Map.Num());

	if (Queued_PointsSkipSimulationQueue_Set.Contains(InPoint))
	{
		Queued_PointsSkipSimulationQueue_Set.Remove(InPoint);

		if (Queued_PointsSkipSimulationQueue_Set.IsEmpty())
		{
			Queued_PointsSkipSimulationQueue_Set.Compact();
		}
	}
	else
	{
		SimulationComponent->QueuePointForTaskAtIndex(0, InPoint, true);
	}
	Queued_Point_To_VoxelInstanceData_Map.Remove(InPoint);
}
//~ End Voxel Data

//~ Begin Voxel Simulation
void AATVoxelTree::QueueFullSimulationUpdateAtChunk(const FIntVector& InChunkCoords)
{
	ensureReturn(SimulationComponent);

	FIntVector ChunkOffset = InChunkCoords * ChunkSize;
	TArray<FIntVector> VoxelPoints;

	for (int32 SampleX = 0; SampleX < ChunkSize; ++SampleX)
	{
		for (int32 SampleY = 0; SampleY < ChunkSize; ++SampleY)
		{
			for (int32 SampleZ = 0; SampleZ < ChunkSize; ++SampleZ)
			{
				SimulationComponent->QueuePointForTaskAtIndex(0, ChunkOffset + FIntVector(SampleX, SampleY, SampleZ), false);
			}
		}
	}
}
//~ End Voxel Simulation

#if DEBUG_VOXELS
	#pragma optimize("", on)
#endif // DEBUG_VOXELS
