// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "World/ATTypes_World.h"

#include "ATVoxelISMC.generated.h"

/**
 *
 */
UCLASS(meta = (DisplayName = "[AT] Voxel Instanced Static Mesh Component"))
class AVALANCHETEST_API UATVoxelISMC : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()

public:

	UATVoxelISMC();
	
//~ Begin Initialize
protected:
	virtual void OnRegister() override; // UActorComponent
	virtual void BeginPlay() override; // UActorComponent
	virtual void EndPlay(const EEndPlayReason::Type InReason) override; // UActorComponent
//~ End Initialize
	
//~ Begin Owner
public:

	UFUNCTION(Category = "Owner", BlueprintCallable, meta = (KeyWords = "GetOwnerVoxelChunk"))
	class AATVoxelChunk* GetOwnerChunk() const { return OwnerChunk; }

protected:

	UPROPERTY(Category = "Owner", BlueprintReadOnly)
	TObjectPtr<class AATVoxelChunk> OwnerChunk;
//~ End Owner

//~ Begin Getters
public:

	UFUNCTION(Category = "Getters", BlueprintCallable)
	const TMap<FIntVector, FVoxelInstanceData>& GetLocalPoint_To_InstanceData_Map() const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	void GetAllLocalPoints(TArray<FIntVector>& OutPoints) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	const FIntVector& GetVoxelInstancePointAtIndex(int32 InInstanceIndex, const bool bInChecked = true) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	FVoxelInstanceData& GetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const bool bInChecked = true) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	bool HasVoxelAtPoint(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Getters | Compounds", BlueprintCallable)
	FVoxelCompoundData& GetCompoundDataAt(const FIntVector& InTargetPoint, const bool bInChecked = true) const;

	UFUNCTION(Category = "Getters | Compounds", BlueprintCallable)
	bool HasCompoundAtPoint(const FIntVector& InPoint) const;

	UFUNCTION(Category = "Getters | Compounds", BlueprintCallable)
	void GetAdjacentCompoundOriginsAt(const FIntVector& InTargetPoint, EATAttachmentDirection InSide, TArray<FIntVector>& OutCompoundOrigins) const;
//~ End Getters

//~ Begin Setters
public:

	UFUNCTION(Category = "Setters", BlueprintCallable)
	bool SetVoxelAtPoint(const FIntVector& InPoint, const class UATVoxelTypeData* InTypeData, const bool bInForced = false);

	UFUNCTION(Category = "Setters", BlueprintCallable)
	bool SetVoxelsAtPoints(const TArray<FIntVector>& InPoints, const class UATVoxelTypeData* InTypeData, const bool bInForced = false);

	UFUNCTION(Category = "Setters", BlueprintCallable)
	bool RemoveVoxelAtPoint(const FIntVector& InPoint, const bool bInChecked = true);

	UFUNCTION(Category = "Setters", BlueprintCallable)
	bool RemoveVoxelsAtPoints(const TArray<FIntVector>& InPoints, const bool bInForced = false);

	UFUNCTION(Category = "Setters", BlueprintCallable)
	void RemoveAllVoxels();

	bool RelocateInstanceIndex(int32 InPrevIndex, int32 InNewIndex, const bool bInChecked = true);

	UFUNCTION(Category = "Setters | Compounds", BlueprintCallable)
	void RegenerateCompoundData();

	UFUNCTION(Category = "Setters | Compounds", BlueprintCallable)
	bool SetCompoundData(const FIntVector& InOrigin, int32 InSize, const bool bInChecked = true);

protected:
	int32 CalcMaxFittingCompoundSizeAt(const FIntVector& InOrigin);
//~ End Setters
	
//~ Begin Data
public:

	UFUNCTION(Category = "Data", BlueprintCallable)
	void QueuePointForVisibilityUpdate(const FIntVector& InPoint, const bool bInQueueNeighborsToo = true);

	UFUNCTION(Category = "Data", BlueprintCallable)
	void UpdateVoxelsVisibilityState();

	UPROPERTY(Transient)
	TArray<FIntVector> FoundationLocalPoints;

protected:

	//UPROPERTY(Transient)
	//TArray<FVoxelInstanceData> VoxelDataArray;

	UPROPERTY(Transient)
	TMap<FIntVector, FVoxelInstanceData> LocalPoint_To_InstanceData_Map;

	UPROPERTY(Transient)
	TMap<FIntVector, FVoxelCompoundData> LocalPoint_To_CompoundData_Map;

	UPROPERTY(Transient)
	TMap<int32, FIntVector> InstanceIndex_To_LocalPoint_Map;
	
	TArraySetPair<FIntVector> QueuedVisibilityUpdatePoints;

	static FVoxelInstanceData InvalidInstanceData_NonConst;
	static FVoxelCompoundData InvalidCompoundData_NonConst;
//~ End Data
};
