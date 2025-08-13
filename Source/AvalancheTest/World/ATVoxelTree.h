// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "ScWTypes_CommonContainers.h"

#include "World/ATTypes_World.h"

#include "ATVoxelTree.generated.h"

static TAutoConsoleVariable<int32> CVarVoxelTree_ForceSyncUpdates(
	TEXT("VoxelTree.ForceSyncUpdates"),
	0,
	TEXT("Force sync updates"),
	ECVF_Default
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBreakVoxelsEventSignature, const class UATVoxelTypeData*, InTypeData, const TArray<FIntVector>&, InPoints, const FVoxelBreakData&, InBreakData);

/**
 *
 */
UCLASS(meta = (DisplayName = "[AT] Voxel Tree"))
class AVALANCHETEST_API AATVoxelTree : public AActor
{
	GENERATED_BODY()

	friend class UATSaveGame_VoxelTree;
	
public:

	AATVoxelTree(const FObjectInitializer& InObjectInitializer = FObjectInitializer::Get());
	
//~ Begin Actor
protected:
	virtual void PostInitializeComponents() override; // AActor
	virtual void OnConstruction(const FTransform& InTransform) override; // AActor
	virtual void BeginPlay() override; // AActor
	virtual void Tick(float InDeltaSeconds) override; // AActor
	virtual bool ShouldTickIfViewportsOnly() const override { return IS_EDITOR_WORLD(); } // AActor
	virtual void EndPlay(const EEndPlayReason::Type InReason) override; // AActor
	virtual void Reset() override; // AActor
//~ End Actor

//~ Begin Tick Updates
public:

	UFUNCTION(Category = "Tick Updates", BlueprintCallable)
	bool IsThisTickUpdatesTimeBudgetExceeded() const;

	UFUNCTION(Category = "Tick Updates", BlueprintCallable)
	void SetThisTickUpdatesTimeBudget(double InTimeMs);

	UFUNCTION(Category = "Tick Updates", BlueprintCallable)
	void ForceTickUpdateNextFrame();

protected:
	void HandleTickUpdate_FromForceTickUpdate();
	void HandleTickUpdate(float InDeltaSeconds);

	UPROPERTY(Category = "Tick Updates", EditAnywhere, BlueprintReadOnly)
	double TickUpdatesTimeBudgetMs;

	UPROPERTY(Category = "Tick Updates", EditAnywhere, BlueprintReadOnly)
	double TickUpdatesTimeBudgetMs_PerQueuedChunkAdditive;

	UPROPERTY(Category = "Tick Updates", EditAnywhere, BlueprintReadOnly)
	double TickUpdatesTimeBudgetMs_PerSkipSimulationPointQueueAdditive;

	UPROPERTY(Transient)
	uint64 ThisTickUpdatesTimeBudget_CyclesThreshold;
//~ End Tick Updates

//~ Begin Voxel Chunks
public:

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	const TMap<FIntVector, AATVoxelChunk*>& GetChunksMap() const { return ChunksMap; }

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	bool IsChunkCoordsInsideTree(const FIntVector& InChunkCoords) const;

