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
	static FIntVector WorldLocation_To_Point(const FVector& InWorldLocation, float InVoxelSize);

	UFUNCTION(Category = "Locations", BlueprintCallable)
	static FVector Point_To_WorldLocation(const FIntVector& InPoint, float InVoxelSize);

	UFUNCTION(Category = "Locations", BlueprintCallable)
	static FIntVector RelativeLocation_To_Point(const class UATVoxelISMC* InVoxelComponent, const FVector& InRelativeLocation);

	UFUNCTION(Category = "Locations", BlueprintCallable)
	static FVector Point_To_RelativeLocation(const class UATVoxelISMC* InVoxelComponent, const FIntVector& InPoint);

	UFUNCTION(Category = "Locations", BlueprintCallable)
	static FVector GetVoxelCenterWorldLocation(const FIntVector& InPoint, float InVoxelSize);
//~ End Locations

};
