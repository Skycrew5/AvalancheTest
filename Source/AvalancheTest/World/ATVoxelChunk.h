// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "World/ATVoxelNode.h"
#include "World/ATTypes_World.h"

#include "ATVoxelChunk.generated.h"

/**
 * 
 */
UCLASS(Abstract, meta = (DisplayName = "[AT] Voxel Chunk"))
class AVALANCHETEST_API AATVoxelChunk : public AATVoxelNode
{
	GENERATED_BODY()
	
public:

	AATVoxelChunk(const FObjectInitializer& InObjectInitializer = FObjectInitializer::Get());
	
//~ Begin Initialize
protected:
	virtual void PostInitializeComponents() override; // AActor
	virtual void OnConstruction(const FTransform& InTransform) override; // AActor
	virtual void BeginPlay() override; // AActor
	virtual void Tick(float InDeltaSeconds) override; // AActor
	virtual void EndPlay(const EEndPlayReason::Type InReason) override; // AActor

	// Called both in editor (upon creation and when something has changed) and in game on BeginPlay()
	UFUNCTION(Category = "Initialize", BlueprintNativeEvent, meta = (DisplayName = "CommonInitChunk"))
	void BP_CommonInitChunk();
//~ End Initialize
	
//~ Begin Parent Tree
public:
	virtual void InitFromParentTree(class AATVoxelTree* InTree, const FIntVector& InInsideParentLocalPoint) override; // AATVoxelNode
//~ End Parent Tree
	
//~ Begin Voxel Bounds
protected:
	virtual FIntVector GetNodeSizeInVoxels() const override; // AATVoxelNode
//~ End Voxel Bounds

//~ Begin Voxel Components
public:
	virtual class UATVoxelISMC* GetVoxelComponentAtPoint(const FIntVector& InThisNodeScopePoint) const override; // AATVoxelNode

	UFUNCTION(Category = "Voxel Components", BlueprintCallable, meta = (KeyWords = "GetInstanced, GetVoxelInstanced"))
	class UATVoxelISMC* GetVoxelISMC() const { return VoxelISMC; }

protected:
	void HandleISMCUpdatesTick(int32& InOutMaxUpdates);

	UPROPERTY(Category = "Voxel Components", EditAnywhere, BlueprintReadOnly)
	bool bHandleISMCUpdatesTick;

