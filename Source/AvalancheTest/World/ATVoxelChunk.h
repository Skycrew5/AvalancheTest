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
	
//~ Begin Components
public:

	UFUNCTION(Category = "Components", BlueprintCallable, meta = (KeyWords = "GetInstancedStaticMesh, GetVoxelInstancedStaticMesh"))
	class UATVoxelISMC* GetVoxelComponentAtPoint(const FIntVector& InChunkPoint) const;
	
protected:
	void HandleISMCUpdatesTick(int32& InOutMaxUpdates);
	
	UPROPERTY(Category = "Components", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UATVoxelISMC> VoxelComponent;
//~ End Components

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
	void HandleUpdates(int32& InOutUpdatesNum);

	UFUNCTION(Category = "Voxel Data", BlueprintCallable)
	class AATVoxelTree* GetOwnerTree() const { return OwnerTree; }

	UFUNCTION(Category = "Voxel Data", BlueprintCallable)
	FIntVector GetChunkCoords() const { return ChunkCoords; }

	UFUNCTION(Category = "Voxel Data", BlueprintCallable)
	int32 GetChunkSize() const;

	UFUNCTION(Category = "Voxel Data", BlueprintCallable)
	float GetVoxelSize() const;

	UPROPERTY(Category = "Voxel Data", EditAnywhere, BlueprintReadOnly)
	bool bEnableVoxelComponentsUpdatesTick;

protected:

	UPROPERTY(Category = "Voxel Data", EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class AATVoxelTree> OwnerTree;

	UPROPERTY(Category = "Voxel Data", BlueprintReadOnly)
	FIntVector ChunkCoords;
//~ End Voxel Data

//~ Begin Debug
public:

	UFUNCTION(Category = "Debug", BlueprintNativeEvent, BlueprintCallable, meta = (DisplayName = "CollectDataForGameplayDebugger"))
	void BP_CollectDataForGameplayDebugger(APlayerController* ForPlayerController, struct FVoxelChunkDebugData& InOutData) const;
//~ End Debug
};
