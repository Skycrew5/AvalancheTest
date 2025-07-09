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
	
	friend class FATVoxelTree_SimulationAsyncTask;

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
	bool IsInitializingVoxelChunks() const { return bIsInitializingVoxelChunks; }

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

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	const FIntVector& GetBoundsSize() const { return BoundsSize; }

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	void RegisterChunksUpdateReferenceActor(const AActor* InActor);

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	void UnRegisterChunksUpdateReferenceActor(const AActor* InActor);

	UPROPERTY(Category = "Voxel Chunks", EditAnywhere, BlueprintReadOnly)
	float PerlinPow = 3.0f;

protected:
	void HandleChunkUpdates(int32& InOutUpdatesLeft);
	void InitVoxelChunksInSquare(const FIntPoint& InSquareCenterXY, const int32 InSquareExtentXY, int32& InOutUpdatesLeft);

	UPROPERTY(Category = "Simulation", BlueprintReadOnly)
	bool bIsInitializingVoxelChunks;

	UPROPERTY(Category = "Voxel Chunks", EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AATVoxelChunk> ChunkClass;

	UPROPERTY(Category = "Voxel Chunks", EditAnywhere, BlueprintReadOnly)
	FIntVector TreeSizeInChunks;

	UPROPERTY(Category = "Voxel Chunks", EditAnywhere, BlueprintReadOnly)
	int32 ChunkSize;

	UPROPERTY(Category = "Voxel Chunks", EditAnywhere, BlueprintReadOnly)
	float VoxelSize;

	UPROPERTY(Category = "Voxel Chunks", EditAnywhere, BlueprintReadOnly)
	int32 ChunksUpdateMaxSquareExtent;

	UPROPERTY(Transient)
	FIntVector BoundsSize;

	UPROPERTY(Transient)
	TArray<TObjectPtr<const AActor>> ChunksUpdateReferenceActors;

	UPROPERTY(Transient)
	TMap<FIntVector, TObjectPtr<AATVoxelChunk>> ChunksMap;
//~ End Voxel Chunks
	
//~ Begin Voxel Generation
public:

	UFUNCTION(Category = "Voxel Generation", BlueprintCallable, meta = (KeyWords = "GetVoxelGenerator_Perlin"))
	class UFastNoise2PerlinGenerator* GetVoxelPerlinGenerator() const { return VoxelGenerator_Perlin; }

	UFUNCTION(Category = "Voxel Generation", BlueprintCallable)
	int32 GetTreeSeed() const { return TreeSeed; }

protected:
	void InitVoxelGenerators();

	UPROPERTY(Category = "Voxel Generation", EditAnywhere, BlueprintReadOnly)
	int32 TreeSeed;

	UPROPERTY(Transient)
	TObjectPtr<class UFastNoise2PerlinGenerator> VoxelGenerator_Perlin;
//~ End Voxel Generation

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
	bool IsPointInsideTree(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Voxel Chunks | Bounds", BlueprintCallable)
	bool IsPointInsideLoadedChunk(const FIntVector& InPoint) const;
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

	UFUNCTION(Category = "Voxel Data", BlueprintCallable)
	void ForceTickUpdateNextFrame();

	UPROPERTY(Category = "Voxel Data", EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const class UATVoxelTypeData> DefaultVoxelTypeData;

	UPROPERTY(Category = "Voxel Data", EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const class UATVoxelTypeData> WeakVoxelTypeData;

	UPROPERTY(Category = "Voxel Data", EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const class UATVoxelTypeData> FoundationVoxelTypeData;

protected:
	void HandleTickUpdate_FromForceTickUpdate();
	void HandleTickUpdate(float InDeltaSeconds);

	void ApplyQueued_Point_To_VoxelInstanceData_Map();
	void HandleQueuedVoxelInstanceData(const FIntVector& InPoint);

	UPROPERTY(Category = "Voxel Data", EditAnywhere, BlueprintReadOnly)
	int32 MaxUpdatesPerSecond;

	UPROPERTY(Transient)
	TMap<FIntVector, FVoxelInstanceData> Queued_Point_To_VoxelInstanceData_Map;

	UPROPERTY(Transient)
	TMap<FIntVector, FVoxelInstanceData> Point_To_VoxelInstanceData_Map;

	UPROPERTY(Transient)
	FTimerHandle ForceTickUpdateNextFrameTimerHandle;
//~ End Voxel Data

//~ Begin Simulation
public:

	UFUNCTION(Category = "Simulation", BlueprintCallable)
	void QueueFullUpdateAtChunk(const FIntVector& InChunkCoords);

protected:

	UPROPERTY(Category = "Simulation", EditAnywhere, BlueprintReadOnly)
	bool bEnableSimulationUpdates;

	UPROPERTY(Category = "Simulation", EditAnywhere, BlueprintReadOnly)
	float VoxelHealthDrainSpeedMul;

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

	void HandleSimulationUpdates(int32& InOutUpdatesLeft);

	void QueueRecursiveStabilityUpdate(const FIntVector& InPoint, const bool bInQueueNeighborsToo = true);
	TArraySetPair<FIntVector> QueuedRecursiveStabilityUpdatePoints;

	void UpdateStabilityRecursive_TryStartBackgroundTask(int32& InOutUpdatesLeft);
	TArray<FIntVector> UpdateStabilityRecursive_SelectedUpdatePoints;

	void UpdateStabilityRecursive_DoWork();
	float UpdateStabilityRecursive_GetStabilityFromAllNeighbors(const FIntVector& InTargetPoint, FRecursiveThreadData& InThreadData, EATAttachmentDirection InNeighborDirection = EATAttachmentDirection::None, uint8 InCurrentRecursionLevel = 0u);
	TMap<FIntVector, FRecursivePointCache> UpdateStabilityRecursive_PointsCache;

	void UpdateInDangerGroupHealthPoints(int32& InOutUpdatesLeft);

	void UpdateHealth(int32& InOutUpdatesLeft);
	TArray<FIntVector> QueuedInDangerGroupHealthUpdatePoints;
	TArraySetPair<FIntVector> InDangerGroupHealthUpdatePoints;

	FAsyncTask<class FATVoxelTree_SimulationAsyncTask>* SimulationAsyncTaskPtr;
//~ End Simulation

//~ Begin Debug
public:

	UFUNCTION(Category = "Debug", BlueprintImplementableEvent, meta = (DisplayName = "HandleGameplayDebuggerToggled"))
	void BP_HandleGameplayDebuggerToggled(const bool bInWasActivated) const;
//~ End Debug
};

class FATVoxelTree_SimulationAsyncTask : public FNonAbandonableTask
{
public:

	void DoWork()
	{
		ensureReturn(TargetTree);
		TargetTree->UpdateStabilityRecursive_DoWork();
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FATVoxelTree_SimulationAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	TObjectPtr<AATVoxelTree> TargetTree;
};
