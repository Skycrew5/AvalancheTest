// Scientific Ways

#include "World/ATVoxelISMC.h"

#include "World/ATVoxelChunk.h"
#include "World/ATVoxelTypeData.h"
#include "World/ATWorldFunctionLibrary.h"

#if DEBUG_VOXELS
	#pragma optimize("", off)
#endif // DEBUG_VOXELS

FDelegateHandle UATVoxelISMC::InstanceIndexUpdatedDelegateHandle = FDelegateHandle();

UATVoxelISMC::UATVoxelISMC()
{
	PrimaryComponentTick.bCanEverTick = false;

	bSupportRemoveAtSwap = true;

	bDebugStabilityValues = true;
	bDebugHealthValues = true;

	DebugVoxelCustomData_Stability = 0;
	DebugVoxelCustomData_Health = 3;
}

//~ Begin Initialize
void UATVoxelISMC::OnRegister() // UActorComponent
{
	if (!InstanceIndexUpdatedDelegateHandle.IsValid())
	{
		InstanceIndexUpdatedDelegateHandle = FInstancedStaticMeshDelegates::OnInstanceIndexUpdated.AddStatic(&UATVoxelISMC::Static_OnISMInstanceIndicesUpdated);
	}
	Super::OnRegister();
}

void UATVoxelISMC::BeginPlay() // UActorComponent
{
	Super::BeginPlay();

	
}

void UATVoxelISMC::EndPlay(const EEndPlayReason::Type InReason) // UActorComponent
{
	Super::EndPlay(InReason);


}

void UATVoxelISMC::BP_InitComponent_Implementation(AATVoxelChunk* InOwnerChunk)
{
	OwnerChunk = InOwnerChunk;
}
//~ End Initialize

//~ Begin Getters
FVoxelInstanceData& UATVoxelISMC::GetVoxelInstanceDataAtPoint(const FIntVector& InPoint) const
{
	if (const FVoxelInstanceData* SampleData = Point_To_VoxelInstanceData_Map.Find(InPoint))
	{
		return const_cast<FVoxelInstanceData&>(*SampleData);
	}
	return const_cast<FVoxelInstanceData&>(FVoxelInstanceData::Invalid);
}

bool UATVoxelISMC::IsVoxelAtPointFullyClosed(const FIntVector& InPoint) const
{
	if (HasVoxelInstanceDataAtPoint(InPoint + FIntVector(1, 0, 0)) &&
		HasVoxelInstanceDataAtPoint(InPoint + FIntVector(-1, 0, 0)) &&
		HasVoxelInstanceDataAtPoint(InPoint + FIntVector(0, 1, 0)) &&
		HasVoxelInstanceDataAtPoint(InPoint + FIntVector(0, -1, 0)) &&
		HasVoxelInstanceDataAtPoint(InPoint + FIntVector(0, 0, 1)) &&
		HasVoxelInstanceDataAtPoint(InPoint + FIntVector(0, 0, -1)))
	{
		return true;
	}
	return false;
}

float UATVoxelISMC::GetVoxelSize() const
{
	ensureReturn(OwnerChunk, 16.0f);
	return OwnerChunk->GetVoxelSize();
}
//~ End Getters

//~ Begin Setters
void UATVoxelISMC::HandleSetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const FVoxelInstanceData& InVoxelInstanceData)
{
	if (InVoxelInstanceData.IsTypeDataValid())
	{
		Point_To_VoxelInstanceData_Map.Add(InPoint, InVoxelInstanceData);
	}
	else
	{
		Point_To_VoxelInstanceData_Map.Remove(InPoint);
	}
	QueuePointForVisibilityUpdate(InPoint);
	TryQueuePointForDebugUpdate(InPoint);
}

