// Avalanche Test

#include "Framework/ATPlayerController.h"

#include "Gameplay/ScWASC_Base.h"

AATPlayerController::AATPlayerController()
{
	TeamId = FGenericTeamId::NoTeam;
}

//~ Begin Initialize
void AATPlayerController::PostInitializeComponents() // AActor
{
	Super::PostInitializeComponents();

	OnPawnHealthChangedBind.BindUFunction(this, TEXT("BroadcastPawnHealthChanged"));
	OnPawnMaxHealthChangedBind.BindUFunction(this, TEXT("BroadcastPawnMaxHealthChanged"));
	OnPawnDiedBind.BindUFunction(this, TEXT("BroadcastPawnDied"));
}

void AATPlayerController::BeginPlay() // AActor
{
	Super::BeginPlay();

	
}
//~ End Initialize

//~ Begin Pawn
void AATPlayerController::OnPossess(APawn* InPawn) // AController
{
	Super::OnPossess(InPawn);

	if (UScWASC_Base* PawnASC = UScWASC_Base::TryGetBaseAtaASCFromActor(InPawn))
	{
		PawnASC->OnHealthChangedDelegate.Add(OnPawnHealthChangedBind);
		PawnASC->OnMaxHealthChangedDelegate.Add(OnPawnMaxHealthChangedBind);
		PawnASC->OnDiedDelegate.Add(OnPawnDiedBind);
	}
}

void AATPlayerController::OnUnPossess() // AController
{
	Super::OnUnPossess();

	if (UScWASC_Base* PawnASC = UScWASC_Base::TryGetBaseAtaASCFromActor(GetPawn()))
	{
		PawnASC->OnHealthChangedDelegate.Remove(OnPawnHealthChangedBind);
		PawnASC->OnMaxHealthChangedDelegate.Remove(OnPawnMaxHealthChangedBind);
		PawnASC->OnDiedDelegate.Remove(OnPawnDiedBind);
	}
}

void AATPlayerController::BroadcastPawnHealthChanged(const FGameplayAttribute& InAttribute, float InPrevValue, float InNewValue)
{
	OnPawnHealthChangedDelegate.Broadcast(InAttribute, InPrevValue, InNewValue);
}

void AATPlayerController::BroadcastPawnMaxHealthChanged(const FGameplayAttribute& InAttribute, float InPrevValue, float InNewValue)
{
	OnPawnMaxHealthChangedDelegate.Broadcast(InAttribute, InPrevValue, InNewValue);
}

void AATPlayerController::BroadcastPawnDied()
{
	OnPawnDiedDelegate.Broadcast();
}
//~ End Pawn

//~ Begin Team
void AATPlayerController::SetGenericTeamId(const FGenericTeamId& InNewTeamId) // IGenericTeamAgentInterface
{
	TeamId = InNewTeamId;
}
//~ End Team

//~ Begin Input
//~ End Input
