// Avalanche Test

#pragma once

#include "AvalancheTest.h"

#include "Framework/ScWGameState.h"

#include "World/Types_World.h"

#include "ATGameState.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class AVALANCHETEST_API AATGameState : public AScWGameState
{
	GENERATED_BODY()
	
public:

	AATGameState();
	
//~ Begin Statics
public:

	UFUNCTION(Category = "Statics", BlueprintCallable, BlueprintPure, meta = (WorldContext = "InWCO"))
	static AATGameState* TryGetATGameState(const UObject* InWCO);
//~ End Statics

//~ Begin Voxels
public:

	UFUNCTION(Category = "Voxels", BlueprintCallable)
	FORCEINLINE int32 GetVoxelIndexAt(const FIntVector& InGlobalXYZ) const;

	UPROPERTY(Category = "Voxels", EditAnywhere, BlueprintReadOnly)
	int32 VoxelChunkSize;

	UPROPERTY(Category = "Voxels", EditAnywhere, BlueprintReadOnly)
	float VoxelBaseSize;

	UPROPERTY(Category = "Voxels", BlueprintReadWrite)
	TArray<FVoxelData> VoxelDataArray;
//~ End Voxels
};
