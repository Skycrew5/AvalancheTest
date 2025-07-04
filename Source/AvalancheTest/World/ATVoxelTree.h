// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "ScWTypes_Containers.h"

#include "World/ATTypes_World.h"

#include "ATVoxelTree.generated.h"

UCLASS(meta = (DisplayName = "[AT] Voxel Tree"))
class AVALANCHETEST_API AATVoxelTree : public AActor
{
	GENERATED_BODY()
	
public:

	AATVoxelTree(const FObjectInitializer& InObjectInitializer = FObjectInitializer::Get());
	
//~ Begin Initialize
protected:
	virtual void PostInitializeComponents() override; // AActor
	virtual void OnConstruction(const FTransform& InTransform) override; // AActor
	virtual void BeginPlay() override; // AActor
	virtual void Tick(float InDeltaSeconds) override; // AActor
	virtual void EndPlay(const EEndPlayReason::Type InReason) override; // AActor
//~ End Initialize

//~ Begin Voxel Chunks
public:

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	FIntVector GetVoxelChunkCoordsAtPoint(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	class AATVoxelChunk* GetVoxelChunkAtCoords(const FIntVector& InChunkCoords) const { return ChunksMap.FindRef(InChunkCoords, nullptr); }

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	class AATVoxelChunk* GetVoxelChunkAtPoint(const FIntVector& InPoint) const { return GetVoxelChunkAtCoords(GetVoxelChunkCoordsAtPoint(InPoint)); }

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	int32 GetChunkSize() const { return ChunkSize; }

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	float GetVoxelSize() const { return VoxelSize; }

protected:

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	void InitVoxelChunks();

	UPROPERTY(Category = "Voxel Chunks", EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AATVoxelChunk> ChunkClass;

	UPROPERTY(Category = "Voxel Chunks", EditAnywhere, BlueprintReadOnly)
	FIntVector TreeSizeInChunks;

	UPROPERTY(Category = "Voxel Chunks", EditAnywhere, BlueprintReadOnly)
	int32 ChunkSize;

	UPROPERTY(Category = "Voxel Chunks", EditAnywhere, BlueprintReadOnly)
	float VoxelSize;

	UPROPERTY()
	TMap<FIntVector, TObjectPtr<AATVoxelChunk>> ChunksMap;
//~ End Voxel Chunks

//~ Begin Voxel Getters
public:

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable)
	bool HasVoxelInstanceDataAtPoint(const FIntVector& InPoint, const bool bInIgnoreQueued = false) const;

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable, meta = (KeyWords = "GetInstanceData"))
	struct FVoxelInstanceData& GetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const bool bInChecked = true, const bool bInIgnoreQueued = false) const;

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable)
	int32 GetVoxelNeighborsNumAtPoint(const FIntVector& InPoint, const bool bInIgnoreQueued = false) const;

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable)
	void GetAllVoxelPointsInRadius(const FIntVector& InCenterPoint, int32 InRadius, TArray<FIntVector>& OutPoints) const;

	UFUNCTION(Category = "Voxel Getters | Break", BlueprintCallable)
	bool CanBreakVoxelAtPoint(const FIntVector& InPoint, const bool bInIgnoreQueued = false) const;

	UFUNCTION(Category = "Voxel Chunks | Bounds", BlueprintCallable)
	const FIntVector& GetBoundsSize() const { return BoundsSize; }

	UFUNCTION(Category = "Voxel Chunks | Bounds", BlueprintCallable)
	bool IsPointInsideTree(const FIntVector& InPoint) const;

protected:

	UPROPERTY()
	FIntVector BoundsSize;
//~ End Voxel Getters
	
