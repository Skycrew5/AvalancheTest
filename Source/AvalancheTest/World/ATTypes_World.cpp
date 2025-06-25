// Scientific Ways

#include "World/ATTypes_World.h"

const FVoxelInstanceData FVoxelInstanceData::Invalid = FVoxelInstanceData();
const FVoxelCompoundData FVoxelCompoundData::Invalid = FVoxelCompoundData();

void FVoxelCompoundData::GetAllPoints(TArray<FIntVector>& OutPoints) const
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
			OutPoints = GetAllPoints();
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