// Avalanche Test

#pragma once

#include "AvalancheTest.h"

#include "World/ATTypes_World.h"

#include "ATVoxelChunk.generated.h"

USTRUCT()
struct FVoxelContainersData
{
	GENERATED_BODY()

private:

	//UPROPERTY()
	//TArray<FVoxelInstanceData> VoxelDataArray;

	UPROPERTY()
	TMap<int32, FIntVector> MeshInstanceIndex_To_LocalPoint_Map;

	UPROPERTY()
	TMap<FIntVector, int32> LocalPoint_To_MeshInstanceIndex_Map;

	UPROPERTY()
	TMap<FIntVector, FVoxelInstanceData> LocalPoint_To_VoxelDataMap;

public:

	FORCEINLINE bool AddVoxel(int32 InIndex, const FIntVector& InPoint, const FVoxelInstanceData& InData)
	{
		ensure(!MeshInstanceIndex_To_LocalPoint_Map.Contains(InIndex));
		MeshInstanceIndex_To_LocalPoint_Map.Add(InIndex, InPoint);

		ensure(!LocalPoint_To_MeshInstanceIndex_Map.Contains(InPoint));
		LocalPoint_To_MeshInstanceIndex_Map.Add(InPoint, InIndex);

		ensure(!LocalPoint_To_VoxelDataMap.Contains(InPoint));
		LocalPoint_To_VoxelDataMap.Add(InPoint, InData);
	}

	FORCEINLINE bool RemoveVoxelByIndex(int32 InIndex)
	{
		ensure(MeshInstanceIndex_To_LocalPoint_Map.Contains(InIndex));
		if (const FIntVector* SamplePointPtr = MeshInstanceIndex_To_LocalPoint_Map.Find(InIndex))
		{
			ensure(LocalPoint_To_MeshInstanceIndex_Map.Contains(*SamplePointPtr));
			LocalPoint_To_MeshInstanceIndex_Map.Remove(*SamplePointPtr);

			ensure(LocalPoint_To_VoxelDataMap.Contains(*SamplePointPtr));
			LocalPoint_To_VoxelDataMap.Remove(*SamplePointPtr);
		}
		else
		{
			ensure(false);
			return false;
		}
		MeshInstanceIndex_To_LocalPoint_Map.Remove(InIndex);
	}

	FORCEINLINE bool RemoveVoxelByPoint(const FIntVector& InPoint)
	{
		ensure(LocalPoint_To_MeshInstanceIndex_Map.Contains(InPoint));
		if (const int32* SampleIndexPtr = LocalPoint_To_MeshInstanceIndex_Map.Find(InPoint))
		{
			ensure(MeshInstanceIndex_To_LocalPoint_Map.Contains(*SampleIndexPtr));
			MeshInstanceIndex_To_LocalPoint_Map.Remove(*SampleIndexPtr);
		}
		else
		{
			ensure(false);
			return false;
		}
		LocalPoint_To_MeshInstanceIndex_Map.Remove(InPoint);

		ensure(LocalPoint_To_VoxelDataMap.Contains(InPoint));
		LocalPoint_To_VoxelDataMap.Remove(InPoint);
		return true;
	}

	FORCEINLINE const FIntVector& GetVoxelPoint(int32 InIndex)
	{
		if (const FIntVector* SamplePointPtr = MeshInstanceIndex_To_LocalPoint_Map.Find(InIndex))
		{
			return *SamplePointPtr;
		}
		else
		{
			return FIntVector::NoneValue;
		}
	}

	FORCEINLINE int32 GetVoxelInstanceIndex(const FIntVector& InPoint)
	{
		if (const int32* SampleIndexPtr = LocalPoint_To_MeshInstanceIndex_Map.Find(InPoint))
		{
			return *SampleIndexPtr;
		}
		else
		{
			return INDEX_NONE;
		}
	}

	FORCEINLINE bool ChangeVoxelInstanceIndex(int32 InPrevIndex, int32 InNewIndex)
	{
		if (MeshInstanceIndex_To_LocalPoint_Map.Contains(InPrevIndex))
		{
			FIntVector OldIndexPoint;
			MeshInstanceIndex_To_LocalPoint_Map.RemoveAndCopyValue(InPrevIndex, OldIndexPoint);
			MeshInstanceIndex_To_LocalPoint_Map.Add(InNewIndex, OldIndexPoint);
			return true;
		}
		ensure(false);
		return false;
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
	void BreakVoxelsWithInstanceIds(const TArray<int32>& InInstanceIndices);
//~ End Setters

//~ Begin Data
public:

	UFUNCTION(Category = "Data", BlueprintCallable)
	void UpdateStabilityData();

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	int32 VoxelChunkSize;

	UPROPERTY(Category = "Data", EditAnywhere, BlueprintReadOnly)
	float VoxelBaseSize;

protected:

	UPROPERTY()
	FVoxelContainersData Data;

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
