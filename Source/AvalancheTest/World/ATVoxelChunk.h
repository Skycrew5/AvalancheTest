// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "World/ATTypes_World.h"

#include "ATVoxelChunk.generated.h"

#if DEBUG_VOXELS
	#pragma optimize("", off)
#endif // DEBUG_VOXELS

USTRUCT(BlueprintType)
struct FVoxelChunkPendingUpdates
{
	GENERATED_BODY()

private:

	TArraySetPair<FIntVector> PendingPoints;
	TArraySetPair<FIntVector> ThisTickSelectedPoints;
	TArraySetPair<FIntVector> ThisTickAlreadyUpdatedPoints;
	TArraySetPair<FIntVector> NextTickNewPendingIndices;

	UPROPERTY()
	bool bIsInsideUpdateSequence;

public:

	const TArray<FIntVector>& GetPendingPointsConstArray() const { return PendingPoints.GetConstArray(); }
	const TArray<FIntVector>& GetThisTickSelectedPointsConstArray() const { return ThisTickSelectedPoints.GetConstArray(); }

	UPROPERTY()
	TObjectPtr<class UATVoxelISMC> TargetISMC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DebugVoxelCustomData_ThisTickSelected = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDebugMarkThisTickSelectedIndices = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDebugDeMarkThisTickSelectedIndices = true;

	void QueuePointIfRelevant(const FIntVector& InPoint);
	void MarkPointAsUpdatedThisTick(const FIntVector& InPoint);
	bool PrepareThisTickSelectedPoints(int32 InDesiredUpdatesNum);
	bool IsInstanceWaitingToUpdateThisTick(const FIntVector& InPoint) const;
	void ResolveThisTickSelectedPoints();
	int32 ResolveThisTickAlreadyUpdatedPoints();
};

#if DEBUG_VOXELS
	#pragma optimize("", on)
#endif // DEBUG_VOXELS

/**
 * 
 */
UCLASS(Abstract, meta = (DisplayName = "[AT] Voxel Chunk"))
class AVALANCHETEST_API AATVoxelChunk : public AActor
{
	GENERATED_BODY()
	
public:

	AATVoxelChunk();
	
//~ Begin Initialize
protected:
	virtual void PostInitializeComponents() override; // AActor
	virtual void OnConstruction(const FTransform& InTransform) override; // AActor
	virtual void BeginPlay() override; // AActor
	virtual void Tick(float InDeltaSeconds)  override; // AActor
	virtual void EndPlay(const EEndPlayReason::Type InReason) override; // AActor

	// Called both in editor (upon creation and when something has changed) and in game on BeginPlay()
	UFUNCTION(Category = "Initialize", BlueprintNativeEvent, meta = (DisplayName = "CommonInitChunk"))
	void BP_CommonInitChunk();
//~ End Initialize
	
//~ Begin Components
public:

	UFUNCTION(Category = "Components", BlueprintCallable, meta = (KeyWords = "GetInstanced, GetVoxelInstanced"))
	class UATVoxelISMC* GetVoxelISMC() const { return VoxelISMC; }

protected:
	void HandleISMCUpdatesTick(int32& InOutMaxUpdates);

	UPROPERTY(Category = "Components", EditAnywhere, BlueprintReadOnly)
	bool bHandleISMCUpdatesTick;

