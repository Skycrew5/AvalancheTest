// Scientific Ways

#include "World/ATVoxelTypeData.h"

#include "World/ATVoxelChunk.h"

UATVoxelTypeData::UATVoxelTypeData()
{
	DisplayName = FText::FromString(TEXT("Unnamed Voxel Type"));

	MaxHealth = 1.0f;
}

FVoxelInstanceData UATVoxelTypeData::K2_InitializeInstanceData_Implementation(AATVoxelChunk* InVoxelChunk, const FIntVector& InLocalPoint) const
{
	if (!InVoxelChunk)
	{
		return FVoxelInstanceData::Invalid;
	}
	return FVoxelInstanceData(this, MaxHealth);
}
