// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "World/ATVoxelNode.h"

#include "ATVoxelTree.generated.h"

USTRUCT(BlueprintType)
struct FVoxelSubTreeInitData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Size = FIntVector(2, 2, 2);
};

UCLASS(meta = (DisplayName = "[AT] Voxel Tree"))
class AVALANCHETEST_API AATVoxelTree : public AATVoxelNode
{
	GENERATED_BODY()
	
public:

	AATVoxelTree(const FObjectInitializer& InObjectInitializer = FObjectInitializer::Get());
	
//~ Begin Initialize
protected:
	virtual void PostInitializeComponents() override; // AActor
	virtual void OnConstruction(const FTransform& InTransform) override; // AActor
	virtual void BeginPlay() override; // AActor
	virtual void Tick(float InDeltaSeconds) override; // AActor
	virtual void EndPlay(const EEndPlayReason::Type InReason) override; // AActor
//~ End Initialize

//~ Begin Parent Tree
public:
	virtual void InitFromParentTree(class AATVoxelTree* InTree, const FIntVector& InInsideParentLocalPoint) override; // AATVoxelNode
//~ End Parent Tree

//~ Begin SubTree
public:

	UFUNCTION(Category = "SubTree", BlueprintCallable)
	FIntVector GetSubTreeNodeSizeInVoxels() const;

	UPROPERTY(Category = "SubTree", EditAnywhere, BlueprintReadWrite)
	TArray<FVoxelSubTreeInitData> SubTreesInitData;

	UPROPERTY(Category = "SubTree", EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AATVoxelTree> SubTreeClass;

protected:
	void InitSubTreeNodesRecursive();

	UPROPERTY()
	TMap<FIntVector, TObjectPtr<AATVoxelNode>> TreeNodesMap;
//~ End SubTree

//~ Begin Voxel Bounds
protected:
	virtual FIntVector GetNodeSizeInVoxels() const override; // AATVoxelNode
//~ End Voxel Bounds

//~ Begin Voxel Components
public:
	virtual class UATVoxelISMC* GetVoxelComponentAtPoint(const FIntVector& InThisNodeScopePoint) const override; // AATVoxelNode
//~ End Voxel Components

//~ Begin Voxel Chunks
public:
	virtual class AATVoxelChunk* GetVoxelChunkAtPoint(const FIntVector& InThisNodeScopePoint) const override; // AATVoxelNode

	UPROPERTY(Category = "Voxel Chunks", EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AATVoxelChunk> ChunkClass;
//~ End Voxel Chunks
};
