// Scientific Ways

#include "World/ATVoxelChunk.h"

#include "Framework/ScWPlayerController.h"

#include "World/ATVoxelISMC.h"
#include "World/ScWTypes_World.h"
#include "World/ATVoxelTypeData.h"

#if DEBUG_VOXELS
	#pragma optimize("", off)
#endif // DEBUG_VOXELS

FDelegateHandle AATVoxelChunk::InstanceIndexUpdatedDelegateHandle = FDelegateHandle();

AATVoxelChunk::AATVoxelChunk()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	VoxelISMC = CreateDefaultSubobject<UATVoxelISMC>(TEXT("VoxelISMC"));
	VoxelISMC->SetNumCustomDataFloats(3);
	VoxelISMC->SetupAttachment(RootComponent);

	bHandleISMCUpdatesTick = true;

	bHandleVoxelDataUpdatesTick = true;
	ChunkSize = 12;
	VoxelBaseSize = 16.0f;
	bIsOnFoundation = true;
	MaxUpdatesPerSecond = 10000;

	AttachmentUpdates.TargetISMC = VoxelISMC;
	AttachmentUpdates.DebugVoxelCustomData_ThisTickSelected = 1;

	StabilityUpdates.TargetISMC = VoxelISMC;
	StabilityUpdates.DebugVoxelCustomData_ThisTickSelected = 2;

	StabilityUpdatePropagationThreshold = 0.1f;
	StabilityUpdatePropagationSkipProbability = 0.1f;
}

//~ Begin Initialize
void AATVoxelChunk::OnConstruction(const FTransform& InTransform) // AActor
{
	if (!InstanceIndexUpdatedDelegateHandle.IsValid())
	{
		InstanceIndexUpdatedDelegateHandle = FInstancedStaticMeshDelegates::OnInstanceIndexUpdated.AddStatic(&AATVoxelChunk::Static_OnISMInstanceIndicesUpdated);
	}
	Super::OnConstruction(InTransform);

	if (UWorld* World = GetWorld())
	{
		if (World->IsEditorWorld())
		{
			if (bIsOnFoundation)
			{
				CreateFoundation();
			}
		}
	}
}

