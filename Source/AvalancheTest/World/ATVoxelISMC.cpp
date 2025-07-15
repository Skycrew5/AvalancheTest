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

	SetCollisionProfileName(TEXT("Voxel"));
	SetNumCustomDataFloats(4);

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

void UATVoxelISMC::BP_InitComponent_Implementation(AATVoxelChunk* InOwnerChunk, const UATVoxelTypeData* InTypeData)
{
	ensureReturn(InOwnerChunk);
	OwnerChunk = InOwnerChunk;

	ensureReturn(InTypeData);
	VoxelTypeData = InTypeData;

	ensureReturn(InTypeData->StaticMesh);
	SetStaticMesh(InTypeData->StaticMesh);

	for (const auto& SampleIndexAndMaterial : InTypeData->StaticMeshOverrideMaterials)
	{
		SetMaterial(SampleIndexAndMaterial.Key, SampleIndexAndMaterial.Value);
	}
}
//~ End Initialize

//~ Begin Getters
bool UATVoxelISMC::HasVoxelOfThisTypeAtPoint(const FIntVector& InPoint) const
{
	ensureReturn(VoxelTypeData, false);
	return GetVoxelInstanceDataAtPoint(InPoint, false).TypeData == VoxelTypeData;
}

bool UATVoxelISMC::HasVoxelOfAnyTypeAtPoint(const FIntVector& InPoint) const
{
	return GetVoxelInstanceDataAtPoint(InPoint, false).IsTypeDataValid();
}

bool UATVoxelISMC::CouldCreateMeshAtPoint(const FIntVector& InPoint) const
{
	return PossibleMeshPointsSet.Contains(InPoint);
}

FVoxelInstanceData& UATVoxelISMC::GetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const bool bInChecked) const
{
	ensureReturn(OwnerChunk, const_cast<FVoxelInstanceData&>(FVoxelInstanceData::Invalid));
	return OwnerChunk->GetVoxelInstanceDataAtPoint(InPoint, bInChecked);
}

FIntVector UATVoxelISMC::GetPointOfMeshIndex(int32 InMeshIndex, const bool bInChecked) const
{
	if (!IsValidInstance(InMeshIndex))
	{
		ensure(!bInChecked);
		return FIntVector::ZeroValue;
	}
	FTransform TargetWorldTransform;
	GetInstanceTransform(InMeshIndex, TargetWorldTransform, true);
	return UATWorldFunctionLibrary::WorldLocation_To_Point3D( TargetWorldTransform.GetLocation(), GetVoxelSize());
}

bool UATVoxelISMC::IsVoxelAtPointFullyClosed(const FIntVector& InPoint) const
{
	if (HasVoxelOfAnyTypeAtPoint(InPoint + FIntVector(1, 0, 0)) &&
		HasVoxelOfAnyTypeAtPoint(InPoint + FIntVector(-1, 0, 0)) &&
		HasVoxelOfAnyTypeAtPoint(InPoint + FIntVector(0, 1, 0)) &&
		HasVoxelOfAnyTypeAtPoint(InPoint + FIntVector(0, -1, 0)) &&
		HasVoxelOfAnyTypeAtPoint(InPoint + FIntVector(0, 0, 1)) &&
		HasVoxelOfAnyTypeAtPoint(InPoint + FIntVector(0, 0, -1)))
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
		PossibleMeshPointsSet.Add(InPoint);
	}
	else
	{
		PossibleMeshPointsSet.Remove(InPoint);
	}
	QueuePointForVisibilityUpdate(InPoint);
	TryQueuePointForDebugUpdate(InPoint);
}

bool UATVoxelISMC::HandleBreakVoxelAtPoint(const FIntVector& InPoint, const FVoxelBreakData& InBreakData)
{
	HandleSetVoxelInstanceDataAtPoint(InPoint, FVoxelInstanceData::Invalid);

	if (InBreakData.bNotify)
	{
		ensureReturn(OwnerChunk, true);
		OwnerChunk->OnBreakVoxelAtPoint.Broadcast(this, InPoint, InBreakData);
	}
	return true;
}

bool UATVoxelISMC::HandleMeshInstanceIndexRelocated(int32 InPrevIndex, int32 InNewIndex)
{
	if (IsValidInstance(InPrevIndex))
	{
		FIntVector NewPrevIndexPoint = GetPointOfMeshIndex(InPrevIndex, true);
		Point_To_MeshIndex_Map.Add(NewPrevIndexPoint, InPrevIndex);
	}
	FIntVector NewNewIndexPoint = GetPointOfMeshIndex(InNewIndex, true);
	Point_To_MeshIndex_Map.Add(NewNewIndexPoint, InNewIndex);
	return true;
}
//~ End Setters

