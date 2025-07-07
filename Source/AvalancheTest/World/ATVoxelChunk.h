// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "World/ATTypes_World.h"

#include "ATVoxelChunk.generated.h"

/**
 * 
 */
UCLASS(Abstract, meta = (DisplayName = "[AT] Voxel Chunk"))
class AVALANCHETEST_API AATVoxelChunk : public AActor
{
	GENERATED_BODY()
	
public:

	AATVoxelChunk(const FObjectInitializer& InObjectInitializer = FObjectInitializer::Get());
	
//~ Begin Initialize
protected:
	virtual void PostInitializeComponents() override; // AActor
	virtual void OnConstruction(const FTransform& InTransform) override; // AActor
	virtual void BeginPlay() override; // AActor
	virtual void EndPlay(const EEndPlayReason::Type InReason) override; // AActor
public:

	UFUNCTION(Category = "Initialize", BlueprintNativeEvent, meta = (DisplayName = "InitChunk"))
	void BP_InitChunk(class AATVoxelTree* InOwnerTree, const FIntVector& InChunkCoords);
//~ End Initialize
	
//~ Begin Voxel Tree
public:

	UFUNCTION(Category = "Voxel Tree", BlueprintCallable)
	class AATVoxelTree* GetOwnerTree() const { return OwnerTree; }

	UFUNCTION(Category = "Voxel Tree", BlueprintCallable)
	FIntVector GetChunkCoords() const { return ChunkCoords; }

	UFUNCTION(Category = "Voxel Tree", BlueprintCallable)
	int32 GetChunkSize() const;

	UFUNCTION(Category = "Voxel Tree", BlueprintCallable)
	float GetVoxelSize() const;

	UFUNCTION(Category = "Voxel Tree", BlueprintCallable)
	FIntVector GetChunkBackLeftCornerPoint() const { return GetChunkCoords() * GetChunkSize(); }

protected:

	UPROPERTY(Category = "Voxel Tree", EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class AATVoxelTree> OwnerTree;

	UPROPERTY(Category = "Voxel Tree", BlueprintReadOnly)
	FIntVector ChunkCoords;
//~ End Voxel Tree

//~ Begin Voxel Components
public:

	UFUNCTION(Category = "Voxel Components", BlueprintCallable, meta = (KeyWords = "GetInstancedStaticMesh, GetVoxelInstancedStaticMesh"))
	class UATVoxelISMC* GetVoxelComponentAtPoint(const FIntVector& InChunkPoint) const;
	
protected:
	void HandleISMCUpdatesTick(int32& InOutUpdatesLeft);
	
	UPROPERTY(Category = "Voxel Components", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UATVoxelISMC> VoxelComponent;
//~ End Voxel Components

//~ Begin Voxel Getters
public:

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable)
	FVector GetChunkCenterWorldLocation() const;
//~ End Voxel Getters

//~ Begin Voxel Setters
public:

	UFUNCTION(Category = "Voxel Setters", BlueprintCallable, meta = (KeyWords = "AddVoxelAt, PlaceVoxelAt"))
	void HandleSetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const struct FVoxelInstanceData& InVoxelInstanceData);

	UFUNCTION(Category = "Voxel Setters", BlueprintCallable, meta = (KeyWords = "RemoveVoxelAt, DeleteVoxelAt"))
	void HandleBreakVoxelAtPoint(const FIntVector& InPoint, const bool bInNotify = false);

	//UFUNCTION(Category = "Setters", BlueprintCallable)
	//void HandleSetVoxelStabilityAtPoint(const FIntVector& InPoint, float InNewStability);

	//UFUNCTION(Category = "Setters", BlueprintCallable)
	//void HandleSetVoxelHealthAtPoint(const FIntVector& InPoint, float InNewHealth);
//~ End Voxel Setters

//~ Begin Voxel Data
public:
	void HandleUpdates(int32& InOutUpdatesLeft);

	UPROPERTY(Category = "Voxel Data", EditAnywhere, BlueprintReadOnly)
	bool bEnableVoxelComponentsUpdatesTick;

protected:

	
//~ End Voxel Data
	
//~ Begin Voxel Generation
public:
	void HandleProceduralGeneration();
protected:

	
//~ End Voxel Generation

//~ Begin Debug
public:

	UFUNCTION(Category = "Debug", BlueprintNativeEvent, BlueprintCallable, meta = (DisplayName = "CollectDataForGameplayDebugger"))
	void BP_CollectDataForGameplayDebugger(APlayerController* ForPlayerController, struct FVoxelChunkDebugData& InOutData) const;
//~ End Debug
};
