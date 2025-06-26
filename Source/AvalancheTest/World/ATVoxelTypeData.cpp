// Scientific Ways

#include "World/ATVoxelTypeData.h"

#include "World/ATVoxelChunk.h"

UATVoxelTypeData::UATVoxelTypeData()
{
	DisplayName = FText::FromString(TEXT("Unnamed Voxel Type"));

	IsFoundation = false;
	MaxHealth = 10.0f;
}

FVoxelInstanceData UATVoxelTypeData::BP_InitializeInstanceData_Implementation(AATVoxelChunk* InVoxelChunk, const FIntVector& InPoint) const
{
	if (!InVoxelChunk)
	{
		return FVoxelInstanceData::Invalid;
	}
	FVoxelInstanceData OutData = FVoxelInstanceData(this, MaxHealth);
	OutData.Stability = 0.0f;

	if (IsFoundation)
	{
		OutData.AttachmentDirections = { EATAttachmentDirection::Bottom };
	}
	return OutData;
}
