// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "World/ATTypes_World.h"

#include "ATVoxelChunk.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBreakVoxelEventSignature, class UATVoxelISMC*, InVoxelComponent, const FIntVector&, InPoint);

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

	UFUNCTION(Category = "Voxel Tree", BlueprintCallable)
	int32 GetChunkSeed() const;

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

	UFUNCTION(Category = "Voxel Components", BlueprintCallable, meta = (KeyWords = "GetInstancedStaticMesh, GetVoxelInstancedStaticMesh, GetVoxelComponentForType"))
	class UATVoxelISMC* GetOrInitVoxelComponentForType(const class UATVoxelTypeData* InTypeData);

	UPROPERTY(Category = "Setters", BlueprintAssignable)
	FBreakVoxelEventSignature OnBreakVoxelAtPoint;

protected:

	UPROPERTY(Category = "Voxel Components", EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class UATVoxelISMC> VoxelComponentClass;

	UPROPERTY(Transient)
	TMap<TObjectPtr<const class UATVoxelTypeData>, TObjectPtr<class UATVoxelISMC>> PerTypeVoxelComponentMap;
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

	UFUNCTION(Category = "Voxel Data", BlueprintCallable)
	bool IsThisTickUpdatesTimeBudgetExceeded() const;

	UFUNCTION(Category = "Voxel Data", BlueprintCallable)
	bool IsChunkSimulationReady() const { return bChunkSimulationReady; }

	void MarkChunkAsSimulationReady() { ensure(!bChunkSimulationReady); bChunkSimulationReady = true; }
	void HandleUpdates();
protected:

	UPROPERTY(Category = "Voxel Data", EditAnywhere, BlueprintReadOnly)
	bool bEnableVoxelComponentsUpdatesTick;

	UPROPERTY(Transient)
	bool bChunkSimulationReady;
//~ End Voxel Data
	
//~ Begin Debug
public:

	UFUNCTION(Category = "Debug", BlueprintNativeEvent, BlueprintCallable, meta = (DisplayName = "CollectDataForGameplayDebugger"))
	void BP_CollectDataForGameplayDebugger(APlayerController* ForPlayerController, struct FVoxelChunkDebugData& InOutData) const;
//~ End Debug
};
