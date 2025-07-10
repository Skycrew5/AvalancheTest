// Scientific Ways

#include "World/ATVoxelTree.h"

#include "Framework/ATGameState.h"

#include "Simulations/ATSimulationComponent.h"

#include "World/ATVoxelChunk.h"
#include "World/ATVoxelTypeData.h"
#include "World/ATWorldFunctionLibrary.h"

#include "FastNoise2/FastNoise2BlueprintLibrary.h"

#if DEBUG_VOXELS
	#pragma optimize("", off)
#endif // DEBUG_VOXELS

AATVoxelTree::AATVoxelTree(const FObjectInitializer& InObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent")));
	SimulationComponent = CreateDefaultSubobject<UATSimulationComponent>(TEXT("SimulationComponent"));

	ChunkClass = AATVoxelChunk::StaticClass();
	TreeSizeInChunks = FIntVector(4, 4, 2);
	ChunkSize = 16;
	VoxelSize = 16.0f;
	ChunksUpdateMaxSquareExtent = 4;

	TreeSeed = 1337;

	MaxUpdatesPerSecond = 10000;
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

	InitVoxelGenerators();

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

void AATVoxelTree::HandleChunkUpdates(int32& InOutUpdatesLeft)
{
	ensureReturn(SimulationComponent);
	if (!SimulationComponent->IsCurrentTaskActive())
	{
		for (const AActor* UpdateReferenceActor : ChunksUpdateReferenceActors)
		{
			FIntPoint UpdatePointXY = UATWorldFunctionLibrary::WorldLocation_To_PointXY(UpdateReferenceActor->GetActorLocation(), GetVoxelSize());
			InitVoxelChunksInSquare(UpdatePointXY / GetChunkSize(), ChunksUpdateMaxSquareExtent, InOutUpdatesLeft);

			if (InOutUpdatesLeft <= 0)
			{
				//break;
				return;
			}
		}
	}
	TArray<FIntVector> MostRelevantChunkCoords;
	//GetMostRelevantChunksForTick(MostRelevantChunks);
	ChunksMap.GenerateKeyArray(MostRelevantChunkCoords);

	for (const FIntVector& SampleChunkCoords : MostRelevantChunkCoords)
	{
		AATVoxelChunk* SampleChunk = GetVoxelChunkAtCoords(SampleChunkCoords);
		ensureContinue(SampleChunk);
		SampleChunk->HandleUpdates(InOutUpdatesLeft);
	}
}

void AATVoxelTree::InitVoxelChunksInSquare(const FIntPoint& InSquareCenterXY, const int32 InSquareExtentXY, int32& InOutUpdatesLeft)
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

	bIsInitializingVoxelChunks = true;

	FIntPoint BackLeftCorner = FIntPoint(FMath::Max(InSquareCenterXY.X - InSquareExtentXY, 0), FMath::Max(InSquareCenterXY.Y - InSquareExtentXY, 0));
	FIntPoint FrontRightCorner = FIntPoint(FMath::Min(InSquareCenterXY.X + InSquareExtentXY, TreeSizeInChunks.X), FMath::Min(InSquareCenterXY.Y + InSquareExtentXY, TreeSizeInChunks.Y));

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

				SampleChunk->BP_InitChunk(this, SamplePoint);

				if (SamplePoint.Z == 0)
				{
					CreateFoundationAtChunk(SamplePoint);
					InOutUpdatesLeft -= FMath::Square(GetChunkSize());
				}
				SampleChunk->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
				SampleChunk->FinishSpawning(SampleTransform);

				//InOutUpdatesLeft -= FMath::Cube(GetChunkSize());

				if (InOutUpdatesLeft <= 0)
				{
					goto LeaveLoop;
				}
			}
		}
	}
	LeaveLoop:
	bIsInitializingVoxelChunks = false;
}
//~ End Voxel Chunks

//~ Begin Voxel Generation
void AATVoxelTree::InitVoxelGenerators()
{
	ensureReturn(VoxelGenerator_Perlin == nullptr);
	VoxelGenerator_Perlin = UFastNoise2BlueprintLibrary::MakePerlinGenerator();
}
//~ End Voxel Generation

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

bool AATVoxelTree::IsPointInsideLoadedChunk(const FIntVector& InPoint) const
{
	return ChunksMap.Contains(GetVoxelChunkCoordsAtPoint(InPoint));
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
		ensure(!HasVoxelInstanceDataAtPoint(InPoint));

		FVoxelInstanceData NewInstanceData = Queued_Point_To_VoxelInstanceData_Map.Add(InPoint, InTypeData->BP_InitializeInstanceData(this, InPoint));
		SampleChunk->HandleSetVoxelInstanceDataAtPoint(InPoint, NewInstanceData);

		if (bIsInitializingVoxelChunks)
		{
			HandleQueuedVoxelInstanceData(InPoint);
		}
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
		Queued_Point_To_VoxelInstanceData_Map.Add(InPoint, FVoxelInstanceData::Invalid);
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
	int32 UpdatesLeft = FMath::CeilToInt((float)MaxUpdatesPerSecond * InDeltaSeconds);
	HandleChunkUpdates(UpdatesLeft);
}

void AATVoxelTree::ApplyQueued_Point_To_VoxelInstanceData_Map()
{
	if (Queued_Point_To_VoxelInstanceData_Map.IsEmpty())
	{

	}
	else
	{
		TArray<FIntVector> QueuedPointArray;
		Queued_Point_To_VoxelInstanceData_Map.GenerateKeyArray(QueuedPointArray);

		for (const FIntVector& SamplePoint : QueuedPointArray)
		{
			HandleQueuedVoxelInstanceData(SamplePoint);
		}
	}
}

void AATVoxelTree::HandleQueuedVoxelInstanceData(const FIntVector& InPoint)
{
	ensureReturn(SimulationComponent);

	ensureReturn(Queued_Point_To_VoxelInstanceData_Map.Contains(InPoint));
	const FVoxelInstanceData& QueuedVoxelInstanceData = Queued_Point_To_VoxelInstanceData_Map[InPoint];

	if (QueuedVoxelInstanceData.IsTypeDataValid())
	{
		FVoxelInstanceData& AddedVoxelInstanceData = Point_To_VoxelInstanceData_Map.Add(InPoint, QueuedVoxelInstanceData);

		if (bIsInitializingVoxelChunks)
		{
			
		}
		else
		{
			SimulationComponent->QueuePointForTaskAtIndex(0, InPoint);
		}
	}
	else
	{
		Point_To_VoxelInstanceData_Map.Remove(InPoint);
		SimulationComponent->QueuePointForTaskAtIndex(0, InPoint);
	}
	Queued_Point_To_VoxelInstanceData_Map.Remove(InPoint);
}
//~ End Voxel Data

//~ Begin Voxel Simulation
void AATVoxelTree::QueueFullUpdateAtChunk(const FIntVector& InChunkCoords)
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
