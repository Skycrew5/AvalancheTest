// Scientific Ways

#include "World/ATVoxelChunk.h"

#include "Framework/ATGameState.h"
#include "Framework/ScWPlayerController.h"

#include "World/ScWTypes_World.h"
#include "World/ATVoxelTypeData.h"

FVoxelInstanceData FVoxelChunkContainers::InvalidInstanceData_NonConst = FVoxelInstanceData::Invalid;

AATVoxelChunk::AATVoxelChunk()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	InstancedStaticMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedStaticMeshComponent"));
	InstancedStaticMeshComponent->SetNumCustomDataFloats(3);
	InstancedStaticMeshComponent->SetupAttachment(RootComponent);

	AttachmentUpdates.TargetISMC = InstancedStaticMeshComponent;
	AttachmentUpdates.DebugThisTickSelectedDataIndex = 1;

	StabilityUpdates.TargetISMC = InstancedStaticMeshComponent;
	StabilityUpdates.DebugThisTickSelectedDataIndex = 2;

	ChunkSize = 12;
	VoxelBaseSize = 16.0f;
	bIsOnFoundation = true;
	MaxUpdatesPerSecond = 10000;

	StabilityUpdatePropagationThreshold = 0.1f;
	StabilityUpdatePropagationSkipProbability = 0.1f;
}

//~ Begin Initialize
void AATVoxelChunk::OnConstruction(const FTransform& InTransform) // AActor
{
	Super::OnConstruction(InTransform);

	if (UWorld* World = GetWorld())
	{
		if (World->IsEditorWorld())
		{
			InstanceIndexUpdatedDelegateHandle = FInstancedStaticMeshDelegates::OnInstanceIndexUpdated.AddUObject(this, &AATVoxelChunk::OnInstanceIndicesUpdated);

			if (bIsOnFoundation)
			{
				CreateFoundation();
			}
		}
	}
}

void AATVoxelChunk::BeginPlay() // AActor
{
	UpdateCache();

	if (UWorld* World = GetWorld())
	{
		if (World->IsGameWorld())
		{
			InstanceIndexUpdatedDelegateHandle = FInstancedStaticMeshDelegates::OnInstanceIndexUpdated.AddUObject(this, &AATVoxelChunk::OnInstanceIndicesUpdated);

			if (bIsOnFoundation)
			{
				CreateFoundation();
			}
		}
	}
	Super::BeginPlay();
}

void AATVoxelChunk::Tick(float InDeltaSeconds) // AActor
{
	Super::Tick(InDeltaSeconds);

	int32 MaxUpdates = FMath::CeilToInt((float)MaxUpdatesPerSecond * InDeltaSeconds);
	MaxUpdates -= UpdatePendingAttachmentData(MaxUpdates);
	UpdatePendingStabilityData(MaxUpdates);
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
	return Data.GetVoxelInstanceData(InLocalPoint);
}

/*const FVoxelInstanceData& AATVoxelChunk::GetVoxelDataAtIndex(int32 InLocalIndex) const
{
	return VoxelDataArray.IsValidIndex(InLocalIndex) ? VoxelDataArray[InLocalIndex] : FVoxelInstanceData::Invalid;
}

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
}*/

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
	if (bOffsetMeshToCenter)
	{
		return FVector(InLocalPoint * VoxelBaseSize) + FVector(VoxelBaseSize * 0.5f, VoxelBaseSize * 0.5f, VoxelBaseSize * 0.5f);
	}
	else
	{
		return FVector(InLocalPoint * VoxelBaseSize);
	}
}

FVector AATVoxelChunk::LocalPoint_To_WorldLocation(const FIntVector& InLocalPoint) const
{
	return GetActorTransform().TransformPosition(LocalPoint_To_RelativeLocation(InLocalPoint));
}

FVector AATVoxelChunk::GetChunkCenterWorldLocation() const
{
	return GetActorLocation() + FVector((float)ChunkSize * VoxelBaseSize * 0.5f);
}

