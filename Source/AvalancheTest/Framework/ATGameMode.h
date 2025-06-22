// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "Framework/ScWGameMode.h"

#include "ATGameMode.generated.h"

/**
 * 
 */
UCLASS(Abstract, meta = (DisplayName = "[AT] Game Mode"))
class AVALANCHETEST_API AATGameMode : public AScWGameMode
{
	GENERATED_BODY()
	
public:

	AATGameMode();
};
