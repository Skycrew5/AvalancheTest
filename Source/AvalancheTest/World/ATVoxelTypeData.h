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

//~ Begin Initialize
public:

	UFUNCTION(Category = "Initialize", BlueprintNativeEvent, BlueprintCallable, meta = (DisplayName = "InitializeInstanceData"))
	FVoxelInstanceData BP_InitializeInstanceData(class AATVoxelTree* InVoxelTree, const FIntVector& InPoint) const;
//~ End Initialize

//~ Begin UI
public:

	UPROPERTY(Category = "UI", EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;
//~ End UI

//~ Begin Stability
public:

	UPROPERTY(Category = "Stability", EditDefaultsOnly, BlueprintReadOnly)
	bool bIsFoundation;

	UPROPERTY(Category = "Stability", EditDefaultsOnly, BlueprintReadOnly)
	float MaxHealth;
//~ End Stability
};
