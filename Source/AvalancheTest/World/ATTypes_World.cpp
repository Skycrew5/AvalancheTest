// Scientific Ways

#include "World/ATTypes_World.h"

#include "World/ATVoxelISMC.h"
#include "World/ATVoxelChunk.h"

#if DEBUG_VOXELS
	#pragma optimize("", off)
#endif // DEBUG_VOXELS

const FVoxelInstanceData FVoxelInstanceData::Invalid = FVoxelInstanceData();
//const FVoxelCompoundData FVoxelCompoundData::Invalid = FVoxelCompoundData();

/*void FVoxelCompoundData::GetAllPoints(TArray<FIntVector>& OutPoints) const
{
	ensure(OutPoints.IsEmpty());

	for (int32 OffsetX = 0; OffsetX < Size; ++OffsetX)
	{
		for (int32 OffsetY = 0; OffsetY < Size; ++OffsetY)
		{
			for (int32 OffsetZ = 0; OffsetZ < Size; ++OffsetZ)
			{
				OutPoints.Add(Origin + FIntVector(OffsetX, OffsetY, OffsetZ));
			}
		}
	}
}

void FVoxelCompoundData::GetPointsAtSide(EATAttachmentDirection InSide, TArray<FIntVector>& OutPoints, const bool bInOutside) const
{
	ensure(OutPoints.IsEmpty());

	const int32 OffsetNear = bInOutside ? -1 : 0;
	const int32 OffsetFar = bInOutside ? Size : Size - 1;

	switch (InSide)
	{
		case EATAttachmentDirection::None:
		{
			GetAllPoints(OutPoints);
			break;
		}
		case EATAttachmentDirection::Front:
		case EATAttachmentDirection::Back:
		{
			const int32 Offset = (InSide == EATAttachmentDirection::Front) ? OffsetFar : OffsetNear;

			for (int32 SampleSideY = 0; SampleSideY < Size; ++SampleSideY)
			{
				for (int32 SampleSideZ = 0; SampleSideZ < Size; ++SampleSideZ)
				{
					OutPoints.Add(Origin + FIntVector(Offset, SampleSideY, SampleSideZ));
				}
			}
			break;
		}
		case EATAttachmentDirection::Right:
		case EATAttachmentDirection::Left:
		{
			const int32 Offset = (InSide == EATAttachmentDirection::Right) ? OffsetFar : OffsetNear;

			for (int32 SampleSideX = 0; SampleSideX < Size; ++SampleSideX)
			{
				for (int32 SampleSideZ = 0; SampleSideZ < Size; ++SampleSideZ)
				{
					OutPoints.Add(Origin + FIntVector(SampleSideX, Offset, SampleSideZ));
				}
			}
			break;
		}
		case EATAttachmentDirection::Top:
		case EATAttachmentDirection::Bottom:
		{
			const int32 Offset = (InSide == EATAttachmentDirection::Top) ? OffsetFar : OffsetNear;

			for (int32 SampleSideX = 0; SampleSideX < Size; ++SampleSideX)
			{
				for (int32 SampleSideY = 0; SampleSideY < Size; ++SampleSideY)
				{
					OutPoints.Add(Origin + FIntVector(SampleSideX, SampleSideY, Offset));
				}
			}
			break;
		}
		default:
		{
			ensure(false);
			break;
		}
	}
}

void FVoxelChunkPendingUpdates::QueuePointIfRelevant(const FIntVector& InPoint)
{
	if (bIsInsideUpdateSequence)
	{
		if (IsInstanceWaitingToUpdateThisTick(InPoint))
		{
			// Index is going to be updated soon anyway - no need to queue it
		}
		else
		{
			NextTickNewPendingIndices.Add(InPoint);
		}
	}
	else
	{
		PendingPoints.Add(InPoint);
	}
}

void FVoxelChunkPendingUpdates::MarkPointAsUpdatedThisTick(const FIntVector& InPoint)
{
	ensure(!ThisTickAlreadyUpdatedPoints.Contains(InPoint));
	ThisTickAlreadyUpdatedPoints.Add(InPoint);
}

bool FVoxelChunkPendingUpdates::PrepareThisTickSelectedPoints(int32 InDesiredUpdatesNum)
{
	if (bDebugDeMarkThisTickSelectedIndices && !ThisTickSelectedPoints.IsEmpty())
	{
		for (const FIntVector& SamplePoint : ThisTickSelectedPoints.GetConstArray())
		{
			const FVoxelInstanceData& SampleData = TargetISMC->GetVoxelInstanceDataAtPoint(SamplePoint, false);
			if (SampleData.HasMesh())
			{
				TargetISMC->SetCustomDataValue(SampleData.SMI_Index, DebugVoxelCustomData_ThisTickSelected, 0.0f, false);
			}
		}
		TargetISMC->MarkRenderStateDirty();
	}
	int32 UpdatesNum = (TargetISMC->GetOwnerChunk()->MaxUpdatesPerSecond > 0) ? FMath::Min(InDesiredUpdatesNum, PendingPoints.Num()) : PendingPoints.Num();

	ThisTickSelectedPoints.Empty(UpdatesNum);
	PendingPoints.AddHeadTo(UpdatesNum, ThisTickSelectedPoints);

	bIsInsideUpdateSequence = !ThisTickSelectedPoints.IsEmpty();
	return bIsInsideUpdateSequence;
	return false;
}

bool FVoxelChunkPendingUpdates::IsInstanceWaitingToUpdateThisTick(const FIntVector& InPoint) const
{
	return ThisTickSelectedPoints.Contains(InPoint) && !ThisTickAlreadyUpdatedPoints.Contains(InPoint);
}

void FVoxelChunkPendingUpdates::ResolveThisTickSelectedPoints()
{
	if (bDebugMarkThisTickSelectedIndices && !ThisTickSelectedPoints.IsEmpty())
	{
		for (const FIntVector& SamplePoint : ThisTickSelectedPoints.GetConstArray())
		{
			const FVoxelInstanceData& SampleData = TargetISMC->GetVoxelInstanceDataAtPoint(SamplePoint, false);
			if (SampleData.HasMesh())
			{
				TargetISMC->SetCustomDataValue(SampleData.SMI_Index, DebugVoxelCustomData_ThisTickSelected, 1.0f, false);
			}
		}
		TargetISMC->MarkRenderStateDirty();
	}
	PendingPoints.RemoveFromOther(ThisTickSelectedPoints);
	PendingPoints.AddFromOther(NextTickNewPendingIndices);

	NextTickNewPendingIndices.Empty();
	bIsInsideUpdateSequence = false;
}

int32 FVoxelChunkPendingUpdates::ResolveThisTickAlreadyUpdatedPoints()
{
	int32 OutPointsNum = ThisTickAlreadyUpdatedPoints.Num();
	ThisTickAlreadyUpdatedPoints.Empty();
	return OutPointsNum;
}*/

#if DEBUG_VOXELS
	#pragma optimize("", on)
#endif // DEBUG_VOXELS