	UPROPERTY(Category = "Components", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UATVoxelISMC> VoxelISMC;
//~ End Components

//~ Begin Locations
public:

	UFUNCTION(Category = "Getters", BlueprintCallable)
	FIntVector RelativeLocation_To_LocalPoint(const FVector& InRelativeLocation) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	FIntVector WorldLocation_To_LocalPoint(const FVector& InWorldLocation) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	FVector LocalPoint_To_RelativeLocation(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	FVector LocalPoint_To_WorldLocation(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	FVector GetChunkCenterWorldLocation() const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	FVector GetVoxelCenterWorldLocation(const FIntVector& InPoint) const;
//~ End Locations

//~ Begin Getters
public:

	UFUNCTION(Category = "Getters", BlueprintCallable)
	const FVoxelInstanceData& GetVoxelInstanceDataAtPoint(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	bool HasVoxelAtPoint(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	int32 GetVoxelNeighborsNumAtLocalPoint(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	bool IsVoxelAtPointFullyClosed(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	void GetAllPointsInRadius(const FIntVector& InCenterPoint, int32 InRadius, TArray<FIntVector>& OutPoints) const;
//~ End Getters
	
//~ Begin Setters
public:

	UFUNCTION(Category = "Setters", BlueprintCallable, meta = (KeyWords = "AddVoxelAt, PlaceVoxelAt"))
	bool SetVoxelAtLocalPoint(const FIntVector& InPoint, const class UATVoxelTypeData* InTypeData, const bool bInForced = false);

	UFUNCTION(Category = "Setters", BlueprintCallable, meta = (KeyWords = "AddVoxelsAt, PlaceVoxelsAt"))
	bool SetVoxelsAtLocalPoints(const TArray<FIntVector>& InPoints, const class UATVoxelTypeData* InTypeData, const bool bInForced = false);

	UFUNCTION(Category = "Setters", BlueprintCallable)
	void FillWithVoxels(const UATVoxelTypeData* InTypeData);

	UFUNCTION(Category = "Setters", BlueprintCallable, meta = (KeyWords = "ClearAll, BreakAllVoxels"))
	void RemoveAllVoxels();

	UFUNCTION(Category = "Setters", BlueprintCallable, meta = (KeyWords = "RemoveVoxelAt, DeleteVoxelAt"))
	bool BreakVoxelAtLocalPoint(const FIntVector& InPoint, const bool bInForced = false);

	UFUNCTION(Category = "Setters", BlueprintCallable, meta = (KeyWords = "RemoveVoxelsAt, DeleteVoxelsAt"))
	bool BreakVoxelsAtLocalPoints(const TArray<FIntVector>& InPoints, const bool bInForced = false);

	UFUNCTION(Category = "Setters", BlueprintCallable)
	void CreateFoundation();
//~ End Setters

//~ Begin Voxel Data
public:

	UFUNCTION(Category = "Data | Attachment", BlueprintCallable)
	void QueueFullUpdate();

	void HandleVoxelDataUpdatesTick(int32& InOutMaxUpdates);

	void QueueRecursiveStabilityUpdate(const FIntVector& InPoint, const bool bInQueueNeighborsToo = true);
	TArraySetPair<FIntVector> QueuedRecursiveStabilityUpdatePoints;

	void UpdateStabilityRecursive(int32& InOutUpdatesLeft);
	TSet<FIntVector> UpdateStabilityRecursive_ThisOrderUpdatedPoints;
	TMap<FIntVector, float> UpdateStabilityRecursive_CachedPointStabilities;

	float UpdateStabilityRecursive_GetStabilityFromAllNeighbors(const FIntVector& InTargetPoint, EATAttachmentDirection InDirectionsOrder[6], EATAttachmentDirection InNeighborDirection = EATAttachmentDirection::None);
	
	//float UpdateStabilityRecursive_GetStabilityFromAllNeighbors(TSet<FIntVector>& InOutCurrentSubChain, int32 InOutSubChainHash, const FIntVector& InTargetPoint, EATAttachmentDirection InNeighborDirection = EATAttachmentDirection::None);
	TMap<int32, float> UpdateStabilityRecursive_CachedSubchainStabilities;

	UFUNCTION(Category = "Data | Attachment", BlueprintCallable)
	int32 UpdatePendingAttachmentData(int32 InMaxUpdates);
	/*FORCEINLINE*/ void UpdatePendingAttachmentData_UpdateFromAllNeighbors(const FIntVector& InTargetPoint, TMap<FIntVector, EATAttachmentDirection>& InOutAttachedNeighborsAndDirections);
	/*FORCEINLINE*/ void UpdatePendingAttachmentData_UpdateFromNeighbor(const FIntVector& InTargetPoint, const FIntVector& InNeighborOffset, EATAttachmentDirection ToTargetDirection, EATAttachmentDirection ToNeighborDirection, TMap<FIntVector, EATAttachmentDirection>& InOutAttachedNeighborsAndDirections);

	UFUNCTION(Category = "Data | Stability", BlueprintCallable)
	int32 UpdatePendingStabilityData(int32 InMaxUpdates);
	/*FORCEINLINE*/ void UpdatePendingStabilityData_UpdateFromNeighbor(const FIntVector& InTargetPoint, const FIntVector& InNeighborOffset, float InAttachmentStrengthMul, EATAttachmentDirection ToTargetDirection, EATAttachmentDirection ToNeighborDirection, TArray<FIntVector>& InOutAttachedOrInvalidNeighbors);

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	bool bHandleVoxelDataUpdatesTick;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	int32 ChunkSize;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	float VoxelBaseSize;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	bool bIsOnFoundation;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	int32 MaxUpdatesPerSecond;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const class UATVoxelTypeData> FoundationVoxelTypeData;

	UPROPERTY(Category = "Data | Attachment", EditAnywhere, BlueprintReadWrite)
	FVoxelChunkPendingUpdates AttachmentUpdates;

	UPROPERTY(Category = "Data | Stability", EditAnywhere, BlueprintReadWrite)
	FVoxelChunkPendingUpdates StabilityUpdates;

protected:
	static void Static_OnISMInstanceIndicesUpdated(UInstancedStaticMeshComponent* InUpdatedComponent, TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData> InIndexUpdates);
	static FDelegateHandle InstanceIndexUpdatedDelegateHandle;

	void HandleInstanceIndicesUpdates(const TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData>& InIndexUpdates);

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadWrite)
	float StabilityUpdatePropagationThreshold;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadWrite)
	float StabilityUpdatePropagationSkipProbability;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadWrite)
	bool bOffsetMeshToCenter;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadWrite)
	bool bDebugInstancesAttachmentDirection;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadWrite)
	bool bDebugInstancesStabilityValues;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	int32 DebugVoxelCustomData_Stability;
//~ End Voxel Data

//~ Begin Debug
public:
	UFUNCTION(Category = "Debug", BlueprintNativeEvent, BlueprintCallable, meta = (DisplayName = "CollectDataForGameplayDebugger"))
	void BP_CollectDataForGameplayDebugger(APlayerController* ForPlayerController, FVoxelChunkDebugData& InOutData) const;

protected:
	void HandleDebugInstancesAttachmentDirections();
	void DebugInstanceAttachmentDirection(int32 InInstanceIndex);
	TArray<int32> DebugInstancesAttachmentDirections_QueuedIndices;

	void HandleDebugInstancesStabilityValues();
	void DebugInstanceStabilityValue(int32 InInstanceIndex);
	TArray<int32> DebugInstancesStablityValues_QueuedIndices;
//~ End Debug
};