void AATVoxelChunk::BeginPlay() // AActor
{
	if (UWorld* World = GetWorld())
	{
		if (World->IsGameWorld())
		{
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

	int32 UpdatesLeft = FMath::CeilToInt((float)MaxUpdatesPerSecond * InDeltaSeconds);

	if (bHandleISMCUpdatesTick)
	{
		HandleISMCUpdatesTick(UpdatesLeft);
	}
	if (bHandleVoxelDataUpdatesTick)
	{
		HandleVoxelDataUpdatesTick(UpdatesLeft);
	}
}

void AATVoxelChunk::EndPlay(const EEndPlayReason::Type InReason) // AActor
{
	

	Super::EndPlay(InReason);
}
//~ End Initialize

//~ Begin Components
void AATVoxelChunk::HandleISMCUpdatesTick(int32& InOutMaxUpdates)
{
	VoxelISMC->UpdateVoxelsVisibilityState();
}
//~ End Components

//~ Begin Locations
FIntVector AATVoxelChunk::RelativeLocation_To_LocalPoint(const FVector& InRelativeLocation) const
{
	FVector VoxelScaledRelativeLocation = (InRelativeLocation / VoxelBaseSize);
	return FIntVector(FMath::CeilToInt(VoxelScaledRelativeLocation.X), FMath::CeilToInt(VoxelScaledRelativeLocation.Y), FMath::CeilToInt(VoxelScaledRelativeLocation.Z));
}

FIntVector AATVoxelChunk::WorldLocation_To_LocalPoint(const FVector& InWorldLocation) const
{
	return RelativeLocation_To_LocalPoint(GetActorTransform().InverseTransformPosition(InWorldLocation));
}

FVector AATVoxelChunk::LocalPoint_To_RelativeLocation(const FIntVector& InPoint) const
{
	if (bOffsetMeshToCenter)
	{
		return FVector(InPoint * VoxelBaseSize) + FVector(VoxelBaseSize * 0.5f, VoxelBaseSize * 0.5f, VoxelBaseSize * 0.5f);
	}
	else
	{
		return FVector(InPoint * VoxelBaseSize);
	}
}

FVector AATVoxelChunk::LocalPoint_To_WorldLocation(const FIntVector& InPoint) const
{
	return GetActorTransform().TransformPosition(LocalPoint_To_RelativeLocation(InPoint));
}

FVector AATVoxelChunk::GetChunkCenterWorldLocation() const
{
	return GetActorLocation() + FVector((float)ChunkSize * VoxelBaseSize * 0.5f);
}

FVector AATVoxelChunk::GetVoxelCenterWorldLocation(const FIntVector& InPoint) const
{
	return GetActorLocation() + FVector(InPoint * VoxelBaseSize) + FVector(VoxelBaseSize * 0.5f, VoxelBaseSize * 0.5f, VoxelBaseSize * 0.5f);
}
//~ End Locations

//~ Begin Getters
const FVoxelInstanceData& AATVoxelChunk::GetVoxelInstanceDataAtPoint(const FIntVector& InPoint) const
{
	return VoxelISMC->GetVoxelInstanceDataAtPoint(InPoint);
}

bool AATVoxelChunk::HasVoxelAtPoint(const FIntVector& InPoint) const
{
	return VoxelISMC->HasVoxelAtPoint(InPoint);
}

int32 AATVoxelChunk::GetVoxelNeighborsNumAtLocalPoint(const FIntVector& InPoint) const
{
	int32 OutNum = 0;

	if (HasVoxelAtPoint(InPoint + FIntVector(1, 0, 0))) OutNum += 1;
	if (HasVoxelAtPoint(InPoint + FIntVector(-1, 0, 0))) OutNum += 1;
	if (HasVoxelAtPoint(InPoint + FIntVector(0, 1, 0))) OutNum += 1;
	if (HasVoxelAtPoint(InPoint + FIntVector(0, -1, 0))) OutNum += 1;
	if (HasVoxelAtPoint(InPoint + FIntVector(0, 0, 1))) OutNum += 1;
	if (HasVoxelAtPoint(InPoint + FIntVector(0, 0, -1))) OutNum += 1;
	return OutNum;
}

bool AATVoxelChunk::IsVoxelAtPointFullyClosed(const FIntVector& InPoint) const
{
	if (!HasVoxelAtPoint(InPoint + FIntVector(1, 0, 0)))
	{
		return false;
	}
	if (!HasVoxelAtPoint(InPoint + FIntVector(-1, 0, 0)))
	{
		return false;
	}
	if (!HasVoxelAtPoint(InPoint + FIntVector(0, 1, 0)))
	{
		return false;
	}
	if (!HasVoxelAtPoint(InPoint + FIntVector(0, -1, 0)))
	{
		return false;
	}
	if (!HasVoxelAtPoint(InPoint + FIntVector(0, 0, 1)))
	{
		return false;
	}
	if (!HasVoxelAtPoint(InPoint + FIntVector(0, 0, -1)))
	{
		return false;
	}
	return true;
}

void AATVoxelChunk::GetAllPointsInRadius(const FIntVector& InCenterPoint, int32 InRadius, TArray<FIntVector>& OutPoints) const
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
//~ End Getters

//~ Begin Setters
bool AATVoxelChunk::SetVoxelAtLocalPoint(const FIntVector& InPoint, const UATVoxelTypeData* InTypeData, const bool bInForced)
{
	return VoxelISMC->SetVoxelAtPoint(InPoint, InTypeData, bInForced);
}

bool AATVoxelChunk::SetVoxelsAtLocalPoints(const TArray<FIntVector>& InPoints, const UATVoxelTypeData* InTypeData, const bool bInForced)
{
	return VoxelISMC->SetVoxelsAtPoints(InPoints, InTypeData, bInForced);
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
	SetVoxelsAtLocalPoints(VoxelPoints, InTypeData, true);
}

void AATVoxelChunk::RemoveAllVoxels()
{
	VoxelISMC->RemoveAllVoxels();
}

bool AATVoxelChunk::BreakVoxelAtLocalPoint(const FIntVector& InPoint, const bool bInForced)
{
	if (bInForced)
	{
		return VoxelISMC->RemoveVoxelAtPoint(InPoint, false);
	}
	else
	{
		const FVoxelInstanceData& TargetData = VoxelISMC->GetVoxelInstanceDataAtPoint(InPoint);
		if (TargetData.IsTypeDataValid() && !TargetData.TypeData->IsFoundation)
		{
			return VoxelISMC->RemoveVoxelAtPoint(InPoint);
		}
		return false;
	}
}

bool AATVoxelChunk::BreakVoxelsAtLocalPoints(const TArray<FIntVector>& InPoints, const bool bInForced)
{
	return VoxelISMC->RemoveVoxelsAtPoints(InPoints, bInForced);
}

void AATVoxelChunk::CreateFoundation()
{
	ensure(FoundationVoxelTypeData);
	if (!FoundationVoxelTypeData)
	{
		return;
	}
	BreakVoxelsAtLocalPoints(VoxelISMC->FoundationLocalPoints, true);
	VoxelISMC->FoundationLocalPoints.Empty(ChunkSize * ChunkSize);

	for (int32 SampleX = 0; SampleX < ChunkSize; ++SampleX)
	{
		for (int32 SampleY = 0; SampleY < ChunkSize; ++SampleY)
		{
			VoxelISMC->FoundationLocalPoints.Add(FIntVector(SampleX, SampleY, -1));
		}
	}
	SetVoxelsAtLocalPoints(VoxelISMC->FoundationLocalPoints, FoundationVoxelTypeData);
}
//~ End Setters

//~ Begin Voxel Data
void AATVoxelChunk::HandleVoxelDataUpdatesTick(int32& InOutMaxUpdates)
{
	InOutMaxUpdates -= UpdatePendingAttachmentData(InOutMaxUpdates);
	InOutMaxUpdates -= UpdatePendingStabilityData(InOutMaxUpdates);
}

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
		const FIntVector& SamplePoint = VoxelISMC->GetVoxelInstancePointAtIndex(SampleInstanceIndex);
		FVoxelInstanceData& SampleData = VoxelISMC->GetVoxelInstanceDataAtPoint(SamplePoint);

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
	FVoxelInstanceData& TargetData = VoxelISMC->GetVoxelInstanceDataAtPoint(InTargetPoint);
	FVoxelInstanceData& NeighborData = VoxelISMC->GetVoxelInstanceDataAtPoint(InTargetPoint + InNeighborOffset, false);

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
		const FIntVector& SamplePoint = VoxelISMC->GetVoxelInstancePointAtIndex(SampleInstanceIndex);
		FVoxelInstanceData& SampleData = VoxelISMC->GetVoxelInstanceDataAtPoint(SamplePoint);

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
				VoxelISMC->SetCustomDataValue(SampleInstanceIndex, DebugVoxelCustomData_Stability, SampleData.Stability, false);
			}
		}
		StabilityUpdates.MarkInstanceIndexAsUpdatedThisTick(SampleInstanceIndex);
	}
	StabilityUpdates.ResolveThisTickSelectedInstanceIndices();
	return StabilityUpdates.ResolveThisTickAlreadyUpdatedInstanceIndices();
}