FVector AATVoxelChunk::GetVoxelCenterWorldLocation(const FIntVector& InLocalPoint) const
{
	return GetActorLocation() + FVector(InLocalPoint * VoxelBaseSize) + FVector(VoxelBaseSize * 0.5f, VoxelBaseSize * 0.5f, VoxelBaseSize * 0.5f);
}

bool AATVoxelChunk::HasVoxelAtLocalPoint(const FIntVector& InLocalPoint) const
{
	return Data.HasVoxelAt(InLocalPoint);
}

int32 AATVoxelChunk::GetVoxelNeighborsNumAtLocalPoint(const FIntVector& InLocalPoint) const
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
	if (!InTypeData)
	{
		return;
	}
	BreakVoxelAtLocalPoint(InLocalPoint);

	FTransform NewInstanceTransform = FTransform(LocalPoint_To_RelativeLocation(InLocalPoint));
	int32 NewInstanceIndex = InstancedStaticMeshComponent->AddInstance(NewInstanceTransform);

	Data.AddVoxel(InLocalPoint, InTypeData->BP_InitializeInstanceData(this, InLocalPoint), NewInstanceIndex);
}

void AATVoxelChunk::SetVoxelsAtLocalPoints(const TArray<FIntVector>& InLocalPoints, const UATVoxelTypeData* InTypeData)
{
	ensure(InTypeData);
	if (!InTypeData)
	{
		return;
	}
	TArray<FTransform> InstanceTransforms;
	for (const FIntVector& SampleLocalPoint : InLocalPoints)
	{
		InstanceTransforms.Add(FTransform(LocalPoint_To_RelativeLocation(SampleLocalPoint)));
	}
	TArray<int32> NewInstanceIndices = InstancedStaticMeshComponent->AddInstances(InstanceTransforms, true);

	ensure(NewInstanceIndices.Num() == InLocalPoints.Num());
	for (int32 SampleIndex = 0; SampleIndex < InLocalPoints.Num(); ++SampleIndex)
	{
		const FIntVector& SamplePoint = InLocalPoints[SampleIndex];

		Data.RemoveVoxelByPoint(SamplePoint, false);
		Data.AddVoxel(SamplePoint, InTypeData->BP_InitializeInstanceData(this, SamplePoint), NewInstanceIndices[SampleIndex]);
	}
}

void AATVoxelChunk::FillWithVoxels(const UATVoxelTypeData* InTypeData)
{
	TArray<FIntVector> VoxelPoints;

	for (int32 SampleX = 0; SampleX < ChunkSize; ++SampleX)
	{
		for (int32 SampleY = 0; SampleY < ChunkSize; ++SampleY)
		{
			for (int32 SampleZ = 0; SampleZ < ChunkSize; ++SampleZ)
			{
				VoxelPoints.Add(FIntVector(SampleX, SampleY, SampleZ));
			}
		}
	}
	SetVoxelsAtLocalPoints(VoxelPoints, InTypeData);
}

void AATVoxelChunk::ClearAll()
{
	InstancedStaticMeshComponent->ClearInstances();
}

void AATVoxelChunk::BreakVoxelAtLocalPoint(const FIntVector& InLocalPoint)
{
	const FVoxelInstanceData& TargetData = Data.GetVoxelInstanceData(InLocalPoint);
	if (TargetData.IsTypeDataValid() && !TargetData.TypeData->IsFoundation)
	{
		InstancedStaticMeshComponent->RemoveInstance(Data.GetVoxelInstanceIndex(InLocalPoint));
	}
}

