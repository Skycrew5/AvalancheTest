// Scientific Ways

#include "World/ATVoxelISMC.h"

#include "World/ATVoxelChunk.h"
#include "World/ATVoxelTypeData.h"

#if DEBUG_VOXELS
	#pragma optimize("", off)
#endif // DEBUG_VOXELS

FVoxelInstanceData UATVoxelISMC::InvalidInstanceData_NonConst = FVoxelInstanceData::Invalid;

UATVoxelISMC::UATVoxelISMC()
{
	bSupportRemoveAtSwap = true;
}

//~ Begin Initialize
void UATVoxelISMC::OnRegister() // UActorComponent
{
	OwnerChunk = GetOwner<AATVoxelChunk>();

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
//~ End Initialize

//~ Begin Getters
const TMap<FIntVector, FVoxelInstanceData>& UATVoxelISMC::GetLocalPoint_To_InstanceData_Map() const
{
	return LocalPoint_To_InstanceData_Map;
}

void UATVoxelISMC::GetAllLocalPoints(TArray<FIntVector>& OutPoints) const
{
	LocalPoint_To_InstanceData_Map.GenerateKeyArray(OutPoints);
}

const FIntVector& UATVoxelISMC::GetVoxelInstancePointAtIndex(int32 InInstanceIndex, const bool bInChecked) const
{
	if (const FIntVector* SamplePoint = InstanceIndex_To_LocalPoint_Map.Find(InInstanceIndex))
	{
		return *SamplePoint;
	}
	else
	{
		ensure(!bInChecked);
		return FIntVector::ZeroValue;
	}
}

FVoxelInstanceData& UATVoxelISMC::GetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const bool bInChecked) const
{
	if (const FVoxelInstanceData* SampleInstanceDataPtr = LocalPoint_To_InstanceData_Map.Find(InPoint))
	{
		return *const_cast<FVoxelInstanceData*>(SampleInstanceDataPtr);
	}
	else
	{
		ensure(!bInChecked);
		return InvalidInstanceData_NonConst;
	}
}

bool UATVoxelISMC::HasVoxelAtPoint(const FIntVector& InPoint) const
{
	return LocalPoint_To_InstanceData_Map.Contains(InPoint);
}
//~ End Getters

//~ Begin Setters
bool UATVoxelISMC::SetVoxelAtPoint(const FIntVector& InPoint, const UATVoxelTypeData* InTypeData, const bool bInForced)
{
	if (!OwnerChunk || !InTypeData)
	{
		ensure(false);
		return false;
	}
	if (bInForced)
	{
		RemoveVoxelAtPoint(InPoint, false);
	}
	else if (HasVoxelAtPoint(InPoint))
	{
		return false;
	}
	ensure(!LocalPoint_To_InstanceData_Map.Contains(InPoint));
	FVoxelInstanceData& NewInstanceData = LocalPoint_To_InstanceData_Map.Add(InPoint, InTypeData->BP_InitializeInstanceData(OwnerChunk, InPoint));

	QueuePointForVisibilityUpdate(InPoint);
	return true;
}

bool UATVoxelISMC::SetVoxelsAtPoints(const TArray<FIntVector>& InPoints, const UATVoxelTypeData* InTypeData, const bool bInForced)
{
	bool bAnySet = false;

	for (const FIntVector& SamplePoint : InPoints)
	{
		bAnySet |= SetVoxelAtPoint(SamplePoint, InTypeData, bInForced);
	}
	return bAnySet;
}

bool UATVoxelISMC::RemoveVoxelAtPoint(const FIntVector& InPoint, const bool bInChecked)
{
	if (bInChecked && !LocalPoint_To_InstanceData_Map.Contains(InPoint))
	{
		ensure(false);
		return false;
	}
	if (FVoxelInstanceData* SampleData = LocalPoint_To_InstanceData_Map.Find(InPoint))
	{
		if (SampleData->HasMesh())
		{
			InstanceIndex_To_LocalPoint_Map.Remove(SampleData->SMI_Index);
			RemoveInstance(SampleData->SMI_Index);
		}
		LocalPoint_To_InstanceData_Map.Remove(InPoint);
		QueuePointForVisibilityUpdate(InPoint);
		return true;
	}
	return false;
}

