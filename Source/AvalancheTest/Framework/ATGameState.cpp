// Scientific Ways

#include "Framework/ATGameState.h"

#include "Gameplay/Characters/ATCharacter.h"

#include "World/ATVoxelTree.h"

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

//~ Begin Initialize
void AATGameState::BeginPlay() // AActor
{
	Super::BeginPlay();
}

void AATGameState::Tick(float InDeltaSeconds) // AActor
{
	Super::Tick(InDeltaSeconds);


}

void AATGameState::EndPlay(const EEndPlayReason::Type InReason) // AActor
{
	Super::EndPlay(InReason);
}
//~ End Initialize

//~ Begin Voxels
void AATGameState::SetMainVoxelTree(AATVoxelTree* InVoxelTree)
{
	ensureReturn(!MainVoxelTree);
	MainVoxelTree = InVoxelTree;

	for (const APlayerState* SamplePlayer : PlayerArray)
	{
		if (AATCharacter* SampleCharacter = SamplePlayer->GetPawn<AATCharacter>()) // Consider only AATCharacter pawns
		{
			MainVoxelTree->RegisterChunksUpdateReferenceActor(SampleCharacter);
		}
	}
}
//~ End Voxels

