// Scientific Ways

#include "World/ATWorldFunctionLibrary.h"

#include "World/ATVoxelISMC.h"

//~ Begin Locations
FIntVector UATWorldFunctionLibrary::WorldLocation_To_Point(const FVector& InWorldLocation, float InVoxelSize)
{
	FVector VoxelScaledWorldLocation = (InWorldLocation / InVoxelSize);
	return FIntVector(FMath::CeilToInt(VoxelScaledWorldLocation.X), FMath::CeilToInt(VoxelScaledWorldLocation.Y), FMath::CeilToInt(VoxelScaledWorldLocation.Z));
}

FVector UATWorldFunctionLibrary::Point_To_WorldLocation(const FIntVector& InPoint, float InVoxelSize)
{
	return FVector(InPoint * InVoxelSize);
}

FIntVector UATWorldFunctionLibrary::RelativeLocation_To_Point(const class UATVoxelISMC* InVoxelComponent, const FVector& InRelativeLocation)
{
	ensureReturn(InVoxelComponent, FIntVector::ZeroValue);
	return WorldLocation_To_Point(InVoxelComponent->GetComponentTransform().InverseTransformPosition(InRelativeLocation), InVoxelComponent->GetVoxelSize());
}

FVector UATWorldFunctionLibrary::Point_To_RelativeLocation(const class UATVoxelISMC* InVoxelComponent, const FIntVector& InPoint)
{
	ensureReturn(InVoxelComponent, FVector::ZeroVector);
	return InVoxelComponent->GetComponentTransform().InverseTransformPosition(Point_To_WorldLocation(InPoint, InVoxelComponent->GetVoxelSize()));
}

FVector UATWorldFunctionLibrary::GetVoxelCenterWorldLocation(const FIntVector& InPoint, float InVoxelSize)
{
	return FVector(InPoint * InVoxelSize) + FVector(InVoxelSize * 0.5f, InVoxelSize * 0.5f, InVoxelSize * 0.5f);
}
//~ End Locations
