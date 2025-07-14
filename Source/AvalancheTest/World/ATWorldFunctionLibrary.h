// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "ATWorldFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "[AT] World Function Library"))
class AVALANCHETEST_API UATWorldFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
//~ Begin Locations
public:

	UFUNCTION(Category = "Locations", BlueprintCallable)
	static FIntVector WorldLocation_To_Point3D(const FVector& InWorldLocation, const float InVoxelSize);

	UFUNCTION(Category = "Locations", BlueprintCallable)
	static FIntPoint WorldLocation_To_PointXY(const FVector& InWorldLocation, const float InVoxelSize);

	UFUNCTION(Category = "Locations", BlueprintCallable)
	static FVector Point3D_To_WorldLocation(const FIntVector& InPoint3D, const float InVoxelSize);

	UFUNCTION(Category = "Locations", BlueprintCallable)
	static FIntVector RelativeLocation_To_Point3D(const class UATVoxelISMC* InVoxelComponent, const FVector& InRelativeLocation);

	UFUNCTION(Category = "Locations", BlueprintCallable)
	static FVector Point3D_To_RelativeLocation(const class UATVoxelISMC* InVoxelComponent, const FIntVector& InPoint3D);

	UFUNCTION(Category = "Locations", BlueprintCallable)
	static FVector GetVoxelCenterWorldLocation(const FIntVector& InPoint3D, const float InVoxelSize);
//~ End Locations

//~ Begin Indices
public:

	UFUNCTION(Category = "Indices", BlueprintCallable)
	static FIntVector ArrayIndex3D_To_Point3D(const int32 InArrayIndex3D, const FIntVector& InBoxSize);

	UFUNCTION(Category = "Indices", BlueprintCallable)
	static int32 Point3D_To_ArrayIndex3D(const FIntVector& InPoint3D, const FIntVector& InBoxSize);

	UFUNCTION(Category = "Indices", BlueprintCallable)
	static FIntVector ArrayIndex2D_To_Point3D(const int32 InArrayIndex2D, const int32 InZ, const FIntVector& InBoxSize);

	UFUNCTION(Category = "Indices", BlueprintCallable)
	static int32 ArrayIndex2D_To_ArrayIndex3D(const int32 InArrayIndex2D, const int32 InZ, const FIntVector& InBoxSize);
//~ End Indices
};