void AATVoxelChunk::BreakVoxelsAtLocalPoints(const TArray<FIntVector>& InLocalPoints)
{
	TArray<int32> BreakableInstanceIndices;

	for (const FIntVector& SampleLocalPoint : InLocalPoints)
	{
		const FVoxelInstanceData& SampleTargetData = Data.GetVoxelInstanceData(SampleLocalPoint);
		if (SampleTargetData.IsTypeDataValid() && !SampleTargetData.TypeData->IsFoundation)
		{
			BreakableInstanceIndices.Add(Data.GetVoxelInstanceIndex(SampleLocalPoint));
		}
	}
	InstancedStaticMeshComponent->RemoveInstances(BreakableInstanceIndices);
}

void AATVoxelChunk::BreakVoxelsWithInstanceIndices(const TArray<int32>& InInstanceIndices)
{
	TArray<int32> BreakableInstanceIndices;

	for (int32 SampleInstanceIndex : InInstanceIndices)
	{
		const FVoxelInstanceData& SampleTargetData = Data.GetVoxelInstanceData(SampleInstanceIndex);
		if (SampleTargetData.IsTypeDataValid() && !SampleTargetData.TypeData->IsFoundation)
		{
			BreakableInstanceIndices.Add(SampleInstanceIndex);
		}
	}
	InstancedStaticMeshComponent->RemoveInstances(BreakableInstanceIndices);
}
//~ End Setters

//~ Begin Data
void AATVoxelChunk::CreateFoundation()
{
	//ensure(Data.FoundationLocalPoints.IsEmpty());
	if (!Data.FoundationLocalPoints.IsEmpty())
	{
		for (const FIntVector& SamplePoint : Data.FoundationLocalPoints)
		{
			Data.RemoveVoxelByPoint(SamplePoint);
		}
	}
	Data.FoundationLocalPoints.Empty(ChunkSize * ChunkSize);

	for (int32 SampleX = 0; SampleX < ChunkSize; ++SampleX)
	{
		for (int32 SampleY = 0; SampleY < ChunkSize; ++SampleY)
		{
			Data.FoundationLocalPoints.Add(FIntVector(SampleX, SampleY, -1));
		}
	}
	SetVoxelsAtLocalPoints(Data.FoundationLocalPoints, FoundationVoxelTypeData);
}

#pragma optimize("", off)

int32 AATVoxelChunk::UpdatePendingAttachmentData(int32 InMaxUpdates)
{
	if (!AttachmentUpdates.PrepareThisTickSelectedInstanceIndices(InMaxUpdates))
	{
		AttachmentUpdates.ResolveThisTickSelectedInstanceIndices();
		return 0;
	}
	static const float SideAttachmentMul = 0.5f;
	static const float TopAttachmentMul = 0.25f;
	static const float BottomAttachmentMul = 1.0f;

	for (int32 SampleInstanceIndex : AttachmentUpdates.GetThisTickSelectedInstanceIndicesConstArray())
	{
		const FIntVector& SamplePoint = Data.GetVoxelInstancePoint(SampleInstanceIndex);
		FVoxelInstanceData& SampleData = Data.GetVoxelInstanceData(SamplePoint);

		if (SampleData.IsTypeDataValid())
		{
			TArray<EATAttachmentDirection> PrevAttachmentDirections = SampleData.AttachmentDirections.Array();

			if (SampleData.TypeData->IsFoundation)
			{
				SampleData.AttachmentDirections = { EATAttachmentDirection::Bottom };
			}
			else
			{
				SampleData.AttachmentDirections.Empty();

				// Front voxel
				UpdatePendingAttachmentData_UpdateFromNeighbor(SamplePoint, FIntVector(1, 0, 0), FVector::ForwardVector, EAxis::X, 1.0f, SideAttachmentMul, EATAttachmentDirection::Back, EATAttachmentDirection::Front);

				// Back voxel
				UpdatePendingAttachmentData_UpdateFromNeighbor(SamplePoint, FIntVector(-1, 0, 0), FVector::BackwardVector, EAxis::X, -1.0f, SideAttachmentMul, EATAttachmentDirection::Front, EATAttachmentDirection::Back);

				// Right voxel
				UpdatePendingAttachmentData_UpdateFromNeighbor(SamplePoint, FIntVector(0, 1, 0), FVector::RightVector, EAxis::Y, 1.0f, SideAttachmentMul, EATAttachmentDirection::Left, EATAttachmentDirection::Right);

				// Left voxel
				UpdatePendingAttachmentData_UpdateFromNeighbor(SamplePoint, FIntVector(0, -1, 0), FVector::LeftVector, EAxis::Y, -1.0f, SideAttachmentMul, EATAttachmentDirection::Right, EATAttachmentDirection::Left);

				// Top voxel
				UpdatePendingAttachmentData_UpdateFromNeighbor(SamplePoint, FIntVector(0, 0, 1), FVector::UpVector, EAxis::Z, 1.0f, TopAttachmentMul, EATAttachmentDirection::Bottom, EATAttachmentDirection::Top);

				// Bottom voxel
				UpdatePendingAttachmentData_UpdateFromNeighbor(SamplePoint, FIntVector(0, 0, -1), FVector::DownVector, EAxis::Z, -1.0f, BottomAttachmentMul, EATAttachmentDirection::Top, EATAttachmentDirection::Bottom);
			}
			if (SampleData.AttachmentDirections.Array() != PrevAttachmentDirections)
			{
				StabilityUpdates.QueueInstanceIndexIfRelevant(SampleInstanceIndex);
			}
			if (bDebugInstancesAttachmentDirection)
			{
				AttachmentUpdates.DebugInstanceAttachmentDirection(SampleData);
			}
		}
		AttachmentUpdates.MarkInstanceIndexAsUpdatedThisTick(SampleInstanceIndex);
	}
	AttachmentUpdates.ResolveThisTickSelectedInstanceIndices();
	return AttachmentUpdates.ResolveThisTickAlreadyUpdatedInstanceIndices();
}

