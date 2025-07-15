// Scientific Ways

#include "World/ATVoxelTypeData.h"

#include "World/ATVoxelTree.h"

UATVoxelTypeData::UATVoxelTypeData()
{
	DisplayName = FText::FromString(TEXT("Unnamed Voxel Type"));

	bAddToInventory = false;

	bIsUnbreakable = false;
	bHasInfiniteStability = false;

	AttachmentStrengthMul = 1.0f;
	MaxHealth = 1.0f;

	BrokenVoxelLandHitDamage = 4.0f;
}

//~ Begin Initialize
void UATVoxelTypeData::PostLoad() // UObject
{
	Super::PostLoad();

	InitCachedAttachmentStrengthMuls();
}

#if WITH_EDITOR
void UATVoxelTypeData::PostEditChangeProperty(FPropertyChangedEvent& InPropertyChangedEvent) // UObject
{
	Super::PostEditChangeProperty(InPropertyChangedEvent);

	InitCachedAttachmentStrengthMuls();
}
#endif // WITH_EDITOR

FVoxelInstanceData UATVoxelTypeData::BP_InitializeInstanceData_Implementation(AATVoxelTree* InVoxelTree, const FIntVector& InPoint) const
{
	if (!InVoxelTree)
	{
		return FVoxelInstanceData::Invalid;
	}
	FVoxelInstanceData OutData = FVoxelInstanceData(this, MaxHealth);
	OutData.Stability = 1.0f;
	return OutData;
}
//~ End Initialize

//~ Begin Stability
float UATVoxelTypeData::GetStabilityAttachmentMulForDirection(EATAttachmentDirection InDirection) const
{
	return CachedAttachmentStrengthMuls[InDirection];
}

void UATVoxelTypeData::InitCachedAttachmentStrengthMuls()
{
	using namespace EATAttachmentDirection_Utils;

	CachedAttachmentStrengthMuls.Empty(AttachmentDirectionsArray.Num());
	for (EATAttachmentDirection SampleDirection : AttachmentDirectionsArray)
	{
		float AdjustedStrengthMul = FMath::Pow(BaseAttachmentStrengthMuls[SampleDirection], 1.0f / AttachmentStrengthMul);
		CachedAttachmentStrengthMuls.Add(SampleDirection, AdjustedStrengthMul);
	}
}
//~ End Stability
