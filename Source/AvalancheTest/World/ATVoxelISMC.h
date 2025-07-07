// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "ScWTypes_Containers.h"

#include "World/ATTypes_World.h"

#include "ATVoxelISMC.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBreakVoxelEventSignature, const FIntVector&, InPoint);

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
	void BP_InitComponent(class AATVoxelChunk* InOwnerChunk);
//~ End Initialize
	
//~ Begin Getters
public:

	UFUNCTION(Category = "Getters | Voxel Instance Data", BlueprintCallable)
	int32 GetVoxelInstancesNum() const { return Point_To_VoxelInstanceData_Map.Num(); }

	UFUNCTION(Category = "Getters | Voxel Instance Data", BlueprintCallable)
	bool HasVoxelInstanceDataAtPoint(const FIntVector& InPoint) const { return Point_To_VoxelInstanceData_Map.Contains(InPoint); }

	UFUNCTION(Category = "Getters | Voxel Instance Data", BlueprintCallable)
	FVoxelInstanceData& GetVoxelInstanceDataAtPoint(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Getters | Mesh Index", BlueprintCallable)
	bool HasMeshAtPoint(const FIntVector& InPoint) const { return Point_To_MeshIndex_MirroredMap.ContainsKey(InPoint); }

	UFUNCTION(Category = "Getters | Mesh Index", BlueprintCallable)
	int32 GetMeshIndexAtPoint(const FIntVector& InPoint) const { return Point_To_MeshIndex_MirroredMap.FindRefByKey(InPoint, INDEX_NONE); }

	UFUNCTION(Category = "Getters | Mesh Index", BlueprintCallable)
	const FIntVector& GetPointOfMeshIndex(int32 InMeshIndex) const { return Point_To_MeshIndex_MirroredMap.FindRefByValue(InMeshIndex, FIntVector::ZeroValue); }

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

	bool RelocateMeshInstanceIndex(int32 InPrevIndex, int32 InNewIndex, const bool bInChecked = true);
protected:

	UPROPERTY(Category = "Setters", BlueprintAssignable)
	FBreakVoxelEventSignature OnBreakVoxelAtPoint;
//~ End Setters
	
//~ Begin Data
public:
	void HandleUpdates(int32& InOutUpdatesLeft);

	UFUNCTION(Category = "Data", BlueprintCallable)
	int32 GetQueuedVisibilityUpdatePointsNum() const { return QueuedVisibilityUpdatePoints.Num(); }

	UFUNCTION(Category = "Data", BlueprintCallable)
	void QueuePointForVisibilityUpdate(const FIntVector& InPoint, const bool bInQueueNeighborsToo = true);

	UFUNCTION(Category = "Data", BlueprintCallable, meta = (KeyWords = "UpdateVoxelsVisibilityState"))
	void UpdateAllVoxelsVisibilityState() { int32 UpdatesNum = GetQueuedVisibilityUpdatePointsNum(); UpdateVoxelsVisibilityState(UpdatesNum); }

	void UpdateVoxelsVisibilityState(int32& InOutUpdatesLeft);
protected:

	UPROPERTY(Category = "Data", BlueprintReadOnly)
	TObjectPtr<class AATVoxelChunk> OwnerChunk;

	// Set from Tree->Chunk for gameplay purposes, not simulation
	UPROPERTY(Transient)
	TMap<FIntVector, FVoxelInstanceData> Point_To_VoxelInstanceData_Map;

	//UPROPERTY(Transient)
	TMirroredMapPair<FIntVector, int32> Point_To_MeshIndex_MirroredMap;

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
	void UpdateVoxelsDebugState(int32& InOutUpdatesLeft);

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