void AATVoxelChunk::UpdatePendingAttachmentData_UpdateFromNeighbor(const FIntVector& InTargetPoint, const FIntVector& InNeighborOffset, const FVector& InAttachmentDirection, EAxis::Type InAxis, float InAxisMul, float InAttachmentStrengthMul, EATAttachmentDirection ToTargetDirection, EATAttachmentDirection ToNeighborDirection)
{
	FVoxelInstanceData& TargetData = Data.GetVoxelInstanceData(InTargetPoint);
	FVoxelInstanceData& NeighborData = Data.GetVoxelInstanceData(InTargetPoint + InNeighborOffset);

	bool bAddNeighborToPending = false;

	if (TargetData.IsTypeDataValid() && NeighborData.IsTypeDataValid())
	{
		if (NeighborData.TypeData->IsFoundation) // Simple case with foundaion neighbor
		{
			TargetData.AttachmentDirections.Add(ToNeighborDirection);
		}
		else
		{
			if (NeighborData.IsAttachmentDataValid()) // Neighbor has some attachment data
			{
				if (NeighborData.IsAttachedTo(ToTargetDirection)) // Neighbor is attached to Target, avoid mutual attachments
				{
					// But also Target is not stable enough to hold Neighbor, need to detatch Neighbor and queue it to update attachments
					if (TargetData.Stability < NeighborData.Stability)
					{
						NeighborData.AttachmentDirections.Remove(ToNeighborDirection);
						AttachmentUpdates.QueueInstanceIndexIfRelevant(NeighborData.SMI_Index);
					}
				}
				else
				{
					if (TargetData.Stability <= NeighborData.Stability) // Attach Target to Neighbor only if Neighbor is stable enough
					{
						TargetData.AttachmentDirections.Add(ToNeighborDirection);
					}
					else // Otherwise Neighbor needs to be attached to Target, queue for it if is not already
					{
						AttachmentUpdates.QueueInstanceIndexIfRelevant(NeighborData.SMI_Index);
					}
				}
			}
			else
			{
				AttachmentUpdates.QueueInstanceIndexIfRelevant(NeighborData.SMI_Index);
			}
		}

	}
	else if (NeighborData.IsTypeDataValid()) // Target is not valid, while Neighbor is
	{
		// Probably update from removing Target, spread it to Neighbor
		AttachmentUpdates.QueueInstanceIndexIfRelevant(NeighborData.SMI_Index);
	}
}