	UFUNCTION(Category = "Voxel Tree", BlueprintCallable)
	bool IsChunkCoordsOnTreeSide(const FIntVector& InChunkCoords, const bool bInIgnoreBottom) const;

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	FIntVector GetVoxelChunkCoordsAtPoint(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	class AATVoxelChunk* GetVoxelChunkAtCoords(const FIntVector& InChunkCoords) const { return ChunksMap.FindRef(InChunkCoords, nullptr); }

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	class AATVoxelChunk* GetVoxelChunkAtPoint(const FIntVector& InPoint) const { return GetVoxelChunkAtCoords(GetVoxelChunkCoordsAtPoint(InPoint)); }

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	const FIntVector& GetTreeSizeInChunks() const { return TreeSizeInChunks; }

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

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	void UpdateChunksForUpdateReferenceActor(const AActor* InActor);

	UFUNCTION(Category = "Voxel Chunks", BlueprintCallable)
	void MarkAllChunksAsSimulationReady();

	UFUNCTION(Category = "Voxel Generation", BlueprintCallable)
	void InitAllChunks();

	UFUNCTION(Category = "Voxel Generation", BlueprintCallable)
	void RemoveAllChunks();

	const TArray<const AActor*>& GetChunksUpdateReferenceActors() const { return ChunksUpdateReferenceActors; }
protected:
	void UpdateBoundsSize() { BoundsSize = TreeSizeInChunks * ChunkSize; }
	void HandleChunkUpdates();
	void UpdateSortedChunkArray();
	void InitVoxelChunksInSquare(const FIntPoint& InSquareCenterXY, const int32 InSquareExtentXY, TArray<AATVoxelChunk*>& OutNewChunks, const bool bInReplacePrevChunks = false);
	void RemoveVoxelChunkAtPoint(const FIntVector& InPoint);

	UPROPERTY(Category = "Voxel Chunks", EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AATVoxelChunk> ChunkClass;

	UPROPERTY(Category = "Voxel Chunks", EditAnywhere, BlueprintReadOnly)
	FIntVector TreeSizeInChunks;

	UPROPERTY(Category = "Voxel Chunks", EditAnywhere, BlueprintReadOnly)
	int32 ChunkSize;

	UPROPERTY(Category = "Voxel Chunks", EditAnywhere, BlueprintReadOnly)
	float VoxelSize;

	UPROPERTY(Category = "Voxel Chunks", EditAnywhere, BlueprintReadWrite)
	int32 ChunksUpdateMaxSquareExtent;

	UPROPERTY(Transient)
	FIntVector BoundsSize;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<TObjectPtr<const AActor>> ChunksUpdateReferenceActors;

	UPROPERTY(Transient)
	TMap<FIntVector, AATVoxelChunk*> ChunksMap;

	UPROPERTY(Transient)
	FSortedChunksBySquaredDistance SortedChunksData;

	UPROPERTY(Transient)
	FTimerHandle UpdateSortedChunkArrayTimerHandle;
//~ End Voxel Chunks
	
//~ Begin Voxel Generation
public:

	UFUNCTION(Category = "Voxel Generation", BlueprintCallable)
	int32 GetTreeSeed() const { return TreeSeed; }

	UFUNCTION(Category = "Voxel Generation", BlueprintCallable)
	void HandleGenerate(bool bInAsync, int32 InTreeSeed);

protected:

	UPROPERTY(Category = "Voxel Generation", EditAnywhere, BlueprintReadOnly)
	int32 TreeSeed;
//~ End Voxel Generation

//~ Begin Voxel Getters
public:

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable, meta = (AdvancedDisplay = "bInIgnoreQueued"))
	bool HasVoxelInstanceDataAtPoint(const FIntVector& InPoint, const bool bInIgnoreQueued = false) const;

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable, meta = (KeyWords = "GetInstanceData", AdvancedDisplay = "bInChecked, bInIgnoreQueued"))
	FVoxelInstanceData& GetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const bool bInChecked = true, const bool bInIgnoreQueued = false) const;

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable, meta = (AdvancedDisplay = "bInIgnoreQueued"))
	int32 GetVoxelNeighborsNumAtPoint(const FIntVector& InPoint, const bool bInIgnoreQueued = false) const;

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable)
	void GetAllVoxelPointsInRadius(const FIntVector& InCenterPoint, int32 InRadius, TArray<FIntVector>& OutPoints) const;

	UFUNCTION(Category = "Voxel Getters | Break", BlueprintCallable, meta = (AdvancedDisplay = "bInIgnoreQueued"))
	bool CanBreakVoxelAtPoint(const FIntVector& InPoint, const bool bInIgnoreQueued = false) const;

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable)
	bool IsPointInsideTree(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Voxel Getters", BlueprintCallable)
	bool IsPointInsideSimulationReadyChunk(const FIntVector& InPoint) const;
//~ End Voxel Getters
	
//~ Begin Voxel Setters
public:

	UFUNCTION(Category = "Voxel Setters | Set", BlueprintCallable, meta = (KeyWords = "AddVoxelAt, PlaceVoxelAt"))
	bool SetVoxelAtPoint(const FIntVector& InPoint, const class UATVoxelTypeData* InTypeData, const bool bInForced = false);
	
	UFUNCTION(Category = "Voxel Setters | Set", BlueprintCallable, meta = (KeyWords = "AddVoxelsAt, PlaceVoxelsAt"))
	bool SetVoxelsAtPoints(const TArray<FIntVector>& InPoints, const class UATVoxelTypeData* InTypeData, const bool bInForced = false);

	UFUNCTION(Category = "Voxel Setters | Break", BlueprintCallable, meta = (KeyWords = "RemoveVoxelAt, DeleteVoxelAt", AutoCreateRefTerm = "InBreakData"))
	bool BreakVoxelAtPoint(const FIntVector& InPoint, const FVoxelBreakData& InBreakData);

	UFUNCTION(Category = "Voxel Setters | Break", BlueprintCallable, meta = (KeyWords = "RemoveVoxelsAt, DeleteVoxelsAt", AutoCreateRefTerm = "InBreakData"))
	bool BreakVoxelsAtPoints(const TArray<FIntVector>& InPoints, const FVoxelBreakData& InBreakData);

	UFUNCTION(Category = "Voxel Setters | Break", BlueprintCallable, meta = (KeyWords = "ClearAll, BreakAllVoxels"))
	void BreakAllVoxelsAtChunk(const FIntVector& InChunkCoords, const bool bInForced = false);

	UFUNCTION(Category = "Voxel Setters | Per Chunk", BlueprintCallable)
	void FillChunkWithVoxels(const FIntVector& InChunkCoords, const UATVoxelTypeData* InTypeData, const bool bInForced = false);

	UPROPERTY(Category = "Voxel Setters | Break", BlueprintAssignable)
	FBreakVoxelsEventSignature OnBreakVoxelsAtPoints;

protected:
	void HandleBrokenVoxelsUpdates();

	struct FBrokenVoxelsData
	{
		TArray<FIntVector> Points;
		FVoxelBreakData BreakData;
	};

	TMap<const class UATVoxelTypeData*, FBrokenVoxelsData> Queued_VoxelTypeData_To_BrokenVoxelsData_Map;
//~ End Voxel Setters
	
//~ Begin Voxel Data
public:

	UFUNCTION(Category = "Voxel Data", BlueprintCallable)
	void SaveData(const FString& InSaveSlot);

	UFUNCTION(Category = "Voxel Data", BlueprintCallable)
	void LoadData(const FString& InSaveSlot);

	UFUNCTION(Category = "Voxel Data", BlueprintCallable)
	void ResetData();

	void ApplyQueued_Point_To_VoxelInstanceData_Map();
protected:
	void HandleQueuedVoxelInstanceData(const FIntVector& InPoint);

	UPROPERTY(Transient)
	TMap<FIntVector, FVoxelInstanceData> Queued_Point_To_VoxelInstanceData_Map;

	UPROPERTY(Transient)
	TSet<FIntVector> Queued_PointsSkipSimulationQueue_Set;

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