	UPROPERTY(Category = "Voxel Components", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UATVoxelISMC> VoxelISMC;
//~ End Components

//~ Begin Voxel Chunks
public:
	virtual class AATVoxelChunk* GetVoxelChunkAtPoint(const FIntVector& InThisNodeScopePoint) const override { return const_cast<AATVoxelChunk*>(this); } // AATVoxelNode
//~ End Voxel Chunks

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
	
//~ Begin Setters
public:

	UFUNCTION(Category = "Setters", BlueprintCallable)
	void FillWithVoxels(const UATVoxelTypeData* InTypeData, const bool bInForced = false);

	UFUNCTION(Category = "Setters", BlueprintCallable, meta = (KeyWords = "ClearAll, BreakAllVoxels"))
	void RemoveAllVoxels();

	UFUNCTION(Category = "Setters", BlueprintCallable)
	void CreateFoundation();
//~ End Setters

//~ Begin Voxel Data
public:

	struct FRecursiveThreadData
	{
		const TArray<EATAttachmentDirection>* DirectionsOrderPtr;
		TSet<FIntVector> ThisOrderUpdatedPoints;

		FRecursiveThreadData(const TArray<EATAttachmentDirection>& InOrder)
			: DirectionsOrderPtr(&InOrder), ThisOrderUpdatedPoints({}) {}
	};

	struct FRecursivePointCache
	{
		float Stability;
		TSet<FIntVector> FinalUpdatedPoints;

		bool Intersects(const TSet<FIntVector>& InOther) const
		{
			const TSet<FIntVector>& SmallerSet = (FinalUpdatedPoints.Num() < InOther.Num()) ? FinalUpdatedPoints : InOther;
			const TSet<FIntVector>& LargerSet = (FinalUpdatedPoints.Num() < InOther.Num()) ? InOther : FinalUpdatedPoints;

			for (const FIntVector& SampleItem : SmallerSet)
			{
				if (LargerSet.Contains(SampleItem))
				{
					return true;
				}
			}
			return false;
		}
	};

	UFUNCTION(Category = "Data | Attachment", BlueprintCallable)
	void QueueFullUpdate();

	void HandleVoxelDataUpdatesTick(int32& InOutMaxUpdates);

	void QueueRecursiveStabilityUpdate(const FIntVector& InChunkLocalPoint, const bool bInQueueNeighborsToo = true);
	TArraySetPair<FIntVector> QueuedRecursiveStabilityUpdatePoints;

	void UpdateStabilityRecursive(int32& InOutUpdatesLeft);
	TMap<FIntVector, FRecursivePointCache> UpdateStabilityRecursive_PointsCache;

	float UpdateStabilityRecursive_GetStabilityFromAllNeighbors(const FIntVector& InTargetPoint, FRecursiveThreadData& InThreadData, EATAttachmentDirection InNeighborDirection = EATAttachmentDirection::None, uint8 InCurrentRecursionLevel = 0u);
	//float UpdateStabilityRecursive_GetStabilityFromAllNeighbors_HandleCompound(const FIntVector& InOrigin, EATAttachmentDirection InNeighborDirection = EATAttachmentDirection::None);
	
	//float UpdateStabilityRecursive_GetStabilityFromAllNeighbors(TSet<FIntVector>& InOutCurrentSubChain, int32 InOutSubChainHash, const FIntVector& InTargetPoint, EATAttachmentDirection InNeighborDirection = EATAttachmentDirection::None);
	TMap<int32, float> UpdateStabilityRecursive_CachedSubchainStabilities;

	void UpdateHealth(int32& InOutUpdatesLeft);
	TArraySetPair<FIntVector> InDangerGroupHealthUpdatePoints;

	//UFUNCTION(Category = "Data | Attachment", BlueprintCallable)
	//int32 UpdatePendingAttachmentData(int32 InMaxUpdates);
	///*FORCEINLINE*/ void UpdatePendingAttachmentData_UpdateFromAllNeighbors(const FIntVector& InTargetPoint, TMap<FIntVector, EATAttachmentDirection>& InOutAttachedNeighborsAndDirections);
	///*FORCEINLINE*/ void UpdatePendingAttachmentData_UpdateFromNeighbor(const FIntVector& InTargetPoint, const FIntVector& InNeighborOffset, EATAttachmentDirection ToTargetDirection, EATAttachmentDirection ToNeighborDirection, TMap<FIntVector, EATAttachmentDirection>& InOutAttachedNeighborsAndDirections);

	//UFUNCTION(Category = "Data | Stability", BlueprintCallable)
	//int32 UpdatePendingStabilityData(int32 InMaxUpdates);
	///*FORCEINLINE*/ void UpdatePendingStabilityData_UpdateFromNeighbor(const FIntVector& InTargetPoint, const FIntVector& InNeighborOffset, float InAttachmentStrengthMul, EATAttachmentDirection ToTargetDirection, EATAttachmentDirection ToNeighborDirection, TArray<FIntVector>& InOutAttachedOrInvalidNeighbors);

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	bool bHandleVoxelDataUpdatesTick;

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

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadWrite)
	bool bDebugInstancesHealthValues;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	int32 DebugVoxelCustomData_Stability;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	int32 DebugVoxelCustomData_Health;
//~ End Voxel Data

//~ Begin Debug
public:
	UFUNCTION(Category = "Debug", BlueprintNativeEvent, BlueprintCallable, meta = (DisplayName = "CollectDataForGameplayDebugger"))
	void BP_CollectDataForGameplayDebugger(APlayerController* ForPlayerController, struct FVoxelChunkDebugData& InOutData) const;

protected:
	void HandleDebugInstancesAttachmentDirections();
	void DebugInstanceAttachmentDirection(int32 InInstanceIndex);
	TArray<int32> DebugInstancesAttachmentDirections_QueuedIndices;

	void HandleDebugInstancesStabilityValues();
	void DebugInstanceStabilityValue(int32 InInstanceIndex);
	TArray<int32> DebugInstancesStablityValues_QueuedIndices;

	void HandleDebugInstancesHealthValues();
	void DebugInstanceHealthValue(int32 InInstanceIndex);
	TArray<int32> DebugInstancesHealthValues_QueuedIndices;
//~ End Debug
};