int32 AATVoxelChunk::UpdatePendingStabilityData(int32 InMaxUpdates)
{
	if (!StabilityUpdates.PrepareThisTickSelectedInstanceIndices(InMaxUpdates))
	{
		return 0;
	}
	static const float SideAttachmentMul = 0.4f;
	static const float TopAttachmentMul = 0.2f;
	static const float BottomAttachmentMul = 1.0f;

	for (int32 SampleInstanceIndex : StabilityUpdates.GetThisTickSelectedInstanceIndicesConstArray())
	{
		const FIntVector& SamplePoint = Data.GetVoxelInstancePoint(SampleInstanceIndex);
		FVoxelInstanceData& SampleData = Data.GetVoxelInstanceData(SamplePoint);

		if (SampleData.IsTypeDataValid())
		{
			float PrevStability = SampleData.Stability;
			TArray<int32> AttachedNeighbors;
			
			if (SampleData.TypeData->IsFoundation)
			{
				SampleData.Stability = 1.0f;
			}
			else
			{
				SampleData.Stability = 0.0f;

				// Front voxel
				UpdatePendingStabilityData_UpdateFromNeighbor(SamplePoint, FIntVector(1, 0, 0), FVector::ForwardVector, EAxis::X, 1.0f, SideAttachmentMul, EATAttachmentDirection::Back, EATAttachmentDirection::Front, AttachedNeighbors);

				// Back voxel
				UpdatePendingStabilityData_UpdateFromNeighbor(SamplePoint, FIntVector(-1, 0, 0), FVector::BackwardVector, EAxis::X, -1.0f, SideAttachmentMul, EATAttachmentDirection::Front, EATAttachmentDirection::Back, AttachedNeighbors);

				// Right voxel
				UpdatePendingStabilityData_UpdateFromNeighbor(SamplePoint, FIntVector(0, 1, 0), FVector::RightVector, EAxis::Y, 1.0f, SideAttachmentMul, EATAttachmentDirection::Left, EATAttachmentDirection::Right, AttachedNeighbors);

				// Left voxel
				UpdatePendingStabilityData_UpdateFromNeighbor(SamplePoint, FIntVector(0, -1, 0), FVector::LeftVector, EAxis::Y, -1.0f, SideAttachmentMul, EATAttachmentDirection::Right, EATAttachmentDirection::Left, AttachedNeighbors);

				// Top voxel
				UpdatePendingStabilityData_UpdateFromNeighbor(SamplePoint, FIntVector(0, 0, 1), FVector::UpVector, EAxis::Z, 1.0f, TopAttachmentMul, EATAttachmentDirection::Bottom, EATAttachmentDirection::Top, AttachedNeighbors);

				// Bottom voxel
				UpdatePendingStabilityData_UpdateFromNeighbor(SamplePoint, FIntVector(0, 0, -1), FVector::DownVector, EAxis::Z, -1.0f, BottomAttachmentMul, EATAttachmentDirection::Top, EATAttachmentDirection::Bottom, AttachedNeighbors);
				
				SampleData.Stability = FMath::Min(FMath::Floor(SampleData.Stability * 10.0f) / 10.0f, 1.0f);
			}
			if (FMath::IsNearlyEqual(SampleData.Stability, PrevStability, StabilityUpdatePropagationThreshold))
			{
				// Stability remained unchanged (mostly), don't notify neighbors
			}
			else if (StabilityUpdatePropagationSkipProbability <= 0.0f || FMath::FRand() > StabilityUpdatePropagationSkipProbability)
			{
				// Notify all attached neighbors to check their stability because target's stability changed
				for (int32 SampleNeighborInstanceIndex : AttachedNeighbors)
				{
					StabilityUpdates.QueueInstanceIndexIfRelevant(SampleNeighborInstanceIndex);
				}
			}
			if (bDebugInstancesStabilityValues)
			{
				InstancedStaticMeshComponent->SetCustomDataValue(SampleInstanceIndex, 0, SampleData.Stability, false);
			}
		}
		StabilityUpdates.MarkInstanceIndexAsUpdatedThisTick(SampleInstanceIndex);
	}
	StabilityUpdates.ResolveThisTickSelectedInstanceIndices();
	return StabilityUpdates.ResolveThisTickAlreadyUpdatedInstanceIndices();
}

