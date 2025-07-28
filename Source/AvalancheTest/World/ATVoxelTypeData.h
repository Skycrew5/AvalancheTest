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
protected:
	virtual void PostLoad() override; // UObject
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& InPropertyChangedEvent) override; // UObject
#endif // WITH_EDITOR
public:

	UFUNCTION(Category = "Initialize", BlueprintNativeEvent, BlueprintCallable, meta = (DisplayName = "InitializeInstanceData"))
	FVoxelInstanceData BP_InitializeInstanceData(class AATVoxelTree* InVoxelTree, const FIntVector& InPoint) const;
//~ End Initialize

//~ Begin UI
public:

	UPROPERTY(Category = "UI", EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(Category = "UI", EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMaterialInterface> ImageMaterial;
//~ End UI
	
//~ Begin Mesh
public:

	UPROPERTY(Category = "Mesh", EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(Category = "Mesh", EditDefaultsOnly, BlueprintReadOnly)
	//TMap<int32, TObjectPtr<UMaterialInterface>> StaticMeshOverrideMaterials;
	TMap<int32, UMaterialInterface*> StaticMeshOverrideMaterials;
//~ End Mesh
	
//~ Begin Inventory
public:

	UPROPERTY(Category = "Inventory", EditDefaultsOnly, BlueprintReadOnly)
	bool bAddToInventory;
//~ End Inventory

//~ Begin Stability
public:

	UFUNCTION(Category = "Stability", BlueprintCallable)
	float GetStabilityAttachmentMulForDirection(EATAttachmentDirection InDirection) const;

	UPROPERTY(Category = "Stability", EditDefaultsOnly, BlueprintReadOnly)
	bool bIsUnbreakable;

	UPROPERTY(Category = "Stability", EditDefaultsOnly, BlueprintReadOnly)
	bool bHasInfiniteStability;

	UPROPERTY(Category = "Stability", EditDefaultsOnly, BlueprintReadOnly)
	float AttachmentStrengthMul;

	UPROPERTY(Category = "Stability", EditDefaultsOnly, BlueprintReadOnly)
	float MaxHealth;

protected:
	void InitCachedAttachmentStrengthMuls();

	UPROPERTY(Transient)
	TMap<EATAttachmentDirection, float> CachedAttachmentStrengthMuls = {
		{ EATAttachmentDirection::None, 1.0f }, // Used on recursion start, should be 1.0f
		{ EATAttachmentDirection::Front, 0.9f },
		{ EATAttachmentDirection::Back, 0.9f },
		{ EATAttachmentDirection::Right, 0.9f },
		{ EATAttachmentDirection::Left, 0.9f },
		{ EATAttachmentDirection::Top, 0.75 },
		{ EATAttachmentDirection::Bottom, 1.0f }
	};
//~ End Stability
	
//~ Begin Break
public:

	UPROPERTY(Category = "Break", EditDefaultsOnly, BlueprintReadOnly)
	float BrokenVoxelLandHitDamage;
//~ End Break
};
