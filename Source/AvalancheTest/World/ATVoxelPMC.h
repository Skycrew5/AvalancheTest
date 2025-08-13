// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "ScWTypes_CommonContainers.h"
#include "World/ATTypes_World.h"
#include "ProceduralMeshComponent.h"

#include "ATVoxelPMC.generated.h"

USTRUCT()
struct FPlaneData
{
	GENERATED_BODY()

	UPROPERTY() 
	float A;

	UPROPERTY()
	float B;

	UPROPERTY()
	float C;

	UPROPERTY()
	float D;

	FPlaneData() = default;

	FORCEINLINE FPlaneData(float InA, float InB, float InC, float InD) : A(InA), B(InB), C(InC), D(InD) {}
};

UENUM()
enum class EVoxelPlaneType : uint8
{
	Side,
	EdgeSection
};

//USTRUCT()
//struct FIntArray
//{
//	GENERATED_BODY()
//
//	UPROPERTY()
//	TArray<int32> DataArray;
//
//	FIntArray() = default;
//
//	FIntArray(const TArray<int32>& InDataArray) : DataArray(InDataArray) {}
//
//	bool operator==(const FIntArray& Other) const
//	{
//		if (DataArray.Num() != Other.DataArray.Num()) return false;
//
//		for (int32 i = 0; i < DataArray.Num(); ++i)
//		{
//			if (DataArray[i] != Other.DataArray[i])
//				return false;
//		}
//
//		return true;
//	}
//
//	bool operator!=(const FIntArray& Other) const
//	{
//		return !(*this == Other);
//	}
//};
//
//inline uint32 GetTypeHash(const FIntArray& InIntArray)
//{
//	uint32 Hash = 0;
//
//	for (const int32& Value : InIntArray.DataArray)
//	{
//		Hash = HashCombine(Hash, GetTypeHash(Value));
//	}
//	return Hash;
//}
//
//USTRUCT()
//struct FIntToIntArrayMap
//{
//	GENERATED_BODY()
//
//	UPROPERTY()
//	TMap<int32, FIntArray> DataMap;
//
//	FIntToIntArrayMap() = default;
//};

/**
 * 
 */
UCLASS()
class AVALANCHETEST_API UATVoxelPMC : public UProceduralMeshComponent
{
	GENERATED_BODY()

public:

	UATVoxelPMC(const FObjectInitializer& ObjectInitializer);

	//~ Begin Initialize
protected:
	virtual void OnRegister() override; // UActorComponent
	virtual void BeginPlay() override; // UActorComponent
	virtual void EndPlay(const EEndPlayReason::Type InReason) override; // UActorComponent
public:

	UFUNCTION(Category = "Initialize", BlueprintNativeEvent, meta = (DisplayName = "InitComponent"))
	void BP_InitComponent(class AATVoxelChunk* InOwnerChunk, const class UATVoxelTypeData* InTypeData);
	//~ End Initialize

	//~ Begin Getters
public:

	UFUNCTION(Category = "Getters | Voxel Instance Data", BlueprintCallable)
	int32 GetVoxelInstancesNum() const { return Point_To_MeshIndex_Map.Num(); }

	UFUNCTION(Category = "Getters", BlueprintCallable)
	bool HasVoxelOfThisTypeAtPoint(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	bool HasVoxelOfAnyTypeAtPoint(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	bool CouldCreateMeshAtPoint(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable, meta = (KeyWords = "GetInstanceData"))
	FVoxelInstanceData& GetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const bool bInChecked = true) const;

	UFUNCTION(Category = "Getters | Mesh Index", BlueprintCallable)
	bool HasMeshAtPoint(const FIntVector& InPoint) const { return Point_To_MeshIndex_Map.Contains(InPoint); }

	UFUNCTION(Category = "Getters | Mesh Index", BlueprintCallable)
	int32 GetMeshIndexAtPoint(const FIntVector& InPoint) const { return Point_To_MeshIndex_Map.FindRef(InPoint, INDEX_NONE); }

	//UFUNCTION(Category = "Getters | Mesh Index", BlueprintCallable)
	//FIntVector GetPointOfMeshIndex(int32 InMeshIndex, const bool bInChecked = true) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	bool IsVoxelSidesClosed(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	bool IsVoxelEdgesClosed(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	float GetVoxelSize() const;
	//~ End Getters

	//~ Begin Setters
public:

	UFUNCTION(Category = "Setters | Voxel Instance Data", BlueprintCallable, meta = (KeyWords = "AddVoxelAt, PlaceVoxelAt"))
	void HandleSetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const struct FVoxelInstanceData& InVoxelInstanceData);