void AATVoxelChunk::UpdatePendingStabilityData_UpdateFromNeighbor(const FIntVector& InTargetPoint, const FIntVector& InNeighborOffset, const FVector& InAttachmentDirection, EAxis::Type InAxis, float InAxisMul, float InAttachmentStrengthMul, EATAttachmentDirection ToTargetDirection, EATAttachmentDirection ToNeighborDirection, TArray<int32>& InOutAttachedNeighbors)
{
	FVoxelInstanceData& TargetData = Data.GetVoxelInstanceData(InTargetPoint);
	FVoxelInstanceData& NeighborData = Data.GetVoxelInstanceData(InTargetPoint + InNeighborOffset);

	// Only consider update if both instances are valid
	if (TargetData.IsTypeDataValid() && NeighborData.IsTypeDataValid())
	{
		if (NeighborData.TypeData->IsFoundation)
		{
			TargetData.Stability += 1.0f * InAttachmentStrengthMul;
		}
		else
		{
			if (NeighborData.IsAttachmentDataValid())
			{
				if (NeighborData.IsAttachedTo(ToTargetDirection))
				{
					// Neighbor is attached to Target, Target shouldn't get Stability from Neighbor
					InOutAttachedNeighbors.Add(NeighborData.SMI_Index);
				}
				else if (TargetData.IsAttachedTo(ToNeighborDirection))
				{
					// Target is attached to Neighbor, Target should get Stability from Neighbor
					TargetData.Stability += NeighborData.Stability * InAttachmentStrengthMul;
				}
			}
			else
			{
				AttachmentUpdates.QueueInstanceIndexIfRelevant(NeighborData.SMI_Index);
			}
		}
	}
	else if (NeighborData.IsTypeDataValid()) // Target is not valid, while Neighbor is
	{
		// Probably update from removing Target, spread it to Neighbor
		StabilityUpdates.QueueInstanceIndexIfRelevant(NeighborData.SMI_Index);
	}
}

#pragma optimize("", on)

