// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "World/ATTypes_World.h"

#include "ATVoxelTypeData.generated.h"

/**
 *
 */
UCLASS(Const, Abstract, Blueprintable, BlueprintType, meta = (DisplayName = "[AT] Voxel Type Data"))
class AVALANCHETEST_API UATVoxelTypeData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:	

	UATVoxelTypeData();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta = (DisplayName = "InitializeInstanceData"))
	FVoxelInstanceData BP_InitializeInstanceData(class AATVoxelChunk* InVoxelChunk, const FIntVector& InPoint) const;

//~ Begin UI
public:

	UPROPERTY(Category = "UI", EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;
//~ End UI

//~ Begin Stability
public:

	UPROPERTY(Category = "Stability", EditDefaultsOnly, BlueprintReadOnly)
	bool IsFoundation;

	UPROPERTY(Category = "Stability", EditDefaultsOnly, BlueprintReadOnly)
	float MaxHealth;
//~ End Stability
};