bool UATVoxelISMC::HandleBreakVoxelAtPoint(const FIntVector& InPoint, const bool bInNotify)
{
	HandleSetVoxelInstanceDataAtPoint(InPoint, FVoxelInstanceData::Invalid);

	if (bInNotify)
	{
		OnBreakVoxelAtPoint.Broadcast(InPoint);
	}
	return true;
}

bool UATVoxelISMC::RelocateMeshInstanceIndex(int32 InPrevIndex, int32 InNewIndex, const bool bInChecked)
{
	if (!Point_To_MeshIndex_MirroredMap.ContainsValue(InPrevIndex))
	{
		ensure(!bInChecked);
		return false;
	}
	FIntVector PrevMeshIndexPoint = *Point_To_MeshIndex_MirroredMap.FindByValue(InPrevIndex);
	Point_To_MeshIndex_MirroredMap.ReplaceValue(InPrevIndex, InNewIndex);

	ensure(Point_To_VoxelInstanceData_Map.Contains(PrevMeshIndexPoint));
	return true;
}
//~ End Setters

//~ Begin Data
void UATVoxelISMC::HandleUpdates(int32& InOutUpdatesLeft)
{
	UpdateVoxelsVisibilityState(InOutUpdatesLeft);
	UpdateVoxelsDebugState(InOutUpdatesLeft);
}

void UATVoxelISMC::QueuePointForVisibilityUpdate(const FIntVector& InPoint, const bool bInQueueNeighborsToo)
{
	QueuedVisibilityUpdatePoints.Add(InPoint);

	if (bInQueueNeighborsToo)
	{
		QueuedVisibilityUpdatePoints.Add(InPoint + FIntVector(1, 0, 0));
		QueuedVisibilityUpdatePoints.Add(InPoint + FIntVector(-1, 0, 0));
		QueuedVisibilityUpdatePoints.Add(InPoint + FIntVector(0, 1, 0));
		QueuedVisibilityUpdatePoints.Add(InPoint + FIntVector(0, -1, 0));
		QueuedVisibilityUpdatePoints.Add(InPoint + FIntVector(0, 0, 1));
		QueuedVisibilityUpdatePoints.Add(InPoint + FIntVector(0, 0, -1));
	}
}

void UATVoxelISMC::UpdateVoxelsVisibilityState(int32& InOutUpdatesLeft)
{
	if (QueuedVisibilityUpdatePoints.IsEmpty())
	{
		return;
	}
	TArray<FTransform> MeshTransformsToAdd;
	TArray<FIntVector> MeshTransformsToAdd_Points;
	TArray<int32> MeshInstancesToRemove;

	while (InOutUpdatesLeft > 0 && !QueuedVisibilityUpdatePoints.IsEmpty())
	{
		InOutUpdatesLeft -= 1;
		FIntVector SamplePoint = QueuedVisibilityUpdatePoints.Pop();
		
		if (HasVoxelInstanceDataAtPoint(SamplePoint))
		{
			if (int32* SampleIndexPtr = Point_To_MeshIndex_MirroredMap.FindByKey(SamplePoint)) // Has mesh...
			{
				if (IsVoxelAtPointFullyClosed(SamplePoint)) // ...but doesn't need anymore
				{
					MeshInstancesToRemove.Add(*SampleIndexPtr);
				}
			}
			else // Does not have mesh...
			{
				if (!IsVoxelAtPointFullyClosed(SamplePoint)) // ...but needs now
				{
					FTransform NewInstanceTransform = FTransform(UATWorldFunctionLibrary::Point_To_RelativeLocation(this, SamplePoint));
					MeshTransformsToAdd.Add(NewInstanceTransform);
					MeshTransformsToAdd_Points.Add(SamplePoint);
				}
			}
		}
		else // Point is empty - no voxel instance data
		{
			if (int32* SampleIndexPtr = Point_To_MeshIndex_MirroredMap.FindByKey(SamplePoint)) // But has mesh to remove
			{
				MeshInstancesToRemove.Add(*SampleIndexPtr);
			}
		}
	}
	// Remove
	RemoveInstances(MeshInstancesToRemove);

	// Add
	TArray<int32> AddedIndices = AddInstances(MeshTransformsToAdd, true);
	ensure(AddedIndices.Num() == MeshTransformsToAdd_Points.Num());

	for (int32 SampleArrayIndex = 0; SampleArrayIndex < MeshTransformsToAdd.Num(); ++SampleArrayIndex)
	{
		const FIntVector& SamplePoint = MeshTransformsToAdd_Points[SampleArrayIndex];

		Point_To_MeshIndex_MirroredMap.AddPair(SamplePoint, AddedIndices[SampleArrayIndex]);
		TryQueuePointForDebugUpdate(SamplePoint);
	}
}
//~ End Data

