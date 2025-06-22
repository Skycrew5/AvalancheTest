// Scientific Ways

#include "Framework/ATGameState.h"

AATGameState::AATGameState()
{
	
}

//~ Begin Statics
AATGameState* AATGameState::TryGetATGameState(const UObject* InWCO)
{
	if (!InWCO)
	{
		UE_LOG(LogATGameplay, Error, TEXT("AATGameState::TryGetATGameState() World context is not valid!"));
		return nullptr;
	}

	UWorld* World = InWCO->GetWorld();
	if (!World)
	{
		UE_LOG(LogATGameplay, Error, TEXT("AATGameState::TryGetATGameState() World from context %s is not valid!"), *InWCO->GetName());
		return nullptr;
	}
	if (AATGameState* GameState = World->GetGameState<AATGameState>())
	{
		return GameState;
	}
	UE_LOG(LogATGameplay, Error, TEXT("AATGameState::TryGetATGameState() GameState from World %s is not of class AATGameState!"), *World->GetName());
	return nullptr;
}
//~ End Statics

//~ Begin Voxels
//~ End Voxels
