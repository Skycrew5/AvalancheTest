// Scientific Ways

#include "World/ATVoxelTypeData.h"

#include "World/ATVoxelTree.h"

UATVoxelTypeData::UATVoxelTypeData()
{
	DisplayName = FText::FromString(TEXT("Unnamed Voxel Type"));

	bIsFoundation = false;
	MaxHealth = 10.0f;
}

//~ Begin Initialize
FVoxelInstanceData UATVoxelTypeData::BP_InitializeInstanceData_Implementation(AATVoxelTree* InVoxelTree, const FIntVector& InPoint) const
{
	if (!InVoxelTree)
	{
		return FVoxelInstanceData::Invalid;
	}
	FVoxelInstanceData OutData = FVoxelInstanceData(this, MaxHealth);
	OutData.Stability = 0.0f;
	return OutData;
}
//~ End Initialize
