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
	FVoxelInstanceData K2_InitializeInstanceData(class AATVoxelChunk* InVoxelChunk, const FIntVector& InLocalPoint) const;

//~ Begin UI
public:

	UPROPERTY(Category = "UI", EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;
//~ End UI

//~ Begin Health
public:

	UPROPERTY(Category = "Health", EditDefaultsOnly, BlueprintReadOnly)
	float MaxHealth;
//~ End Health
};