void AATVoxelChunk::UpdatePendingStabilityData_UpdateFromNeighbor(const FIntVector& InTargetPoint, const FIntVector& InNeighborOffset, const FVector& InAttachmentDirection, EAxis::Type InAxis, float InAxisMul, float InAttachmentStrengthMul, EATAttachmentDirection ToTargetDirection, EATAttachmentDirection ToNeighborDirection, TArray<int32>& InOutAttachedNeighbors)
{
	FVoxelInstanceData& TargetData = VoxelISMC->GetVoxelInstanceDataAtPoint(InTargetPoint);
	FVoxelInstanceData& NeighborData = VoxelISMC->GetVoxelInstanceDataAtPoint(InTargetPoint + InNeighborOffset);

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

void AATVoxelChunk::Static_OnISMInstanceIndicesUpdated(UInstancedStaticMeshComponent* InUpdatedComponent, TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData> InIndexUpdates)
{
	check(InUpdatedComponent);

	if (AATVoxelChunk* TargetChunk = InUpdatedComponent->GetOwner<AATVoxelChunk>())
	{
		TargetChunk->HandleInstanceIndicesUpdates(InIndexUpdates);
	}
}

void AATVoxelChunk::HandleInstanceIndicesUpdates(const TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData>& InIndexUpdates)
{
	for (const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData& SampleUpdate : InIndexUpdates)
	{
		//bool bQueueUpdate = false;

		switch (SampleUpdate.Type)
		{
			/*case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Added:
			{
				bQueueUpdate = true;
				break;
			}
			case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Removed:
			case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Cleared:
			//case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Destroyed:
			{
				if (SampleUpdate.Index != INDEX_NONE)
				{
					bQueueUpdate = true;
					VoxelISMC->RemoveVoxelFromComponentUpdateCallback(SampleUpdate.Index);
				}
				break;
			}*/
			case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Relocated:
			{
				//bQueueUpdate = true;
				VoxelISMC->RelocateInstanceIndex(SampleUpdate.OldIndex, SampleUpdate.Index);
				//AttachmentUpdates.RelocateInstanceIndex(SampleUpdate.OldIndex, SampleUpdate.Index);
				//StabilityUpdates.RelocateInstanceIndex(SampleUpdate.OldIndex, SampleUpdate.Index);
				break;
			}
		}
		/*if (bQueueUpdate)
		{
			AttachmentUpdates.QueueInstanceIndexIfRelevant(SampleUpdate.Index);
			StabilityUpdates.QueueInstanceIndexIfRelevant(SampleUpdate.Index);
		}*/
	}
}
//~ End Voxel Data

//~ Begin Debug
void AATVoxelChunk::BP_CollectDataForGameplayDebugger_Implementation(APlayerController* ForPlayerController, FVoxelChunkDebugData& InOutData) const
{
	// Common
	InOutData.ChunkHighlightTransform = FTransform(FRotator::ZeroRotator, GetChunkCenterWorldLocation(), FVector((float)ChunkSize * VoxelBaseSize * 0.5f));

	InOutData.Label = GetActorLabel();
	InOutData.LabelColor = FColor::MakeRandomSeededColor(GetActorGuid()[0]);

	InOutData.CommonEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Instances Num"), VoxelISMC->GetLocalPoint_To_InstanceData_Map().Num()));
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
	if (TargetISMC == VoxelISMC)
	{
		int32 TargetInstanceIndex = ScreenCenterHitResult.Item;
		const FIntVector& TargetPoint = VoxelISMC->GetVoxelInstancePointAtIndex(TargetInstanceIndex);
		InOutData.InstanceLabel = FString::Printf(TEXT("Looking at Voxel at %s, instance index %d"), *TargetPoint.ToString(), TargetInstanceIndex);
		
		const FVoxelInstanceData& TargetData = VoxelISMC->GetVoxelInstanceDataAtPoint(TargetPoint);
		if (TargetData.IsTypeDataValid())
		{
			InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Type Data"), TargetData.TypeData.GetName()));
			InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Health"), TargetData.Health));
			InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Stability"), TargetData.Stability));
			InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Attachment Directions"), EATAttachmentDirection_Utils::CreateStringFromAttachmentDirections(TargetData.AttachmentDirections)));
		}
		InOutData.InstanceHighlightTransform = FTransform(FRotator::ZeroRotator, GetVoxelCenterWorldLocation(VoxelISMC->GetVoxelInstancePointAtIndex(TargetInstanceIndex)), FVector(VoxelBaseSize * 0.5f));
	}
}
//~ End Debug

#if DEBUG_VOXELS
	#pragma optimize("", on)
#endif // DEBUG_VOXELS