//~ Begin Data
void UATVoxelISMC::HandleUpdates()
{
	if (bEnableVoxelsVisibilityUpdates)
	{
		UpdateVoxelsVisibilityState();
	}
	UpdateVoxelsDebugState();

	INC_MEMORY_STAT_BY(STAT_VoxelComponents_Point_To_MeshIndex_Map, Point_To_MeshIndex_Map.GetAllocatedSize());
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

void UATVoxelISMC::UpdateVoxelsVisibilityState(const bool bInIgnoreTimeBugdet)
{
	if (QueuedVisibilityUpdatePoints.IsEmpty())
	{
		return;
	}
	TArray<FTransform> MeshTransformsToAdd;
	TArray<FIntVector> PointsToAdd;

	TArray<int32> MeshInstancesToRemove;
	TArray<FIntVector> PointsToRemove;

	ensureReturn(OwnerChunk);
	while ((bInIgnoreTimeBugdet || !OwnerChunk->IsThisTickUpdatesTimeBudgetExceeded()) && !QueuedVisibilityUpdatePoints.IsEmpty())
	{
		FIntVector SamplePoint = QueuedVisibilityUpdatePoints.Pop();
		
		if (CouldCreateMeshAtPoint(SamplePoint))
		{
			if (int32* SampleIndexPtr = Point_To_MeshIndex_Map.Find(SamplePoint)) // Has mesh...
			{
				if (IsVoxelAtPointFullyClosed(SamplePoint)) // ...but doesn't need anymore
				{
					MeshInstancesToRemove.Add(*SampleIndexPtr);
					PointsToRemove.Add(SamplePoint);
				}
			}
			else // Does not have mesh...
			{
				if (!IsVoxelAtPointFullyClosed(SamplePoint)) // ...but needs now
				{
					FTransform NewInstanceTransform = FTransform(UATWorldFunctionLibrary::Point3D_To_RelativeLocation(this, SamplePoint));
					MeshTransformsToAdd.Add(NewInstanceTransform);
					PointsToAdd.Add(SamplePoint);
				}
			}
		}
		else // Point is empty or not of this component
		{
			if (int32* SampleIndexPtr = Point_To_MeshIndex_Map.Find(SamplePoint)) // But has mesh to remove
			{
				MeshInstancesToRemove.Add(*SampleIndexPtr);
				PointsToRemove.Add(SamplePoint);
			}
		}
	}
	// Remove
	ensure(MeshInstancesToRemove.Num() == PointsToRemove.Num());

	for (const FIntVector& SamplePoint : PointsToRemove)
	{
		Point_To_MeshIndex_Map.Remove(SamplePoint);
	}
	RemoveInstances(MeshInstancesToRemove);

	// Add
	TArray<int32> AddedIndices = AddInstances(MeshTransformsToAdd, true);
	ensure(AddedIndices.Num() == PointsToAdd.Num());
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
				FIntVector TargetPoint = GetPointOfMeshIndex(SampleUpdate.Index);

				ensure(!Point_To_MeshIndex_Map.Contains(TargetPoint));
				Point_To_MeshIndex_Map.Add(TargetPoint, SampleUpdate.Index);

				bQueueDebug = true;
				break;
			}
			case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Removed:
			case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Cleared:
			case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Destroyed:
			{
				//FIntVector TargetPoint = GetPointOfMeshIndex(SampleUpdate.Index);
				//Point_To_MeshIndex_Map.Remove(TargetPoint);
				break;
			}
			case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Relocated:
			{
				HandleMeshInstanceIndexRelocated(SampleUpdate.OldIndex, SampleUpdate.Index);
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
void UATVoxelISMC::UpdateVoxelsDebugState()
{
	if (QueuedDebugUpdatePoints.IsEmpty())
	{
		return;
	}
	ensureReturn(OwnerChunk);
	while (!OwnerChunk->IsThisTickUpdatesTimeBudgetExceeded() && !QueuedDebugUpdatePoints.IsEmpty())
	{
		FIntVector SamplePoint = QueuedDebugUpdatePoints.Pop();

		Debug_UpdateStabilityValueAtPoint(SamplePoint);
		Debug_UpdateHealthValueAtPoint(SamplePoint);
	}
}

void UATVoxelISMC::Debug_UpdateStabilityValueAtPoint(const FIntVector& InPoint)
{
	if (bDebugStabilityValues && HasMeshAtPoint(InPoint))
	{
		const FVoxelInstanceData& VoxeInstanceData = GetVoxelInstanceDataAtPoint(InPoint, false);
		SetCustomDataValue(GetMeshIndexAtPoint(InPoint), DebugVoxelCustomData_Stability, VoxeInstanceData.Stability, true);
	}
}

void UATVoxelISMC::Debug_UpdateHealthValueAtPoint(const FIntVector& InPoint)
{
	if (bDebugHealthValues && HasMeshAtPoint(InPoint))
	{
		const FVoxelInstanceData& VoxeInstanceData = GetVoxelInstanceDataAtPoint(InPoint, false);
		SetCustomDataValue(GetMeshIndexAtPoint(InPoint), DebugVoxelCustomData_Health, VoxeInstanceData.Health, true);
	}
}
//~ End Debug

#if DEBUG_VOXELS
	#pragma optimize("", on)
#endif // DEBUG_VOXELS
