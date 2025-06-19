// Avalanche Test

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

public:

	FORCEINLINE const TMap<FIntVector, FVoxelInstanceData>& GetLocalPoint_To_InstanceData_Map() const { return LocalPoint_To_InstanceData_Map; }

	FORCEINLINE bool AddVoxel(const FIntVector& InPoint, FVoxelInstanceData InInstanceData, int32 InInstanceIndex)
	{
		InInstanceData.InstanceIndex = InInstanceIndex;

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
			ensure(InstanceIndex_To_LocalPoint_Map.Contains(SampleInstanceDataPtr->InstanceIndex));
			InstanceIndex_To_LocalPoint_Map.Remove(SampleInstanceDataPtr->InstanceIndex);
		}
		else
		{
			return false;
		}
		LocalPoint_To_InstanceData_Map.Remove(InPoint);
		return true;
	}

	FORCEINLINE const FIntVector& GetVoxelPoint(int32 InIndex) const
	{
		if (const FIntVector* SamplePointPtr = InstanceIndex_To_LocalPoint_Map.Find(InIndex))
		{
			return *SamplePointPtr;
		}
		else
		{
			return FIntVector::ZeroValue;
		}
	}

	FORCEINLINE const FVoxelInstanceData& GetVoxelInstanceData(const FIntVector& InPoint) const
	{
		if (const FVoxelInstanceData* SampleInstanceDataPtr = LocalPoint_To_InstanceData_Map.Find(InPoint))
		{
			return *SampleInstanceDataPtr;
		}
		else
		{
			return FVoxelInstanceData::Invalid;
		}
	}

	FORCEINLINE int32 GetVoxelInstanceIndex(const FIntVector& InPoint) const
	{
		ensure(LocalPoint_To_InstanceData_Map.Contains(InPoint));
		if (const FVoxelInstanceData* SampleInstanceDataPtr = LocalPoint_To_InstanceData_Map.Find(InPoint))
		{
			return SampleInstanceDataPtr->InstanceIndex;
		}
		else
		{
			return INDEX_NONE;
		}
	}

	FORCEINLINE bool ChangeVoxelInstanceIndex(int32 InPrevIndex, int32 InNewIndex)
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
		
		ensure(SampleInstanceData.InstanceIndex == InPrevIndex);
		SampleInstanceData.InstanceIndex = InNewIndex;
		return true;
	}
};

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

	UFUNCTION(Category = "Getters", BlueprintCallable)
	int32 LocalPoint_To_LocalIndex(const FIntVector& InLocalPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	const FIntVector& LocalIndex_To_LocalPoint(int32 InLocalIndex) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	FIntVector RelativeLocation_To_LocalPoint(const FVector& InRelativeLocation) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	FIntVector WorldLocation_To_LocalPoint(const FVector& InWorldLocation) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	FVector LocalPoint_To_RelativeLocation(const FIntVector& InLocalPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	bool HasVoxelAtLocalPoint(const FIntVector& InLocalPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	int32 GetVoxelNeighboursNumAtLocalPoint(const FIntVector& InLocalPoint) const;

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

	UFUNCTION(Category = "Data", BlueprintCallable)
	void UpdateStabilityData();

	UPROPERTY(Category = "Data", EditInstanceOnly, BlueprintReadWrite)
	bool bUpdateStabilityDataNextTick;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	int32 VoxelChunkSize;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	float VoxelBaseSize;

protected:

	UPROPERTY()
	FVoxelChunkContainers Data;

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
};
