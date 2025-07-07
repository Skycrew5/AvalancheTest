// Scientific Ways

#include "Gameplay/Characters/ATCharacter.h"

#include "Framework/ATGameState.h"

#include "World/ATVoxelTree.h"

AATCharacter::AATCharacter(const FObjectInitializer& InObjectInitializer)
	: Super(InObjectInitializer)
{
	
}

//~ Begin Controller
void AATCharacter::PossessedBy(AController* InController) // APawn
{
	Super::PossessedBy(InController);

	if (InController && InController->IsPlayerController())
	{
		if (AATGameState* GameState = AATGameState::TryGetATGameState(this))
		{
			if (AATVoxelTree* MainVoxelTree = GameState->GetMainVoxelTree())
			{
				MainVoxelTree->RegisterChunksUpdateReferenceActor(this);
			}
		}
	}
}

void AATCharacter::UnPossessed() // APawn
{
	if (Controller && Controller->IsPlayerController())
	{
		if (AATGameState* GameState = AATGameState::TryGetATGameState(this))
		{
			if (AATVoxelTree* MainVoxelTree = GameState->GetMainVoxelTree())
			{
				MainVoxelTree->UnRegisterChunksUpdateReferenceActor(this);
			}
		}
	}
	Super::UnPossessed();
}
//~ End Controller
