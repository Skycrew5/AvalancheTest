// Avalanche Test

#include "World/ATVoxelChunk.h"

#include "Framework/ATGameState.h"

#include "World/ATVoxelTypeData.h"

AATVoxelChunk::AATVoxelChunk()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	InstancedStaticMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedStaticMeshComponent"));
	InstancedStaticMeshComponent->SetupAttachment(RootComponent);

	VoxelChunkSize = 8;
	VoxelBaseSize = 16.0f;
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

	Super::BeginPlay();

	InstanceIndexUpdatedDelegateHandle = FInstancedStaticMeshDelegates::OnInstanceIndexUpdated.AddUObject(this, &AATVoxelChunk::OnInstanceIndicesUpdated);
}

void AATVoxelChunk::EndPlay(const EEndPlayReason::Type InReason) // AActor
{
	ResetCache();

	Super::EndPlay(InReason);
}
//~ End Initialize

//~ Begin Getters
const FVoxelInstanceData& AATVoxelChunk::GetVoxelDataAtLocalPoint(const FIntVector& InLocalPoint) const
{
	if (const FVoxelInstanceData* SampleData = LocalPointToVoxelDataMap.Find(InLocalPoint))
	{
		return *SampleData;
	}
	return FVoxelInstanceData::Invalid;
}

/*const FVoxelInstanceData& AATVoxelChunk::GetVoxelDataAtIndex(int32 InLocalIndex) const
{
	return VoxelDataArray.IsValidIndex(InLocalIndex) ? VoxelDataArray[InLocalIndex] : FVoxelInstanceData::Invalid;
}*/

int32 AATVoxelChunk::LocalPoint_To_LocalIndex(const FIntVector& InLocalPoint) const
{
	if (!Cache_LocalPoint_To_LocalIndex_Map.Contains(InLocalPoint))
	{
		return const_cast<ThisClass*>(this)->Cache_LocalPoint_To_LocalIndex_Map.Add(InLocalPoint, InLocalPoint.X + InLocalPoint.Y * VoxelChunkSize + InLocalPoint.Z * (VoxelChunkSize * VoxelChunkSize));
	}
	return Cache_LocalPoint_To_LocalIndex_Map[InLocalPoint];
}

const FIntVector& AATVoxelChunk::LocalIndex_To_LocalPoint(int32 InLocalIndex) const
{
	if (!Cache_LocalIndex_To_LocalPoint_Map.Contains(InLocalIndex))
	{
		return const_cast<ThisClass*>(this)->Cache_LocalIndex_To_LocalPoint_Map.Add(InLocalIndex, FIntVector(InLocalIndex % VoxelChunkSize, (InLocalIndex / VoxelChunkSize) % VoxelChunkSize, (InLocalIndex / (VoxelChunkSize * VoxelChunkSize)) % VoxelChunkSize));
	}
	return Cache_LocalIndex_To_LocalPoint_Map[InLocalIndex];
}

FIntVector AATVoxelChunk::RelativeLocation_To_LocalPoint(const FVector& InRelativeLocation) const
{
	FVector VoxelScaledRelativeLocation = (InRelativeLocation / VoxelBaseSize);
	return FIntVector(FMath::CeilToInt(VoxelScaledRelativeLocation.X), FMath::CeilToInt(VoxelScaledRelativeLocation.Y), FMath::CeilToInt(VoxelScaledRelativeLocation.Z));
}

FIntVector AATVoxelChunk::WorldLocation_To_LocalPoint(const FVector& InWorldLocation) const
{
	return RelativeLocation_To_LocalPoint(GetActorTransform().InverseTransformPosition(InWorldLocation));
}

FVector AATVoxelChunk::LocalPoint_To_RelativeLocation(const FIntVector& InLocalPoint) const
{
	return FVector(InLocalPoint * VoxelBaseSize);
}

bool AATVoxelChunk::HasVoxelAtLocalPoint(const FIntVector& InLocalPoint) const
{
	const FVoxelInstanceData& TargetVoxelData = GetVoxelDataAtLocalPoint(InLocalPoint);
	return TargetVoxelData.TypeData != nullptr;
}

int32 AATVoxelChunk::GetVoxelNeighboursNumAtLocalPoint(const FIntVector& InLocalPoint) const
{
	int32 OutNum = 0;

	if (HasVoxelAtLocalPoint(InLocalPoint + FIntVector(1, 0, 0)))
	{
		OutNum += 1;
	}
	if (HasVoxelAtLocalPoint(InLocalPoint + FIntVector(-1, 0, 0)))
	{
		OutNum += 1;
	}
	if (HasVoxelAtLocalPoint(InLocalPoint + FIntVector(0, 1, 0)))
	{
		OutNum += 1;
	}
	if (HasVoxelAtLocalPoint(InLocalPoint + FIntVector(0, -1, 0)))
	{
		OutNum += 1;
	}
	if (HasVoxelAtLocalPoint(InLocalPoint + FIntVector(0, 0, 1)))
	{
		OutNum += 1;
	}
	if (HasVoxelAtLocalPoint(InLocalPoint + FIntVector(0, 0, -1)))
	{
		OutNum += 1;
	}
	return OutNum;
}

