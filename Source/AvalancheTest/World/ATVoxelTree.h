// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "ScWTypes_Containers.h"

#include "World/ATTypes_World.h"

#include "ATVoxelTree.generated.h"

/**
 *
 */
UCLASS(meta = (DisplayName = "[AT] Voxel Tree"))
class AVALANCHETEST_API AATVoxelTree : public AActor
{
	GENERATED_BODY()
	
public:

	AATVoxelTree(const FObjectInitializer& InObjectInitializer = FObjectInitializer::Get());
	
//~ Begin Actor
protected:
	virtual void PostInitializeComponents() override; // AActor
	virtual void OnConstruction(const FTransform& InTransform) override; // AActor
	virtual void BeginPlay() override; // AActor
	virtual void Tick(float InDeltaSeconds) override; // AActor
	virtual void EndPlay(const EEndPlayReason::Type InReason) override; // AActor
//~ End Actor

//~ Begin Voxel Chunks
public:

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	bool IsChunkCoordsInsideTree(const FIntVector& InChunkCoords) const;

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

protected:
	void HandleChunkUpdates();
	void InitVoxelChunksInSquare(const FIntPoint& InSquareCenterXY, const int32 InSquareExtentXY);

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

	UFUNCTION(Category = "Voxel Generation", BlueprintCallable)
	int32 GetTreeSeed() const { return TreeSeed; }

protected:

	UPROPERTY(Category = "Voxel Generation", EditAnywhere, BlueprintReadOnly)
	int32 TreeSeed;
//~ End Voxel Generation

//~ Begin Voxel Getters
public:

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable)
	bool HasVoxelInstanceDataAtPoint(const FIntVector& InPoint, const bool bInIgnoreQueued = false) const;

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable, meta = (KeyWords = "GetInstanceData"))
	FVoxelInstanceData& GetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const bool bInChecked = true, const bool bInIgnoreQueued = false) const;

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable)
	int32 GetVoxelNeighborsNumAtPoint(const FIntVector& InPoint, const bool bInIgnoreQueued = false) const;

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable)
	void GetAllVoxelPointsInRadius(const FIntVector& InCenterPoint, int32 InRadius, TArray<FIntVector>& OutPoints) const;

	UFUNCTION(Category = "Voxel Getters | Break", BlueprintCallable)
	bool CanBreakVoxelAtPoint(const FIntVector& InPoint, const bool bInIgnoreQueued = false) const;

	UFUNCTION(Category = "Voxel Chunks | Bounds", BlueprintCallable)
	bool IsPointInsideTree(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Voxel Chunks | Bounds", BlueprintCallable)
	bool IsPointInsideSimulationReadyChunk(const FIntVector& InPoint) const;
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
//~ End Voxel Setters
	
//~ Begin Voxel Data
public:

	UFUNCTION(Category = "Voxel Data", BlueprintCallable)
	bool IsThisTickUpdatesTimeBudgetExceeded() const;

	UFUNCTION(Category = "Voxel Data", BlueprintCallable)
	void SetThisTickUpdatesTimeBudget(double InTimeSeconds);

	UFUNCTION(Category = "Voxel Data", BlueprintCallable)
	void ForceTickUpdateNextFrame();

	void ApplyQueued_Point_To_VoxelInstanceData_Map();
protected:
	void HandleQueuedVoxelInstanceData(const FIntVector& InPoint);

	void HandleTickUpdate_FromForceTickUpdate();
	void HandleTickUpdate(float InDeltaSeconds);

	UPROPERTY(Category = "Voxel Data", EditAnywhere, BlueprintReadOnly)
	double TickUpdatesTimeBudgetSeconds;

	UPROPERTY(Category = "Voxel Data", EditAnywhere, BlueprintReadOnly)
	double TickUpdatesTimeBudgetSeconds_PerQueuedChunkAdditive;

	UPROPERTY(Transient)
	uint64 ThisTickUpdatesTimeBudget_CyclesThreshold;

	UPROPERTY(Transient)
	TMap<FIntVector, FVoxelInstanceData> Queued_Point_To_VoxelInstanceData_Map;

	UPROPERTY(Transient)
	TSet<FIntVector> QueuedPointsSkipSimulationQueue;

	UPROPERTY(Transient)
	TMap<FIntVector, FVoxelInstanceData> Point_To_VoxelInstanceData_Map;

	UPROPERTY(Transient)
	FTimerHandle ForceTickUpdateNextFrameTimerHandle;
//~ End Voxel Data

//~ Begin Simulation
public:

	UFUNCTION(Category = "Simulation", BlueprintCallable)
	void QueueFullSimulationUpdateAtChunk(const FIntVector& InChunkCoords);

	UFUNCTION(Category = "Simulation", BlueprintCallable)
	class UATSimulationComponent* GetSimulationComponent() const { return SimulationComponent; }

protected:

	UPROPERTY(Category = "Simulation", EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UATSimulationComponent> SimulationComponent;
//~ End Simulation
	
//~ Begin Procedural Generation
public:

	UFUNCTION(Category = "Procedural Generation", BlueprintCallable)
	class UATProceduralGeneratorComponent* GetProceduralGeneratorComponent() const { return ProceduralGeneratorComponent; }

protected:

	UPROPERTY(Category = "Procedural Generation", EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UATProceduralGeneratorComponent> ProceduralGeneratorComponent;
//~ End Procedural Generation

//~ Begin Debug
public:

	UFUNCTION(Category = "Debug", BlueprintImplementableEvent, meta = (DisplayName = "HandleGameplayDebuggerToggled"))
	void BP_HandleGameplayDebuggerToggled(const bool bInWasActivated) const;
//~ End Debug
};
