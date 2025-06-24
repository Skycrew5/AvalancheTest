// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "Framework/ScWGameInstance.h"

#include "ATGameInstance.generated.h"

/**
 * 
 */
UCLASS(Abstract, meta = (DisplayName = "[AT] Game Instance"))
class AVALANCHETEST_API UATGameInstance : public UScWGameInstance
{
	GENERATED_BODY()
	
public:

	UATGameInstance();
	
//~ Begin Statics
public:

	UFUNCTION(Category = "Statics", BlueprintCallable, BlueprintPure, meta = (WorldContext = "InWCO"))
	static UATGameInstance* TryGetATGameInstance(const UObject* InWCO);
//~ End Statics
	
//~ Begin Initialize
protected:
	virtual void Init() override; // UGameInstance
	virtual void Shutdown() override; // UGameInstance
//~ End Initialize
	
//~ Begin Voxels
//~ End Voxels
};