//~ Begin Voxel Setters
public:

	UFUNCTION(Category = "Voxel Setters | Set", BlueprintCallable, meta = (KeyWords = "AddVoxelAt, PlaceVoxelAt"))
	bool SetVoxelAtPoint(const FIntVector& InPoint, const class UATVoxelTypeData* InTypeData, const bool bInForced = false);
	
	UFUNCTION(Category = "Voxel Setters | Set", BlueprintCallable, meta = (KeyWords = "AddVoxelsAt, PlaceVoxelsAt"))
	bool SetVoxelsAtPoints(const TArray<FIntVector>& InPoints, const class UATVoxelTypeData* InTypeData, const bool bInForced = false);

	UFUNCTION(Category = "Voxel Setters | Break", BlueprintCallable, meta = (KeyWords = "RemoveVoxelAt, DeleteVoxelAt"))
	bool BreakVoxelAtPoint(const FIntVector& InPoint, const bool bInForced = false, const bool bInNotify = false);

	UFUNCTION(Category = "Voxel Setters | Break", BlueprintCallable, meta = (KeyWords = "RemoveVoxelsAt, DeleteVoxelsAt"))
	bool BreakVoxelsAtPoints(const TArray<FIntVector>& InPoints, const bool bInForced = false, const bool bInNotify = false);

	UFUNCTION(Category = "Voxel Setters | Per Chunk", BlueprintCallable)
	void FillChunkWithVoxels(const FIntVector& InChunkCoords, const UATVoxelTypeData* InTypeData, const bool bInForced = false);

	UFUNCTION(Category = "Voxel Setters | Per Chunk", BlueprintCallable, meta = (KeyWords = "ClearAll, BreakAllVoxels"))
	void BreakAllVoxelsAtChunk(const FIntVector& InChunkCoords, const bool bInForced = false);

	UFUNCTION(Category = "Voxel Setters | Per Chunk", BlueprintCallable)
	void CreateFoundationAtChunk(const FIntVector& InChunkCoords);
//~ End Voxel Setters
	
//~ Begin Voxel Data
public:

protected:
	void ApplyQueued_Point_To_VoxelInstanceData_Map();

	UPROPERTY(Category = "Voxel Data", EditAnywhere, BlueprintReadOnly)
	bool bEnableVoxelDataUpdatesTick;

	UPROPERTY(Category = "Voxel Data", EditAnywhere, BlueprintReadOnly)
	int32 MaxUpdatesPerSecond;

	UPROPERTY(Category = "Voxel Data", EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const class UATVoxelTypeData> FoundationVoxelTypeData;

	UPROPERTY(Transient)
	TMap<FIntVector, FVoxelInstanceData> Queued_Point_To_VoxelInstanceData_Map;

	UPROPERTY(Transient)
	TMap<FIntVector, FVoxelInstanceData> Point_To_VoxelInstanceData_Map;
//~ End Voxel Data

//~ Begin Simulation
public:

	UFUNCTION(Category = "Simulation", BlueprintCallable)
	void QueueFullUpdateAtChunk(const FIntVector& InChunkCoords);

protected:

	struct FRecursiveThreadData
	{
		const TArray<EATAttachmentDirection>* DirectionsOrderPtr;
		TSet<FIntVector> ThisOrderUpdatedPoints;

		FRecursiveThreadData(const TArray<EATAttachmentDirection>& InOrder)
			: DirectionsOrderPtr(&InOrder), ThisOrderUpdatedPoints({}) {
		}
	};

	struct FRecursivePointCache
	{
		bool bIsThreadSafe;
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

	void HandleVoxelDataUpdatesTick(int32& InOutMaxUpdates);

	void QueueRecursiveStabilityUpdate(const FIntVector& InPoint, const bool bInQueueNeighborsToo = true);
	TArraySetPair<FIntVector> QueuedRecursiveStabilityUpdatePoints;

	void UpdateStabilityRecursive(int32& InOutUpdatesLeft);
	TMap<FIntVector, FRecursivePointCache> UpdateStabilityRecursive_PointsCache;

	float UpdateStabilityRecursive_GetStabilityFromAllNeighbors(const FIntVector& InTargetPoint, FRecursiveThreadData& InThreadData, EATAttachmentDirection InNeighborDirection = EATAttachmentDirection::None, uint8 InCurrentRecursionLevel = 0u);

	void UpdateHealth(int32& InOutUpdatesLeft);
	TArraySetPair<FIntVector> InDangerGroupHealthUpdatePoints;
//~ End Simulation
};
