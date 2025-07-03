// Scientific Ways

#include "World/ATVoxelISMC.h"

#include "World/ATVoxelChunk.h"
#include "World/ATVoxelTypeData.h"

#if DEBUG_VOXELS
	#pragma optimize("", off)
#endif // DEBUG_VOXELS

FVoxelInstanceData UATVoxelISMC::InvalidInstanceData_NonConst = FVoxelInstanceData::Invalid;
FVoxelCompoundData UATVoxelISMC::InvalidCompoundData_NonConst = FVoxelCompoundData::Invalid;

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

FVoxelCompoundData& UATVoxelISMC::GetCompoundDataAt(const FIntVector& InTargetPoint, const bool bInChecked) const
{
	if (const FVoxelCompoundData* SampleCompoundDataPtr = LocalPoint_To_CompoundData_Map.Find(InTargetPoint))
	{
		return *const_cast<FVoxelCompoundData*>(SampleCompoundDataPtr);
	}
	else
	{
		ensure(!bInChecked);
		return InvalidCompoundData_NonConst;
	}
}

bool UATVoxelISMC::HasCompoundAtPoint(const FIntVector& InPoint) const
{
	return LocalPoint_To_CompoundData_Map.Contains(InPoint);
}

void UATVoxelISMC::GetAdjacentCompoundOriginsAt(const FIntVector& InTargetPoint, EATAttachmentDirection InSide, TArray<FIntVector>& OutCompoundOrigins) const
{
	const FVoxelCompoundData& TargetCompoundData = GetCompoundDataAt(InTargetPoint);

	TArray<FIntVector> SidePoints;
	TargetCompoundData.GetPointsAtSide(InSide, SidePoints);

	for (const FIntVector& SampleSidePoint : SidePoints)
	{
		const FVoxelCompoundData& SampleSideCompoundData = GetCompoundDataAt(SampleSidePoint);
		if (SampleSideCompoundData.IsValid())
		{
			OutCompoundOrigins.AddUnique(SampleSideCompoundData.Origin);
		}
	}
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
	//OwnerChunk->AttachmentUpdates.QueuePointIfRelevant(InPoint);
	//OwnerChunk->StabilityUpdates.QueuePointIfRelevant(InPoint);
	OwnerChunk->QueueRecursiveStabilityUpdate(InPoint);
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
		//OwnerChunk->AttachmentUpdates.QueuePointIfRelevant(InPoint);
		//OwnerChunk->StabilityUpdates.QueuePointIfRelevant(InPoint);
		OwnerChunk->QueueRecursiveStabilityUpdate(InPoint);
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

	ensure(LocalPoint_To_InstanceData_Map.IsEmpty());
	ensure(InstanceIndex_To_LocalPoint_Map.IsEmpty());

	ClearInstances();
}

bool UATVoxelISMC::BreakVoxelAtPoint(const FIntVector& InPoint, const bool bInForced, const bool bInNotify)
{
	if (bInForced)
	{
		if (RemoveVoxelAtPoint(InPoint, false))
		{
			if (bInNotify)
			{
				OnBreakVoxelAtLocalPoint.Broadcast(InPoint, bInForced);
			}
			return true;
		}
	}
	else
	{
		const FVoxelInstanceData& TargetData = GetVoxelInstanceDataAtPoint(InPoint, false);
		if (TargetData.IsTypeDataValid() && !TargetData.TypeData->IsFoundation)
		{
			if (RemoveVoxelAtPoint(InPoint))
			{
				if (bInNotify)
				{
					OnBreakVoxelAtLocalPoint.Broadcast(InPoint, bInForced);
				}
				return true;
			}
		}
	}
	return false;
}

bool UATVoxelISMC::BreakVoxelsAtPoints(const TArray<FIntVector>& InPoints, const bool bInForced, const bool bInNotify)
{
	bool bAnyBroken = false;

	for (const FIntVector& SamplePoint : InPoints)
	{
		bAnyBroken |= BreakVoxelAtPoint(SamplePoint, bInForced, bInNotify);
	}
	return bAnyBroken;
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

void UATVoxelISMC::RegenerateCompoundData()
{
	TArray<FIntVector> AllPoints;
	GetAllLocalPoints(AllPoints);

	LocalPoint_To_CompoundData_Map.Empty();

	for (const FIntVector& SampleCompoundOrigin : AllPoints)
	{
		if (HasCompoundAtPoint(SampleCompoundOrigin))
		{
			continue;
		}
		int32 NewCompoundSize = CalcMaxFittingCompoundSizeAt(SampleCompoundOrigin);
		SetCompoundData(SampleCompoundOrigin, NewCompoundSize);
	}
}

bool UATVoxelISMC::SetCompoundData(const FIntVector& InOrigin, int32 InSize, const bool bInChecked)
{
	if (InSize <= 0)
	{
		ensure(false);
		return false;
	}
	FVoxelCompoundData NewCompoundData = FVoxelCompoundData(InOrigin, InSize);

	for (int32 SampleCompoundX = 0; SampleCompoundX < NewCompoundData.Size; ++SampleCompoundX)
	{
		for (int32 SampleCompoundY = 0; SampleCompoundY < NewCompoundData.Size; ++SampleCompoundY)
		{
			for (int32 SampleCompoundZ = 0; SampleCompoundZ < NewCompoundData.Size; ++SampleCompoundZ)
			{
				ensure(!bInChecked || !LocalPoint_To_CompoundData_Map.Contains(InOrigin));
				LocalPoint_To_CompoundData_Map.Add(InOrigin, NewCompoundData);
			}
		}
	}
	//QueuePointForVisibilityUpdate(InPoint);
	//OwnerChunk->AttachmentUpdates.QueuePointIfRelevant(InPoint);
	//OwnerChunk->StabilityUpdates.QueuePointIfRelevant(InPoint);
	//OwnerChunk->QueueRecursiveStabilityUpdate(InPoint);
	return true;
}

int32 UATVoxelISMC::CalcMaxFittingCompoundSizeAt(const FIntVector& InOrigin)
{
	const int32 CompoundMaxSize = OwnerChunk->ChunkSize;
	int32 OutSize = 0;
	for (int32 CompoundSizeMinusOne = 0; CompoundSizeMinusOne < CompoundMaxSize; ++CompoundSizeMinusOne)
	{
		bool bSuccess = true;

		for (int32 OffsetX = 0; (OffsetX < CompoundSizeMinusOne + 1) && bSuccess; ++OffsetX)
		{
			for (int32 OffsetY = 0; (OffsetY < CompoundSizeMinusOne + 1) && bSuccess; ++OffsetY)
			{
				for (int32 OffsetZ = 0; (OffsetZ < CompoundSizeMinusOne + 1) && bSuccess; ++OffsetZ)
				{
					FIntVector SampleCompoundPoint = InOrigin + FIntVector(OffsetX, OffsetY, OffsetZ);

					if (!HasVoxelAtPoint(SampleCompoundPoint) || HasCompoundAtPoint(SampleCompoundPoint))
					{
						bSuccess = false;
						break;
					}
				}
			}
		}
		if (bSuccess)
		{
			OutSize = CompoundSizeMinusOne + 1;
		}
	}
	return OutSize + 1;
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
