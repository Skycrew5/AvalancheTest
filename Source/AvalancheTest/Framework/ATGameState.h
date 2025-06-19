// Avalanche Test

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

//~ Begin Voxels
//~ End Voxels
};
