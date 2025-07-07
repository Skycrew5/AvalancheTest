// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "Framework/ScWGameState.h"

#include "World/ATTypes_World.h"

#include "ATGameState.generated.h"

/**
 * 
 */
UCLASS(Abstract, meta = (DisplayName = "[AT] Game State"))
class AVALANCHETEST_API AATGameState : public AScWGameState
{
	GENERATED_BODY()
	
public:

	AATGameState();
	
//~ Begin Statics
public:

	UFUNCTION(Category = "Statics", BlueprintCallable, BlueprintPure, meta = (WorldContext = "InWCO"))
	static AATGameState* TryGetATGameState(const UObject* InWCO);
//~ End Statics
	
//~ Begin Initialize
protected:
	virtual void BeginPlay() override; // AActor
	virtual void Tick(float InDeltaSeconds)  override; // AActor
	virtual void EndPlay(const EEndPlayReason::Type InReason) override; // AActor
//~ End Initialize
	
//~ Begin Voxels
public:

	UFUNCTION(Category = "Voxels", BlueprintCallable)
	class AATVoxelTree* GetMainVoxelTree() const { return MainVoxelTree; }

	UFUNCTION(Category = "Voxels", BlueprintCallable)
	void SetMainVoxelTree(class AATVoxelTree* InVoxelTree);

protected:

	UPROPERTY(Transient)
	TObjectPtr<class AATVoxelTree> MainVoxelTree;
//~ End Voxels
};
