// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "World/ATTypes_World.h"

#include "ATVoxelChunk.generated.h"

USTRUCT()
struct FVoxelChunkContainers
{
	GENERATED_BODY()

private:

	//UPROPERTY()
	//TArray<FVoxelInstanceData> VoxelDataArray;

	UPROPERTY()
	TMap<FIntVector, FVoxelInstanceData> LocalPoint_To_InstanceData_Map;

	UPROPERTY()
	TMap<int32, FIntVector> InstanceIndex_To_LocalPoint_Map;

	static FVoxelInstanceData InvalidInstanceData_NonConst;

public:

	UPROPERTY()
	TArray<FIntVector> FoundationLocalPoints;

	FORCEINLINE const TMap<FIntVector, FVoxelInstanceData>& GetLocalPoint_To_InstanceData_Map() const { return LocalPoint_To_InstanceData_Map; }
	FORCEINLINE void GetAllLocalPoints(TArray<FIntVector>& OutPoints) const { LocalPoint_To_InstanceData_Map.GenerateKeyArray(OutPoints); }

	FORCEINLINE bool AddVoxel(const FIntVector& InPoint, FVoxelInstanceData InInstanceData, int32 InInstanceIndex)
	{
		InInstanceData.SMI_Index = InInstanceIndex;

		ensure(!LocalPoint_To_InstanceData_Map.Contains(InPoint));
		LocalPoint_To_InstanceData_Map.Add(InPoint, InInstanceData);

		ensure(!InstanceIndex_To_LocalPoint_Map.Contains(InInstanceIndex));
		InstanceIndex_To_LocalPoint_Map.Add(InInstanceIndex, InPoint);
		return true;
	}

	FORCEINLINE bool RemoveVoxelByInstanceIndex(int32 InInstanceIndex, const bool bInChecked)
	{
		ensure(!bInChecked || InstanceIndex_To_LocalPoint_Map.Contains(InInstanceIndex));
		if (const FIntVector* SamplePointPtr = InstanceIndex_To_LocalPoint_Map.Find(InInstanceIndex))
		{
			const FIntVector& SamplePoint = *SamplePointPtr;

			ensure(LocalPoint_To_InstanceData_Map.Contains(SamplePoint));
			LocalPoint_To_InstanceData_Map.Remove(SamplePoint);
		}
		else
		{
			return false;
		}
		InstanceIndex_To_LocalPoint_Map.Remove(InInstanceIndex);
		return true;
	}

	FORCEINLINE bool RemoveVoxelByPoint(const FIntVector& InPoint, const bool bInChecked)
	{
		ensure(!bInChecked || LocalPoint_To_InstanceData_Map.Contains(InPoint));
		if (const FVoxelInstanceData* SampleInstanceDataPtr = LocalPoint_To_InstanceData_Map.Find(InPoint))
		{
			ensure(InstanceIndex_To_LocalPoint_Map.Contains(SampleInstanceDataPtr->SMI_Index));
			InstanceIndex_To_LocalPoint_Map.Remove(SampleInstanceDataPtr->SMI_Index);
		}
		else
		{
			return false;
		}
		LocalPoint_To_InstanceData_Map.Remove(InPoint);
		return true;
	}

	FORCEINLINE const FIntVector& GetVoxelInstancePoint(int32 InInstanceIndex) const
	{
		if (const FIntVector* SamplePointPtr = InstanceIndex_To_LocalPoint_Map.Find(InInstanceIndex))
		{
			return *SamplePointPtr;
		}
		else
		{
			return FIntVector::ZeroValue;
		}
	}

	FORCEINLINE FVoxelInstanceData& GetVoxelInstanceData(int32 InInstanceIndex) const
	{
		return GetVoxelInstanceData(GetVoxelInstancePoint(InInstanceIndex));
	}

	FORCEINLINE FVoxelInstanceData& GetVoxelInstanceData(const FIntVector& InPoint) const
	{
		if (const FVoxelInstanceData* SampleInstanceDataPtr = LocalPoint_To_InstanceData_Map.Find(InPoint))
		{
			return *const_cast<FVoxelInstanceData*>(SampleInstanceDataPtr);
		}
		else
		{
			return InvalidInstanceData_NonConst;
		}
	}

	FORCEINLINE int32 GetVoxelInstanceIndex(const FIntVector& InPoint) const
	{
		ensure(LocalPoint_To_InstanceData_Map.Contains(InPoint));
		if (const FVoxelInstanceData* SampleInstanceDataPtr = LocalPoint_To_InstanceData_Map.Find(InPoint))
		{
			return SampleInstanceDataPtr->SMI_Index;
		}
		else
		{
			return INDEX_NONE;
		}
	}