void AATVoxelChunk::OnInstanceIndicesUpdated(UInstancedStaticMeshComponent* InUpdatedComponent, TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData> InIndexUpdates)
{
	if (InUpdatedComponent == InstancedStaticMeshComponent)
	{
		//PendingAttachmentUpdateInstanceIndices.Reserve(PendingAttachmentUpdateInstanceIndices.Num() + InIndexUpdates.Num());

		for (const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData& SampleUpdate : InIndexUpdates)
		{
			bool bQueueUpdate = false;

			switch (SampleUpdate.Type)
			{
				case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Added:
				{
					bQueueUpdate = true;
					break;
				}
				case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Removed:
				case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Cleared:
				case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Destroyed:
				{
					bQueueUpdate = true;
					Data.RemoveVoxelByInstanceIndex(SampleUpdate.Index, true);
					break;
				}
				case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Relocated:
				{
					bQueueUpdate = true;
					Data.RelocateInstanceIndex(SampleUpdate.OldIndex, SampleUpdate.Index);
					AttachmentUpdates.RelocateInstanceIndex(SampleUpdate.OldIndex, SampleUpdate.Index);
					StabilityUpdates.RelocateInstanceIndex(SampleUpdate.OldIndex, SampleUpdate.Index);
					break;
				}
			}
			if (bQueueUpdate)
			{
				AttachmentUpdates.QueueInstanceIndexIfRelevant(SampleUpdate.Index);
				StabilityUpdates.QueueInstanceIndexIfRelevant(SampleUpdate.Index);
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
	//Cache_LocalPoint_To_LocalIndex_Map.Empty();
	//Cache_LocalIndex_To_LocalPoint_Map.Empty();
}
//~ End Cache

//~ Begin Debug
void AATVoxelChunk::BP_CollectDataForGameplayDebugger_Implementation(APlayerController* ForPlayerController, FVoxelChunkDebugData& InOutData) const
{
	// Common
	InOutData.ChunkHighlightTransform = FTransform(FRotator::ZeroRotator, GetChunkCenterWorldLocation(), FVector((float)ChunkSize * VoxelBaseSize * 0.5f));

	InOutData.Label = GetActorLabel();
	InOutData.LabelColor = FColor::MakeRandomSeededColor(GetActorGuid()[0]);

	InOutData.CommonEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Instances Num"), Data.GetLocalPoint_To_InstanceData_Map().Num()));
	InOutData.CommonEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Chunk Size"), ChunkSize));
	InOutData.CommonEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Voxel Base Size"), VoxelBaseSize));
	InOutData.CommonEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Max Updates per Second"), MaxUpdatesPerSecond));

	// Attachments
	InOutData.AttachmentsEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Pending Updates Num"), AttachmentUpdates.GetPendingInstanceIndicesConstArray().Num()));
	InOutData.AttachmentsEntries.Add(FVoxelChunkDebugData_Entry(TEXT("This Tick Updates Num"), AttachmentUpdates.GetThisTickSelectedInstanceIndicesConstArray().Num()));

	// Stability
	InOutData.StabilityEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Pending Updates Num"), StabilityUpdates.GetPendingInstanceIndicesConstArray().Num()));
	InOutData.StabilityEntries.Add(FVoxelChunkDebugData_Entry(TEXT("This Tick Updates Num"), StabilityUpdates.GetThisTickSelectedInstanceIndicesConstArray().Num()));

	// Instance under cursor
	AScWPlayerController* ScWPlayerController = Cast<AScWPlayerController>(ForPlayerController);

	FHitResult ScreenCenterHitResult;
	ScWPlayerController->GetHitResultUnderScreenCenter(TraceTypeQuery_Visibility, false, ScreenCenterHitResult);

	UInstancedStaticMeshComponent* TargetISMC = Cast<UInstancedStaticMeshComponent>(ScreenCenterHitResult.GetComponent());
	if (TargetISMC == InstancedStaticMeshComponent)
	{
		int32 TargetInstanceIndex = ScreenCenterHitResult.Item;
		InOutData.InstanceLabel = FString::Printf(TEXT("Looking at Voxel at %s, instance index %d"), *Data.GetVoxelInstancePoint(TargetInstanceIndex).ToString(), TargetInstanceIndex);
		
		const FVoxelInstanceData& TargetData = Data.GetVoxelInstanceData(TargetInstanceIndex);
		if (TargetData.IsTypeDataValid())
		{
			InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Type Data"), TargetData.TypeData.GetName()));
			InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Health"), TargetData.Health));
			InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Stability"), TargetData.Stability));
			InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Attachment Directions"), EATAttachmentDirection_Utils::CreateStringFromAttachmentDirections(TargetData.AttachmentDirections)));
		}
		InOutData.InstanceHighlightTransform = FTransform(FRotator::ZeroRotator, GetVoxelCenterWorldLocation(Data.GetVoxelInstancePoint(TargetInstanceIndex)), FVector(VoxelBaseSize * 0.5f));
	}
}
//~ End Debug
