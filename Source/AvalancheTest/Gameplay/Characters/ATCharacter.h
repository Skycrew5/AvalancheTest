// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "Gameplay/Characters/ScWCharacter.h"

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
	
//~ Begin Controller
protected:
	virtual void PossessedBy(AController* InController) override; // APawn
	virtual void UnPossessed() override; // APawn
//~ End Controller
	
};