	FORCEINLINE bool RelocateInstanceIndex(int32 InPrevIndex, int32 InNewIndex)
	{
		if (!InstanceIndex_To_LocalPoint_Map.Contains(InPrevIndex))
		{
			ensure(false);
			return false;
		}
		FIntVector OldIndexPoint;
		InstanceIndex_To_LocalPoint_Map.RemoveAndCopyValue(InPrevIndex, OldIndexPoint);
		InstanceIndex_To_LocalPoint_Map.Add(InNewIndex, OldIndexPoint);

		if (!LocalPoint_To_InstanceData_Map.Contains(OldIndexPoint))
		{
			ensure(false);
			return false;
		}
		FVoxelInstanceData& SampleInstanceData = *LocalPoint_To_InstanceData_Map.Find(OldIndexPoint);
		
		ensure(SampleInstanceData.SMI_Index == InPrevIndex);
		SampleInstanceData.SMI_Index = InNewIndex;
		return true;
	}
};

#if DEBUG_VOXELS
	#pragma optimize("", off)
#endif // DEBUG_VOXELS

USTRUCT(BlueprintType)
struct FVoxelChunkPendingUpdates
{
	GENERATED_BODY()

private:

	UPROPERTY()
	FIntArraySetPair PendingInstanceIndices;

	UPROPERTY()
	FIntArraySetPair ThisTickSelectedInstanceIndices;

	UPROPERTY()
	FIntArraySetPair ThisTickAlreadyUpdatedInstanceIndices;

	UPROPERTY()
	FIntArraySetPair NextTickNewPendingIndices;

	UPROPERTY()
	bool bIsInsideUpdateSequence;

public:

	const TArray<int32>& GetPendingInstanceIndicesConstArray() const { return PendingInstanceIndices.GetConstArray(); }
	const TArray<int32>& GetThisTickSelectedInstanceIndicesConstArray() const { return ThisTickSelectedInstanceIndices.GetConstArray(); }

	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> TargetISMC;

	UPROPERTY()
	int32 DebugThisTickSelectedDataIndex = 1;

	UPROPERTY(BlueprintReadWrite)
	bool DebugMarkThisTickSelectedIndices = true;

	UPROPERTY(BlueprintReadWrite)
	bool DebugDeMarkThisTickSelectedIndices = true;

	void QueueInstanceIndexIfRelevant(int32 InInstanceIndex)
	{
		if (bIsInsideUpdateSequence)
		{
			if (IsInstanceWaitingToUpdateThisTick(InInstanceIndex))
			{
				// Index is going to be updated soon anyway - no need to queue it
			}
			else
			{
				NextTickNewPendingIndices.Add(InInstanceIndex);
			}
		}
		else
		{
			PendingInstanceIndices.Add(InInstanceIndex);
		}
	}

	void MarkInstanceIndexAsUpdatedThisTick(int32 InInstanceIndex)
	{
		ensure(!ThisTickAlreadyUpdatedInstanceIndices.Contains(InInstanceIndex));
		ThisTickAlreadyUpdatedInstanceIndices.Add(InInstanceIndex);
	}

	bool PrepareThisTickSelectedInstanceIndices(int32 InDesiredUpdatesNum)
	{
		if (DebugDeMarkThisTickSelectedIndices && !ThisTickSelectedInstanceIndices.IsEmpty())
		{
			for (int32 SampleInstanceIndex : ThisTickSelectedInstanceIndices.GetConstArray())
			{
				TargetISMC->SetCustomDataValue(SampleInstanceIndex, DebugThisTickSelectedDataIndex, 0.0f, false);
			}
			TargetISMC->MarkRenderStateDirty();
		}
		ThisTickSelectedInstanceIndices.Empty(InDesiredUpdatesNum);
		PendingInstanceIndices.AddTailTo(InDesiredUpdatesNum, ThisTickSelectedInstanceIndices);

		bIsInsideUpdateSequence = !ThisTickSelectedInstanceIndices.IsEmpty();
		return bIsInsideUpdateSequence;
	}

	bool IsInstanceWaitingToUpdateThisTick(int32 InInstanceIndex) const
	{
		return ThisTickSelectedInstanceIndices.Contains(InInstanceIndex) && !ThisTickAlreadyUpdatedInstanceIndices.Contains(InInstanceIndex);
	}