	UFUNCTION(Category = "Setters | Voxel Instance Data", BlueprintCallable, meta = (AutoCreateRefTerm = "InBreakData"))
	bool HandleBreakVoxelAtPoint(const FIntVector& InPoint, const FVoxelBreakData& InBreakData);

	bool HandleMeshInstanceIndexRelocated(int32 InPrevIndex, int32 InNewIndex);
protected:

	//~ End Setters

	//~ Begin Data
public:
	void HandleUpdates();

	UFUNCTION(Category = "Data", BlueprintCallable)
	int32 GetQueuedVisibilityUpdatePointsNum() const { return QueuedVisibilityUpdatePoints.Num(); }

	UFUNCTION(Category = "Data", BlueprintCallable)
	void QueuePointForVisibilityUpdate(const FIntVector& InPoint, const bool bInQueueNeighborsToo = true);

	UFUNCTION(Category = "Data", BlueprintCallable, meta = (KeyWords = "UpdateVoxelsVisibilityState"))
	void ForceUpdateAllVoxelsVisibilityState() { UpdateVoxelsVisibilityState(true); }

	void UpdateVoxelsVisibilityState(const bool bInIgnoreTimeBugdet = false);

	UPROPERTY(Transient)
	bool bEnableVoxelsVisibilityUpdates;

protected:

	UPROPERTY(Category = "Data", BlueprintReadOnly)
	TObjectPtr<class AATVoxelChunk> OwnerChunk;

	UPROPERTY(Category = "Data", BlueprintReadOnly)
	TObjectPtr<const class UATVoxelTypeData> VoxelTypeData;

	UPROPERTY(Transient)
	TMap<FIntVector, int32> Point_To_MeshIndex_Map;

	//UPROPERTY(Transient)
	//TSet<FIntVector> MeshedPointsSet;

	UPROPERTY(Transient)
	TSet<FIntVector> PossibleMeshPointsSet;

	//UPROPERTY(Transient)
	TArraySetPair<FIntVector> QueuedVisibilityUpdatePoints;
	//~ End Data

	//~ Begin Mesh
protected:
	static void Static_OnISMInstanceIndicesUpdated(UInstancedStaticMeshComponent* InUpdatedComponent, TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData> InIndexUpdates);

	void HandleInstanceIndicesUpdates(const TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData>& InIndexUpdates);

	void GenerateUnattachedSideIndexesCombinations(TArray<TArray<int32>>& OutCombinationsArray);

	void GenerateUnattachedEdgeIndexesCombinations(TArray<TArray<int32>>& OutCombinationsArray);

	void GenerateCombinations(const TArray<int32>& InElements, int32 InStartIndex, int32 InCurrentLength, int32 InMaxLength, TArray<int32>& InCurrentCombination, TArray<TArray<int32>>& OutCombinationsArray);

	void CreateVoxelMeshTemplate(TArray<int32>& InUnattachedSideIndexesArray, TArray<int32>& InUnattachedEdgeIndexesArray);

	void CreateMesh(TArray<FIntVector>& InVisibleVoxelPointsArray);

	void GetUnattachedSideIndexes(FIntVector& InPoint, TArray<int32>& OutUnattachedSideIndexesArray);

	void GetUnattachedEdgeIndexes(FIntVector& InPoint, TArray<int32>& OutUnattachedEdgeIndexesArray);

	bool IsEdgeSectionPlanesIntersectable(const int32& InFirstEdgeSectionPlaneIndex, const int32& InSecondEdgeSectionPlaneIndex) const;

	bool IsSidePlanesIntersectable(const int32& InFirstSidePlaneIndex, const int32& InSecondSidePlaneIndex) const;

	FORCEINLINE bool TryIntersectThreePlanes(const FPlaneData& InFirstPlane, const FPlaneData& InSecondPlane, const FPlaneData& InThirdPlane, FVector& OutPoint) const;

	TArray<FPlaneData> VoxelSidePlaneDataArray;

	TArray<FPlaneData> EdgeSectionPlaneDataArray;

	TArray<FPlaneData> AngleSectionPlaneDataArray;

	TMap<FIntPoint, int32> SideIndexesPair_to_EdgeIndex_Map;