//~ Begin Meshes
void UATVoxelISMC::Static_OnISMInstanceIndicesUpdated(UInstancedStaticMeshComponent* InUpdatedComponent, TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData> InIndexUpdates)
{
	ensureReturn(InUpdatedComponent);

	if (UATVoxelISMC* TargetVoxelComponent = Cast<UATVoxelISMC>(InUpdatedComponent))
	{
		TargetVoxelComponent->HandleInstanceIndicesUpdates(InIndexUpdates);
	}
}

void UATVoxelISMC::HandleInstanceIndicesUpdates(const TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData>& InIndexUpdates)
{
	for (const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData& SampleUpdate : InIndexUpdates)
	{
		bool bQueueDebug = false;

		switch (SampleUpdate.Type)
		{
			case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Added:
			{
				bQueueDebug = true;
				break;
			}
			case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Removed:
			case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Cleared:
			case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Destroyed:
			{
				Point_To_MeshIndex_MirroredMap.RemoveByValue(SampleUpdate.Index);
				break;
			}
			case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Relocated:
			{
				RelocateMeshInstanceIndex(SampleUpdate.OldIndex, SampleUpdate.Index);
				bQueueDebug = true;
				break;
			}
		}
		if (bQueueDebug)
		{
			TryQueueMeshIndexForDebugUpdate(SampleUpdate.Index);
		}
	}
}
//~ End Meshes

//~ Begin Debug
void UATVoxelISMC::UpdateVoxelsDebugState(int32& InOutUpdatesLeft)
{
	if (QueuedDebugUpdatePoints.IsEmpty())
	{
		return;
	}
	while (InOutUpdatesLeft > 0 && !QueuedDebugUpdatePoints.IsEmpty())
	{
		InOutUpdatesLeft -= 1;
		FIntVector SamplePoint = QueuedDebugUpdatePoints.Pop();

		Debug_UpdateStabilityValueAtPoint(SamplePoint);
		Debug_UpdateHealthValueAtPoint(SamplePoint);
	}
}

void UATVoxelISMC::Debug_UpdateStabilityValueAtPoint(const FIntVector& InPoint)
{
	if (bDebugStabilityValues && HasMeshAtPoint(InPoint))
	{
		const FVoxelInstanceData& VoxeInstanceData = GetVoxelInstanceDataAtPoint(InPoint);
		SetCustomDataValue(GetMeshIndexAtPoint(InPoint), DebugVoxelCustomData_Stability, VoxeInstanceData.Stability, true);
	}
}

void UATVoxelISMC::Debug_UpdateHealthValueAtPoint(const FIntVector& InPoint)
{
	if (bDebugHealthValues && HasMeshAtPoint(InPoint))
	{
		const FVoxelInstanceData& VoxeInstanceData = GetVoxelInstanceDataAtPoint(InPoint);
		SetCustomDataValue(GetMeshIndexAtPoint(InPoint), DebugVoxelCustomData_Health, VoxeInstanceData.Health, true);
	}
}
//~ End Debug

#if DEBUG_VOXELS
	#pragma optimize("", on)
#endif // DEBUG_VOXELS