	void ResolveThisTickSelectedInstanceIndices()
	{
		if (DebugMarkThisTickSelectedIndices && !ThisTickSelectedInstanceIndices.IsEmpty())
		{
			for (int32 SampleInstanceIndex : ThisTickSelectedInstanceIndices.GetConstArray())
			{
				TargetISMC->SetCustomDataValue(SampleInstanceIndex, DebugThisTickSelectedDataIndex, 1.0f, false);
			}
			TargetISMC->MarkRenderStateDirty();
		}
		PendingInstanceIndices.RemoveFromOther(ThisTickSelectedInstanceIndices);
		PendingInstanceIndices.AddFromOther(NextTickNewPendingIndices);

		NextTickNewPendingIndices.Empty();
		bIsInsideUpdateSequence = false;
	}

	int32 ResolveThisTickAlreadyUpdatedInstanceIndices()
	{
		int32 OutIndicesNum = ThisTickAlreadyUpdatedInstanceIndices.Num();
		ThisTickAlreadyUpdatedInstanceIndices.Empty();
		return OutIndicesNum;
	}

	void RelocateInstanceIndex(int32 InPrevIndex, int32 InNewIndex)
	{
		PendingInstanceIndices.Replace(InPrevIndex, InNewIndex);
		ThisTickSelectedInstanceIndices.Replace(InPrevIndex, InNewIndex);

		ensure(NextTickNewPendingIndices.IsEmpty());
	}

	void DebugInstanceAttachmentDirection(const FVoxelInstanceData& InInstanceData)
	{
		FTransform NewInstanceTransform;
		TargetISMC->GetInstanceTransform(InInstanceData.SMI_Index, NewInstanceTransform);

		FVector AttachmentVector = EATAttachmentDirection_Utils::CreateVectorFromAttachmentDirections(InInstanceData.AttachmentDirections);
		NewInstanceTransform.SetRotation(AttachmentVector.ToOrientationQuat());
		NewInstanceTransform.SetScale3D(FVector(0.1f + AttachmentVector.Length(), 0.5f, 0.5f));
		TargetISMC->UpdateInstanceTransform(InInstanceData.SMI_Index, NewInstanceTransform);
	}
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
	virtual void OnConstruction(const FTransform& InTransform) override; // AActor
	virtual void BeginPlay() override; // AActor
	virtual void Tick(float InDeltaSeconds)  override; // AActor
	virtual void EndPlay(const EEndPlayReason::Type InReason) override; // AActor
//~ End Initialize
	
//~ Begin Components
public:

	UFUNCTION(Category = "Components", BlueprintCallable)
	UInstancedStaticMeshComponent* GetInstancedStaticMeshComponent() const { return InstancedStaticMeshComponent; }

protected:

