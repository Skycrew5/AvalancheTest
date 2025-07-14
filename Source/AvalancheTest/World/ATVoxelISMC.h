// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "ScWTypes_Containers.h"

#include "World/ATTypes_World.h"

#include "ATVoxelISMC.generated.h"

/**
 *
 */
UCLASS(meta = (DisplayName = "[AT] Voxel Instanced Static Mesh Component"))
class AVALANCHETEST_API UATVoxelISMC : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()

public:

	UATVoxelISMC();
	
//~ Begin Initialize
protected:
	virtual void OnRegister() override; // UActorComponent
	virtual void BeginPlay() override; // UActorComponent
	virtual void EndPlay(const EEndPlayReason::Type InReason) override; // UActorComponent
public:

	UFUNCTION(Category = "Initialize", BlueprintNativeEvent, meta = (DisplayName = "InitComponent"))
	void BP_InitComponent(class AATVoxelChunk* InOwnerChunk, const class UATVoxelTypeData* InTypeData);
//~ End Initialize
	
//~ Begin Getters
public:

	UFUNCTION(Category = "Getters | Voxel Instance Data", BlueprintCallable)
	int32 GetVoxelInstancesNum() const { return Point_To_MeshIndex_Map.Num(); }

	UFUNCTION(Category = "Getters", BlueprintCallable)
	bool HasVoxelOfThisTypeAtPoint(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	bool HasVoxelOfAnyTypeAtPoint(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	bool CouldCreateMeshAtPoint(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable, meta = (KeyWords = "GetInstanceData"))
	FVoxelInstanceData& GetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const bool bInChecked = true) const;

	UFUNCTION(Category = "Getters | Mesh Index", BlueprintCallable)
	bool HasMeshAtPoint(const FIntVector& InPoint) const { return Point_To_MeshIndex_Map.Contains(InPoint); }

	UFUNCTION(Category = "Getters | Mesh Index", BlueprintCallable)
	int32 GetMeshIndexAtPoint(const FIntVector& InPoint) const { return Point_To_MeshIndex_Map.FindRef(InPoint, INDEX_NONE); }

	UFUNCTION(Category = "Getters | Mesh Index", BlueprintCallable)
	FIntVector GetPointOfMeshIndex(int32 InMeshIndex, const bool bInChecked = true) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	bool IsVoxelAtPointFullyClosed(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	float GetVoxelSize() const;
//~ End Getters

//~ Begin Setters
public:

	UFUNCTION(Category = "Setters | Voxel Instance Data", BlueprintCallable, meta = (KeyWords = "AddVoxelAt, PlaceVoxelAt"))
	void HandleSetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const struct FVoxelInstanceData& InVoxelInstanceData);

	UFUNCTION(Category = "Setters | Voxel Instance Data", BlueprintCallable)
	bool HandleBreakVoxelAtPoint(const FIntVector& InPoint, const bool bInNotify = false);

	bool HandleMeshInstanceIndexRelocated(int32 InPrevIndex, int32 InNewIndex);
protected:

//~ End Setters
	
//~ Begin Data
public:
	void HandleUpdates();

	UFUNCTION(Category = "Data", BlueprintCallable)
	int32 GetQueuedVisibilityUpdatePointsNum() const { return QueuedVisibilityUpdatePoints.Num(); }

	UFUNCTION(Category = "Data", BlueprintCallable)
	void QueuePointForVisibilityUpdate(const FIntVector& InPoint, const bool bInQueueNeighborsToo = true);

	UFUNCTION(Category = "Data", BlueprintCallable, meta = (KeyWords = "UpdateVoxelsVisibilityState"))
	void ForceUpdateAllVoxelsVisibilityState() { UpdateVoxelsVisibilityState(true); }

	void UpdateVoxelsVisibilityState(const bool bInIgnoreTimeBugdet = false);

	UPROPERTY(Transient)
	bool bEnableVoxelsVisibilityUpdates;

protected:

	UPROPERTY(Category = "Data", BlueprintReadOnly)
	TObjectPtr<class AATVoxelChunk> OwnerChunk;

	UPROPERTY(Category = "Data", BlueprintReadOnly)
	TObjectPtr<const class UATVoxelTypeData> VoxelTypeData;

	UPROPERTY(Transient)
	TMap<FIntVector, int32> Point_To_MeshIndex_Map;

	UPROPERTY(Transient)
	TSet<FIntVector> PossibleMeshPointsSet;

	//UPROPERTY(Transient)
	TArraySetPair<FIntVector> QueuedVisibilityUpdatePoints;
//~ End Data

//~ Begin Meshes
protected:
	static void Static_OnISMInstanceIndicesUpdated(UInstancedStaticMeshComponent* InUpdatedComponent, TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData> InIndexUpdates);
	void HandleInstanceIndicesUpdates(const TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData>& InIndexUpdates);

	static FDelegateHandle InstanceIndexUpdatedDelegateHandle;
//~ End Meshes

//~ Begin Debug
public:

	UFUNCTION(Category = "Debug", BlueprintCallable)
	void UpdateVoxelsDebugState();

	FORCEINLINE void TryQueuePointForDebugUpdate(const FIntVector& InPoint) { if (bDebugStabilityValues || bDebugHealthValues) QueuedDebugUpdatePoints.Add(InPoint); }
	FORCEINLINE void TryQueueMeshIndexForDebugUpdate(int32 InMeshIndex) { TryQueuePointForDebugUpdate(GetPointOfMeshIndex(InMeshIndex)); }
	
	UPROPERTY(Category = "Debug", EditAnywhere, BlueprintReadWrite)
	bool bDebugStabilityValues;

	UPROPERTY(Category = "Debug", EditAnywhere, BlueprintReadWrite)
	bool bDebugHealthValues;

	UPROPERTY(Category = "Debug", EditAnywhere, BlueprintReadOnly)
	int32 DebugVoxelCustomData_Stability;

	UPROPERTY(Category = "Debug", EditAnywhere, BlueprintReadOnly)
	int32 DebugVoxelCustomData_Health;

protected:

	void Debug_UpdateStabilityValueAtPoint(const FIntVector& InPoint);
	void Debug_UpdateHealthValueAtPoint(const FIntVector& InPoint);

	void Debug_UpdateStabilityValueAtMeshIndex(int32 InMeshIndex) { Debug_UpdateStabilityValueAtPoint(GetPointOfMeshIndex(InMeshIndex)); }
	void Debug_UpdateHealthValueAtMeshIndex(int32 InMeshIndex) { Debug_UpdateHealthValueAtPoint(GetPointOfMeshIndex(InMeshIndex)); }

	//UPROPERTY(Transient)
	TArraySetPair<FIntVector> QueuedDebugUpdatePoints;
//~ End Debug
};