	TArray<FIntPoint> UnattachedSideIndexesPairsArray;

	TMap<int32, TArray<int32>> SideIndex_to_IntersectableSideIndexesArray_Map;

	TMap<int32, TArray<int32>> SideIndex_to_AngleIndexesArray_Map;

	TMap<FIntPoint, int32> SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map;

	TMap<FIntPoint, int32> SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map;

	TMap<int32, TArray<int32>> EdgeIndex_to_IntersectableSideIndexesArray_Map;

	TMap<int32, TArray<int32>> EdgeIndex_to_IntersectableEdgeIndexesArray_Map;

	TMap<
		TArray<TArray<int32>>, // [0] - Unattached side indexes array, [1] - Unattached edge indexes array
		TMap<
			int32, // Side index
			TArray<FVector>>> // Side belong vertices array
	UnattachedSideAndEdgeIndexes_to_VoxelSideIndexToBelongVerticesMap_Map;

	//TMap<TArray<int32>, TMap<int32, TArray<FVector>>> UnattachedSideIndexes_to_VoxelSideIndexToBelongVerticesMap_Map;

	TMap<
		TArray<TArray<int32>>, // [0] - Unattached side indexes array, [1] - Unattached edge indexes array
		TMap<
			int32, // Edge section index
			TArray<FVector>>> // Edge section belong vertices array
	UnattachedSideAndEdgeIndexes_to_VoxelEdgeIndexToBelongVerticesMap_Map;

	//TMap<TArray<int32>, TMap<int32, TArray<FVector>>> UnattachedSideIndexes_to_EdgeIndexToBelongVerticesMap_Map;

	TMap<
		TArray<TArray<int32>>, // [0] - Unattached side indexes array, [1] - Unattached edge indexes array
		TMap<
			int32, // Angle section index
			TArray<FVector>>> // Belong vertices array
	UnattachedSideAndEdgeIndexes_to_AngleIndexToBelongVerticesMap_Map;

	TMap<
		TArray<TArray<int32>>, // [0] - Unattached side indexes array, [1] - Unattached edge indexes array
		TArray<
			TPair<
				int32, // Angle section index
				TArray<FVector>>>> // Belong vertices array
	UnattachedSideAndEdgeIndexes_to_AngleIndexToVerticesPairsArray_Map; // To store unintentionally truncated angle vertices (for angles truncated by edge truncation)

	//TMap<
	//	TArray<int32>, // Unattached side indexes array
	//	TArray<
	//		TPair<
	//			int32, // Angle section index
	//			TArray<FVector>>>> // vertices array
	//UnattachedSideIndexes_to_AngleIndexToVerticesPairsArray_Map;

	TMap<FIntPoint, int32> EdgeIndexesPair_to_AngleIndex_Map;

	TMap<int32, TArray<int32>> AngleIndex_to_SideIndexes_Map;

	TMap<int32, TArray<int32>> AngleIndex_to_EdgeIndexes_Map;

	TMap<int32, FVector> SideIndex_to_MeshNormal_Map;

	TMap<int32, FVector> EdgeIndex_to_MeshNormal_Map;

	TMap<int32, FVector> AngleIndex_to_MeshNormal_Map;

	//static FDelegateHandle InstanceIndexUpdatedDelegateHandle;
	//~ End Mesh

	//~ Begin Debug
public:

	UFUNCTION(Category = "Debug", BlueprintCallable)
	void UpdateVoxelsDebugState();

	FORCEINLINE void TryQueuePointForDebugUpdate(const FIntVector& InPoint) { if (bDebugStabilityValues || bDebugHealthValues) QueuedDebugUpdatePoints.Add(InPoint); }
	//FORCEINLINE void TryQueueMeshIndexForDebugUpdate(int32 InMeshIndex) { TryQueuePointForDebugUpdate(GetPointOfMeshIndex(InMeshIndex)); }

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

	//void Debug_UpdateStabilityValueAtMeshIndex(int32 InMeshIndex) { Debug_UpdateStabilityValueAtPoint(GetPointOfMeshIndex(InMeshIndex)); }
	//void Debug_UpdateHealthValueAtMeshIndex(int32 InMeshIndex) { Debug_UpdateHealthValueAtPoint(GetPointOfMeshIndex(InMeshIndex)); }

	//UPROPERTY(Transient)
	TArraySetPair<FIntVector> QueuedDebugUpdatePoints;
	//~ End Debug
};