	UPROPERTY(Category = "Components", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInstancedStaticMeshComponent> InstancedStaticMeshComponent;
//~ End Components

//~ Begin Getters
public:

	UFUNCTION(Category = "Getters", BlueprintCallable)
	const FVoxelInstanceData& GetVoxelDataAtLocalPoint(const FIntVector& InLocalPoint) const;

	//UFUNCTION(Category = "Getters", BlueprintCallable)
	//const FVoxelInstanceData& GetVoxelDataAtIndex(int32 InLocalIndex) const;

	//UFUNCTION(Category = "Getters", BlueprintCallable)
	//int32 LocalPoint_To_LocalIndex(const FIntVector& InLocalPoint) const;

	//UFUNCTION(Category = "Getters", BlueprintCallable)
	//const FIntVector& LocalIndex_To_LocalPoint(int32 InLocalIndex) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	FIntVector RelativeLocation_To_LocalPoint(const FVector& InRelativeLocation) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	FIntVector WorldLocation_To_LocalPoint(const FVector& InWorldLocation) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	FVector LocalPoint_To_RelativeLocation(const FIntVector& InLocalPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	FVector LocalPoint_To_WorldLocation(const FIntVector& InLocalPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	FVector GetChunkCenterWorldLocation() const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	FVector GetVoxelCenterWorldLocation(const FIntVector& InLocalPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	bool HasVoxelAtLocalPoint(const FIntVector& InLocalPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	int32 GetVoxelNeighborsNumAtLocalPoint(const FIntVector& InLocalPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	void GetVoxelPointsInSphere(const FIntVector& InCenterLocalPoint, int32 InRadius, TArray<FIntVector>& OutPoints) const;

	//UFUNCTION(Category = "Getters", BlueprintCallable)
	//void GetVoxelIndicesInSphere(int32 InCenterLocalIndex, int32 InRadius, TArray<int32>& OutIndices) const;
//~ End Getters
	
//~ Begin Setters
public:

	UFUNCTION(Category = "Setters", BlueprintCallable, meta = (KeyWords = "AddVoxelAt, PlaceVoxelAt"))
	void SetVoxelAtLocalPoint(const FIntVector& InLocalPoint, const class UATVoxelTypeData* InTypeData);

	UFUNCTION(Category = "Setters", BlueprintCallable, meta = (KeyWords = "AddVoxelsAt, PlaceVoxelsAt"))
	void SetVoxelsAtLocalPoints(const TArray<FIntVector>& InLocalPoints, const class UATVoxelTypeData* InTypeData);

	//UFUNCTION(Category = "Setters", BlueprintCallable)
	//void FillWithVoxels();

	//UFUNCTION(Category = "Setters", BlueprintCallable, meta = (KeyWords = "RemoveVoxelAt, DeleteVoxelAt"))
	//void BreakVoxelAtLocalIndex(int32 InLocalIndex);

	//UFUNCTION(Category = "Setters", BlueprintCallable, meta = (KeyWords = "RemoveVoxelsAt, DeleteVoxelsAt"))
	//void BreakVoxelsAtLocalIndices(const TArray<int32>& InLocalIndices);

	UFUNCTION(Category = "Setters", BlueprintCallable, meta = (KeyWords = "RemoveVoxelAt, DeleteVoxelAt"))
	void BreakVoxelAtLocalPoint(const FIntVector& InLocalPoint);

	UFUNCTION(Category = "Setters", BlueprintCallable, meta = (KeyWords = "RemoveVoxelsAt, DeleteVoxelsAt"))
	void BreakVoxelsAtLocalPoints(const TArray<FIntVector>& InLocalPoints);

	UFUNCTION(Category = "Setters", BlueprintCallable, meta = (KeyWords = "RemoveWithInstanceIds, DeleteWithInstanceIds"))
	void BreakVoxelsWithInstanceIndices(const TArray<int32>& InInstanceIndices);
//~ End Setters

//~ Begin Data
public:

	//UFUNCTION(Category = "Data", BlueprintCallable)
	void CreateFoundation();

	UFUNCTION(Category = "Data | Attachment", BlueprintCallable)
	int32 UpdatePendingAttachmentData(int32 InMaxUpdates);
	/*FORCEINLINE*/ void UpdatePendingAttachmentData_UpdateFromNeighbor(const FIntVector& InTargetPoint, const FIntVector& InNeighborOffset, const FVector& InAttachmentDirection, EAxis::Type InAxis, float InAxisMul, float InAttachmentStrengthMul, EATAttachmentDirection ToTargetDirection, EATAttachmentDirection ToNeighborDirection);

	UFUNCTION(Category = "Data | Stability", BlueprintCallable)
	int32 UpdatePendingStabilityData(int32 InMaxUpdates);
	/*FORCEINLINE*/ void UpdatePendingStabilityData_UpdateFromNeighbor(const FIntVector& InTargetPoint, const FIntVector& InNeighborOffset, const FVector& InAttachmentDirection, EAxis::Type InAxis, float InAxisMul, float InAttachmentStrengthMul, EATAttachmentDirection ToTargetDirection, EATAttachmentDirection ToNeighborDirection, TArray<int32>& InOutAttachedNeighbors);

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

protected:

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	float StabilityUpdatePropagationThreshold;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	float StabilityUpdatePropagationSkipProbability;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	bool bOffsetMeshToCenter;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	bool bDebugInstancesAttachmentDirection;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	bool bDebugInstancesStabilityValues;

	UPROPERTY()
	FVoxelChunkContainers Data;

	UPROPERTY(Category = "Data | Attachment", BlueprintReadWrite)
	FVoxelChunkPendingUpdates AttachmentUpdates;

	UPROPERTY(Category = "Data | Stability", BlueprintReadWrite)
	FVoxelChunkPendingUpdates StabilityUpdates;

	void OnInstanceIndicesUpdated(UInstancedStaticMeshComponent* InUpdatedComponent, TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData> InIndexUpdates);
	FDelegateHandle InstanceIndexUpdatedDelegateHandle;
//~ End Data

//~ Begin Cache
public:
	void UpdateCache();
	void ResetCache();
protected:

	UPROPERTY(Category = "Cache", BlueprintReadOnly)
	TObjectPtr<class AATGameState> Cache_GameState;

	UPROPERTY()
	TMap<FIntVector, int32> Cache_LocalPoint_To_LocalIndex_Map;

	UPROPERTY()
	TMap<int32, FIntVector> Cache_LocalIndex_To_LocalPoint_Map;
//~ End Cache

//~ Begin Debug
public:

	UFUNCTION(Category = "Debug", BlueprintNativeEvent, BlueprintCallable, meta = (DisplayName = "CollectDataForGameplayDebugger"))
	void BP_CollectDataForGameplayDebugger(APlayerController* ForPlayerController, FVoxelChunkDebugData& InOutData) const;
//~ End Debug
};