void AATVoxelChunk::GetVoxelPointsInSphere(const FIntVector& InCenterLocalPoint, int32 InRadius, TArray<FIntVector>& OutPoints) const
{
	for (int32 OffsetX = -InRadius; OffsetX < InRadius + 1; ++OffsetX)
	{
		int32 SliceRadius = InRadius - FMath::Abs(OffsetX);

		for (int32 OffsetY = -SliceRadius; OffsetY < SliceRadius + 1; ++OffsetY)
		{
			for (int32 OffsetZ = -SliceRadius; OffsetZ < SliceRadius + 1; ++OffsetZ)
			{
				OutPoints.Add(InCenterLocalPoint + FIntVector(OffsetX, OffsetY, OffsetZ));
			}
		}
	}
}

/*void AATVoxelChunk::GetVoxelIndicesInSphere(int32 InCenterLocalIndex, int32 InRadius, TArray<int32>& OutIndices) const
{
	const FIntVector& CenterLocalPoint = LocalIndex_To_LocalPoint(InCenterLocalIndex);

	for (int32 OffsetX = -InRadius; OffsetX < InRadius + 1; ++OffsetX)
	{
		int32 SliceRadius = InRadius - FMath::Abs(OffsetX);

		for (int32 OffsetY = -SliceRadius; OffsetY < SliceRadius + 1; ++OffsetY)
		{
			for (int32 OffsetZ = -SliceRadius; OffsetZ < SliceRadius + 1; ++OffsetZ)
			{
				OutIndices.Add(LocalPoint_To_LocalIndex(CenterLocalPoint + FIntVector(OffsetX, OffsetY, OffsetZ)));
			}
		}
	}
}*/
//~ End Getters

//~ Begin Setters
void AATVoxelChunk::SetVoxelAtLocalPoint(const FIntVector& InLocalPoint, const UATVoxelTypeData* InTypeData)
{
	BreakVoxelAtLocalPoint(InLocalPoint);

	if (InTypeData)
	{
		FTransform NewInstanceTransform = FTransform(LocalPoint_To_RelativeLocation(InLocalPoint));
		int32 NewInstanceIndex = InstancedStaticMeshComponent->AddInstance(NewInstanceTransform);

		Data.AddVoxel(NewInstanceIndex, InLocalPoint, InTypeData->K2_InitializeInstanceData(this, InLocalPoint));
	}
}

void AATVoxelChunk::SetVoxelsAtLocalPoints(const TArray<FIntVector>& InLocalPoints, const UATVoxelTypeData* InTypeData)
{
	for (const FIntVector& SampleLocalPoint : InLocalPoints)
	{
		SetVoxelAtLocalPoint(SampleLocalPoint, InTypeData);
	}
}

/*void AATVoxelChunk::BreakVoxelAtLocalIndex(int32 InLocalIndex)
{
	if (VoxelDataArray.IsValidIndex(InLocalIndex))
	{
		VoxelDataArray[InLocalIndex] = FVoxelInstanceData::Invalid;
	}
}*/

/*void AATVoxelChunk::BreakVoxelsAtLocalIndices(const TArray<int32>& InLocalIndices)
{
	for (int32 SampleLocalIndex : InLocalIndices)
	{
		BreakVoxelAtLocalIndex(SampleLocalIndex);
	}
}*/

void AATVoxelChunk::BreakVoxelAtLocalPoint(const FIntVector& InLocalPoint)
{
	if (Data.RemoveVoxelByPoint(InLocalPoint))
	{
		UpdateStabilityData();
	}
}

void AATVoxelChunk::BreakVoxelsAtLocalPoints(const TArray<FIntVector>& InLocalPoints)
{
	TArray<int32> InstanceIndices;

	for (const FIntVector& SampleLocalPoint : InLocalPoints)
	{
		InstanceIndices.Add(Data.GetVoxelInstanceIndex(SampleLocalPoint));
	}
	BreakVoxelsWithInstanceIds(InstanceIndices);
}

void AATVoxelChunk::BreakVoxelsWithInstanceIds(const TArray<int32>& InInstanceIndices)
{
	InstancedStaticMeshComponent->RemoveInstances(InInstanceIndices);
	UpdateStabilityData();
}
//~ End Setters

//~ Begin Data
void AATVoxelChunk::UpdateStabilityData()
{

}

void AATVoxelChunk::OnInstanceIndicesUpdated(UInstancedStaticMeshComponent* InUpdatedComponent, TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData> InIndexUpdates)
{
	if (InUpdatedComponent == InstancedStaticMeshComponent)
	{
		for (const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData& SampleUpdate : InIndexUpdates)
		{
			switch (SampleUpdate.Type)
			{
				case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Added:
				{
					break;
				}
				case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Removed:
				case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Cleared:
				case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Destroyed:
				{
					Data.RemoveVoxelByIndex(SampleUpdate.Index);
					break;
				}
				case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Relocated:
				{
					Data.ChangeVoxelInstanceIndex(SampleUpdate.OldIndex, SampleUpdate.Index);
					break;
				}
			}
		}
	}
}
//~ End Data

//~ Begin Cache
void AATVoxelChunk::UpdateCache()
{
	Cache_GameState = AATGameState::TryGetATGameState(this);
	ensure(Cache_GameState);
}

void AATVoxelChunk::ResetCache()
{
	Cache_GameState = nullptr;
	Cache_LocalPoint_To_LocalIndex_Map.Empty();
	Cache_LocalIndex_To_LocalPoint_Map.Empty();
}
//~ End Cache