bool UATVoxelISMC::RemoveVoxelsAtPoints(const TArray<FIntVector>& InPoints, const bool bInChecked)
{
	bool bAnyRemoved = false;

	for (const FIntVector& SamplePoint : InPoints)
	{
		bAnyRemoved |= RemoveVoxelAtPoint(SamplePoint, bInChecked);
	}
	return bAnyRemoved;
}

void UATVoxelISMC::RemoveAllVoxels()
{
	TArray<FIntVector> AllPoints;
	GetAllLocalPoints(AllPoints);
	RemoveVoxelsAtPoints(AllPoints);
}

bool UATVoxelISMC::RelocateInstanceIndex(int32 InPrevIndex, int32 InNewIndex, const bool bInChecked)
{
	if (!InstanceIndex_To_LocalPoint_Map.Contains(InPrevIndex))
	{
		ensure(!bInChecked);
		return false;
	}
	FIntVector TargetInstancePoint;
	InstanceIndex_To_LocalPoint_Map.RemoveAndCopyValue(InPrevIndex, TargetInstancePoint);
	InstanceIndex_To_LocalPoint_Map.Add(InNewIndex, TargetInstancePoint);

	if (!LocalPoint_To_InstanceData_Map.Contains(TargetInstancePoint))
	{
		ensure(!bInChecked);
		return false;
	}
	FVoxelInstanceData& TargetInstanceData = *LocalPoint_To_InstanceData_Map.Find(TargetInstancePoint);

	ensure(TargetInstanceData.SMI_Index == InPrevIndex);
	TargetInstanceData.SMI_Index = InNewIndex;
	return true;
}
//~ End Setters

#if DEBUG_VOXELS
	#pragma optimize("", off)
#endif // DEBUG_VOXELS

//~ Begin Data
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

void UATVoxelISMC::UpdateVoxelsVisibilityState()
{
	if (QueuedVisibilityUpdatePoints.IsEmpty())
	{
		return;
	}
	TArray<FTransform> MeshTransformsToAdd;
	TArray<FIntVector> MeshTransformsToAdd_Points;
	TArray<int32> MeshInstancesToRemove;
	
	for (const FIntVector& SamplePoint : QueuedVisibilityUpdatePoints.GetConstArray())
	{
		if (HasVoxelAtPoint(SamplePoint))
		{
			FVoxelInstanceData& SampleData = GetVoxelInstanceDataAtPoint(SamplePoint);

			if (SampleData.HasMesh()) // Has mesh...
			{
				if (OwnerChunk->IsVoxelAtPointFullyClosed(SamplePoint)) // ...but doesn't need anymore
				{
					MeshInstancesToRemove.Add(SampleData.SMI_Index);
					SampleData.SMI_Index = INDEX_NONE;
				}
			}
			else // Does not have mesh...
			{
				if (!OwnerChunk->IsVoxelAtPointFullyClosed(SamplePoint)) // ...but needs now
				{
					FTransform NewInstanceTransform = FTransform(OwnerChunk->LocalPoint_To_RelativeLocation(SamplePoint));
					MeshTransformsToAdd.Add(NewInstanceTransform);
					MeshTransformsToAdd_Points.Add(SamplePoint);
				}
			}
		}
		else // Point is empty - no instance data
		{
			
		}
	}
	// Remove
	RemoveInstances(MeshInstancesToRemove);

	for (int32 SampleInstanceIndex : MeshInstancesToRemove)
	{
		InstanceIndex_To_LocalPoint_Map.Remove(SampleInstanceIndex);
	}
	// Add
	TArray<int32> AddedMeshInstancesIndices = AddInstances(MeshTransformsToAdd, true);
	ensure(AddedMeshInstancesIndices.Num() == MeshTransformsToAdd_Points.Num());

	for (int32 SampleArrayIndex = 0; SampleArrayIndex < MeshTransformsToAdd.Num(); ++SampleArrayIndex)
	{
		FVoxelInstanceData& SampleData = GetVoxelInstanceDataAtPoint(MeshTransformsToAdd_Points[SampleArrayIndex]);
		SampleData.SMI_Index = AddedMeshInstancesIndices[SampleArrayIndex];
		InstanceIndex_To_LocalPoint_Map.Add(SampleData.SMI_Index, MeshTransformsToAdd_Points[SampleArrayIndex]);
	}
	// Reset queue
	QueuedVisibilityUpdatePoints.Empty();
}
//~ End Data

#if DEBUG_VOXELS
	#pragma optimize("", on)
#endif // DEBUG_VOXELS
