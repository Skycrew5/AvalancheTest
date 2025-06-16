// Avalanche Test

#include "World/ATVoxelChunk.h"

#include "Framework/ATGameState.h"

AATVoxelChunk::AATVoxelChunk()
{
	
}

//~ Begin Initialize
void AATVoxelChunk::OnConstruction(const FTransform& InTransform) // AActor
{
	Super::OnConstruction(InTransform);

	if (UWorld* World = GetWorld())
	{
		if (World->IsEditorWorld())
		{
			
		}
	}
}

void AATVoxelChunk::BeginPlay() // AActor
{
	UpdateCache();

	FVector GlobalLocation = GetActorLocation();
	FVector VoxelGlobalLocation = (GlobalLocation / Cache_GameState->VoxelBaseSize);
	GlobalOffsetXYZ = FIntVector(FMath::CeilToInt(VoxelGlobalLocation.X), FMath::CeilToInt(VoxelGlobalLocation.Y), FMath::CeilToInt(VoxelGlobalLocation.Z));

	Super::BeginPlay();
}

void AATVoxelChunk::EndPlay(const EEndPlayReason::Type InReason) // AActor
{
	ResetCache();

	Super::EndPlay(InReason);
}
//~ End Initialize

//~ Begin Getters
int32 AATVoxelChunk::GetVoxelGlobalIndexAt(const FIntVector& InLocalXYZ) const
{
	return Cache_GameState->GetVoxelIndexAt(GlobalOffsetXYZ + InLocalXYZ);
}

const FVoxelData& AATVoxelChunk::GetVoxelDataAt(const FIntVector& InLocalXYZ) const
{
	int32 TargetGlobalIndex = GetVoxelGlobalIndexAt(InLocalXYZ);
	return Cache_GameState->VoxelDataArray.IsValidIndex(TargetGlobalIndex) ? Cache_GameState->VoxelDataArray[TargetGlobalIndex] : FVoxelData::Invalid;
}

bool AATVoxelChunk::HasVoxelAt(const FIntVector& InLocalXYZ) const
{
	const FVoxelData& TargetVoxelData = GetVoxelDataAt(InLocalXYZ);
	return (TargetVoxelData.TypeID != FVoxelData::InvalidTypeID) && (TargetVoxelData.TypeID != FVoxelData::AirTypeID);
}

int32 AATVoxelChunk::GetVoxelNeighboursNumAt(const FIntVector& InLocalXYZ) const
{
	int32 OutNum = 0;

	if (HasVoxelAt(InLocalXYZ + FIntVector(1, 0, 0)))
	{
		OutNum += 1;
	}
	if (HasVoxelAt(InLocalXYZ + FIntVector(-1, 0, 0)))
	{
		OutNum += 1;
	}
	if (HasVoxelAt(InLocalXYZ + FIntVector(0, 1, 0)))
	{
		OutNum += 1;
	}
	if (HasVoxelAt(InLocalXYZ + FIntVector(0, -1, 0)))
	{
		OutNum += 1;
	}
	if (HasVoxelAt(InLocalXYZ + FIntVector(0, 0, 1)))
	{
		OutNum += 1;
	}
	if (HasVoxelAt(InLocalXYZ + FIntVector(0, 0, -1)))
	{
		OutNum += 1;
	}
	return OutNum;
}
//~ End Getters

//~ Begin Setters
void AATVoxelChunk::BreakVoxelAt(const FIntVector& InLocalXYZ)
{
	int32 TargetGlobalIndex = GetVoxelGlobalIndexAt(InLocalXYZ);
	if (Cache_GameState->VoxelDataArray.IsValidIndex(TargetGlobalIndex))
	{
		Cache_GameState->VoxelDataArray[TargetGlobalIndex] = FVoxelData::Air;
	}
}

void AATVoxelChunk::BreakVoxelsAt(const TArray<FIntVector>& InLocalXYZ)
{
	for (const FIntVector& SampleLocalXYZ : InLocalXYZ)
	{
		BreakVoxelAt(SampleLocalXYZ);
	}
}
//~ End Setters

//~ Begin Cache
void AATVoxelChunk::UpdateCache()
{
	Cache_GameState = AATGameState::TryGetATGameState(this);
	ensure(Cache_GameState);
}

void AATVoxelChunk::ResetCache()
{
	Cache_GameState = nullptr;
}
//~ End Cache