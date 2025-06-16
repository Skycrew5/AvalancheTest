// Avalanche Test

#pragma once

#include "AvalancheTest.h"

#include "Types_World.h"

#include "ATVoxelChunk.generated.h"

/**
 * 
 */
UCLASS(Abstract)
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

//~ Begin Getters
public:

	UFUNCTION(Category = "Getters", BlueprintCallable)
	int32 GetVoxelGlobalIndexAt(const FIntVector& InLocalXYZ) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	const FVoxelData& GetVoxelDataAt(const FIntVector& InLocalXYZ) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	bool HasVoxelAt(const FIntVector& InLocalXYZ) const;

	UFUNCTION(Category = "Getters", BlueprintCallable)
	int32 GetVoxelNeighboursNumAt(const FIntVector& InLocalXYZ) const;
//~ End Getters
	
//~ Begin Setters
public:

	//UFUNCTION(Category = "Setters", BlueprintCallable)
	//void FillWithVoxels();

	UFUNCTION(Category = "Setters", BlueprintCallable)
	void BreakVoxelAt(const FIntVector& InLocalXYZ);

	UFUNCTION(Category = "Setters", BlueprintCallable)
	void BreakVoxelsAt(const TArray<FIntVector>& InLocalXYZ);
//~ End Setters

//~ Begin Data
public:

	UPROPERTY(Category = "Data", BlueprintReadWrite)
	FIntVector GlobalOffsetXYZ;
//~ End Data

//~ Begin Cache
public:
	void UpdateCache();
	void ResetCache();
protected:

	UPROPERTY(Category = "Cache", BlueprintReadOnly)
	TObjectPtr<class AATGameState> Cache_GameState;
//~ End Cache
};
