// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "ATVoxelNode.generated.h"

UCLASS(Abstract, meta = (DisplayName = "[AT] Voxel Node"))
class AVALANCHETEST_API AATVoxelNode : public AActor
{
	GENERATED_BODY()
	
public:

	AATVoxelNode(const FObjectInitializer& InObjectInitializer = FObjectInitializer::Get());
	
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
	virtual void InitFromParentTree(class AATVoxelTree* InTree, const FIntVector& InInsideParentLocalPoint);

	UFUNCTION(Category = "Parent Tree", BlueprintCallable)
	class AATVoxelTree* GetParentTree() const { return ParentTree; }

	UFUNCTION(Category = "Parent Tree", BlueprintCallable)
	class AATVoxelTree* GetRootTree() const { return RootTree; }

	UFUNCTION(Category = "Parent Tree", BlueprintCallable)
	const FIntVector& GetInsideParentLocalPoint() const { return InsideParentLocalPoint; }

	UFUNCTION(Category = "Parent Tree", BlueprintCallable)
	FIntVector GetGlobalOffset() const;

protected:

	UPROPERTY()
	TObjectPtr<class AATVoxelTree> ParentTree;

	UPROPERTY()
	TObjectPtr<class AATVoxelTree> RootTree;

	UPROPERTY()
	FIntVector InsideParentLocalPoint;
//~ End Parent Tree

//~ Begin Voxel Chunks
public:

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	virtual class AATVoxelChunk* GetVoxelChunkAtPoint(const FIntVector& InThisNodeScopePoint) const;

	UPROPERTY(Category = "Voxel Chunks", EditAnywhere, BlueprintReadOnly)
	int32 ChunkSize;

	UPROPERTY(Category = "Voxel Chunks", EditAnywhere, BlueprintReadOnly)
	float VoxelSize;
//~ End Voxel Chunks
	
//~ Begin Voxel Bounds
public:

	UFUNCTION(Category = "Voxel Bounds", BlueprintCallable)
	virtual FIntVector GetNodeSizeInVoxels() const;

	UFUNCTION(Category = "Voxel Bounds", BlueprintCallable)
	bool IsLocalPointInVoxelBounds(const FIntVector& InLocalPoint) const;

	UFUNCTION(Category = "Voxel Bounds", BlueprintCallable)
	bool IsGlobalPointInVoxelBounds(const FIntVector& InGlobalPoint) const;

protected:
	virtual void Cache_UpdateVoxelBounds();

	UPROPERTY()
	FIntVector Cache_NodeSizeInVoxels;

	UPROPERTY()
	FIntVector Cache_VoxelBoundsStart;

	UPROPERTY()
	FIntVector Cache_VoxelBoundsEnd;
//~ End Voxel Bounds

//~ Begin Voxel Components
public:

	UFUNCTION(Category = "Voxel Components", BlueprintCallable, meta = (KeyWords = "GetInstancedStaticMesh, GetVoxelInstancedStaticMesh"))
	virtual class UATVoxelISMC* GetVoxelComponentAtPoint(const FIntVector& InThisNodeScopePoint) const;
//~ End Voxel Components

//~ Begin Voxel Getters
public:

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable, meta = (KeyWords = "GetInstanceData"))
	struct FVoxelInstanceData& GetVoxelInstanceDataAtPoint(const FIntVector& InThisNodeScopePoint, const bool bInChecked = true, const bool bInCanGetDataFromOtherNode = false) const;

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable)
	bool HasVoxelAtPoint(const FIntVector& InThisNodeScopePoint, const bool bInCanCheckOtherNode = false) const;

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable)
	int32 GetVoxelNeighborsNumAtLocalPoint(const FIntVector& InThisNodeScopePoint) const;

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable)
	bool IsVoxelAtPointFullyClosed(const FIntVector& InThisNodeScopePoint) const;

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable)
	void GetAllVoxelPointsInRadius(const FIntVector& InThisNodeScopeCenterPoint, int32 InRadius, TArray<FIntVector>& OutPoints) const;
//~ End Voxel Getters
	
//~ Begin Voxel Setters
public:

	UFUNCTION(Category = "Voxel Setters", BlueprintCallable, meta = (KeyWords = "AddVoxelAt, PlaceVoxelAt"))
	bool SetVoxelAtLocalPoint(const FIntVector& InThisNodeScopePoint, const class UATVoxelTypeData* InTypeData, const bool bInForced = false, const bool bInCanSetInOtherNode = false);

	UFUNCTION(Category = "Voxel Setters", BlueprintCallable, meta = (KeyWords = "AddVoxelsAt, PlaceVoxelsAt"))
	bool SetVoxelsAtLocalPoints(const TArray<FIntVector>& InThisNodeScopePoints, const class UATVoxelTypeData* InTypeData, const bool bInForced = false, const bool bInCanSetInOtherNode = false);

	UFUNCTION(Category = "Voxel Setters", BlueprintCallable, meta = (KeyWords = "RemoveVoxelAt, DeleteVoxelAt"))
	bool BreakVoxelAtLocalPoint(const FIntVector& InThisNodeScopePoint, const bool bInForced = false, const bool bInNotify = false, const bool bInCanBreakInOtherNode = false);

	UFUNCTION(Category = "Voxel Setters", BlueprintCallable, meta = (KeyWords = "RemoveVoxelsAt, DeleteVoxelsAt"))
	bool BreakVoxelsAtLocalPoints(const TArray<FIntVector>& InThisNodeScopePoints, const bool bInForced = false, const bool bInNotify = false, const bool bInCanBreakInOtherNode = false);
//~ End Voxel Setters
};
