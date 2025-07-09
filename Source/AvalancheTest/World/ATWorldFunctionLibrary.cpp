// Scientific Ways

#include "World/ATWorldFunctionLibrary.h"

#include "World/ATVoxelISMC.h"

//~ Begin Locations
FIntVector UATWorldFunctionLibrary::WorldLocation_To_Point(const FVector& InWorldLocation, const float InVoxelSize)
{
	FVector VoxelScaledWorldLocation = (InWorldLocation / InVoxelSize);
	return FIntVector(FMath::CeilToInt(VoxelScaledWorldLocation.X), FMath::CeilToInt(VoxelScaledWorldLocation.Y), FMath::CeilToInt(VoxelScaledWorldLocation.Z));
}

FIntPoint UATWorldFunctionLibrary::WorldLocation_To_PointXY(const FVector& InWorldLocation, const float InVoxelSize)
{
	FVector VoxelScaledWorldLocation = (InWorldLocation / InVoxelSize);
	return FIntPoint(FMath::CeilToInt(VoxelScaledWorldLocation.X), FMath::CeilToInt(VoxelScaledWorldLocation.Y));
}

FVector UATWorldFunctionLibrary::Point_To_WorldLocation(const FIntVector& InPoint, const float InVoxelSize)
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


//~ Begin Indices
FIntVector UATWorldFunctionLibrary::ArrayIndex_To_Point(const int32 InArrayIndex, const FIntVector& InBoxSize)
{
	return FIntVector(InArrayIndex % InBoxSize.X, (InArrayIndex / InBoxSize.X) % InBoxSize.Y, InArrayIndex / (InBoxSize.X * InBoxSize.Y));
}

int32 UATWorldFunctionLibrary::Point_To_ArrayIndex(const FIntVector& InPoint, const FIntVector& InBoxSize)
{
	return InPoint.X + InPoint.Y * (InBoxSize.X) + InPoint.Z * (InBoxSize.X * InBoxSize.Y);
}
//~ End Indices
