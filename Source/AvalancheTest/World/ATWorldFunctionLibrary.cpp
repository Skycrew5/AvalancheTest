// Scientific Ways

#include "World/ATWorldFunctionLibrary.h"

#include "World/ATVoxelISMC.h"

//~ Begin Locations
FIntVector UATWorldFunctionLibrary::WorldLocation_To_Point3D(const FVector& InWorldLocation, const float InVoxelSize)
{
	FVector VoxelScaledWorldLocation = (InWorldLocation / InVoxelSize);
	return FIntVector(FMath::CeilToInt(VoxelScaledWorldLocation.X), FMath::CeilToInt(VoxelScaledWorldLocation.Y), FMath::CeilToInt(VoxelScaledWorldLocation.Z));
}

FIntPoint UATWorldFunctionLibrary::WorldLocation_To_PointXY(const FVector& InWorldLocation, const float InVoxelSize)
{
	FVector VoxelScaledWorldLocation = (InWorldLocation / InVoxelSize);
	return FIntPoint(FMath::CeilToInt(VoxelScaledWorldLocation.X), FMath::CeilToInt(VoxelScaledWorldLocation.Y));
}

FVector UATWorldFunctionLibrary::Point3D_To_WorldLocation(const FIntVector& InPoint3D, const float InVoxelSize)
{
	return FVector(InPoint3D * InVoxelSize);
}

FIntVector UATWorldFunctionLibrary::RelativeLocation_To_Point3D(const class UATVoxelISMC* InVoxelComponent, const FVector& InRelativeLocation)
{
	ensureReturn(InVoxelComponent, FIntVector::ZeroValue);
	return WorldLocation_To_Point3D(InVoxelComponent->GetComponentTransform().TransformPosition(InRelativeLocation), InVoxelComponent->GetVoxelSize());
}

FVector UATWorldFunctionLibrary::Point3D_To_RelativeLocation(const class UATVoxelISMC* InVoxelComponent, const FIntVector& InPoint3D)
{
	ensureReturn(InVoxelComponent, FVector::ZeroVector);
	return InVoxelComponent->GetComponentTransform().InverseTransformPosition(Point3D_To_WorldLocation(InPoint3D, InVoxelComponent->GetVoxelSize()));
}

FVector UATWorldFunctionLibrary::GetVoxelCenterWorldLocation(const FIntVector& InPoint3D, float InVoxelSize)
{
	return FVector(InPoint3D * InVoxelSize) + FVector(InVoxelSize * 0.5f, InVoxelSize * 0.5f, InVoxelSize * 0.5f);
}
//~ End Locations


//~ Begin Indices
FIntVector UATWorldFunctionLibrary::ArrayIndex3D_To_Point3D(const int32 InArrayIndex3D, const FIntVector& InBoxSize)
{
	return FIntVector(InArrayIndex3D % InBoxSize.X, (InArrayIndex3D / InBoxSize.X) % InBoxSize.Y, InArrayIndex3D / (InBoxSize.X * InBoxSize.Y));
}

int32 UATWorldFunctionLibrary::Point3D_To_ArrayIndex3D(const FIntVector& InPoint3D, const FIntVector& InBoxSize)
{
	return InPoint3D.X + InPoint3D.Y * (InBoxSize.X) + InPoint3D.Z * (InBoxSize.X * InBoxSize.Y);
}

FIntVector UATWorldFunctionLibrary::ArrayIndex2D_To_Point3D(const int32 InArrayIndex2D, const int32 InZ, const FIntVector& InBoxSize)
{
	return FIntVector(InArrayIndex2D % InBoxSize.X, InArrayIndex2D / InBoxSize.Y, InZ);
}

int32 UATWorldFunctionLibrary::ArrayIndex2D_To_ArrayIndex3D(const int32 InArrayIndex2D, const int32 InZ, const FIntVector& InBoxSize)
{
	return Point3D_To_ArrayIndex3D(ArrayIndex2D_To_Point3D(InArrayIndex2D, InZ, InBoxSize), InBoxSize);
}
//~ End Indices
