// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "Characters/ScWCharacter.h"

#include "ATCharacter.generated.h"

/**
 *
 */
UCLASS(Abstract, Blueprintable)
class AATCharacter : public AScWCharacter
{
	GENERATED_BODY()

public:

	AATCharacter(const FObjectInitializer& InObjectInitializer = FObjectInitializer::Get());

};
