// Avalanche Test

#pragma once

#include "AvalancheTest.h"

#include "ScWTypes_Delegates.h"

#include "ATPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class AVALANCHETEST_API AATPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:

	AATPlayerController();

//~ Begin Initialize
protected:
	virtual void PostInitializeComponents() override; // AActor
	virtual void BeginPlay() override; // AActor
//~ End Initialize

//~ Begin Pawn
public:

	UPROPERTY(Category = "Pawn", BlueprintAssignable)
	FAttributeChangedSignature OnPawnHealthChangedDelegate;

	UPROPERTY(Category = "Pawn", BlueprintAssignable)
	FAttributeChangedSignature OnPawnMaxHealthChangedDelegate;

	UPROPERTY(Category = "Pawn", BlueprintAssignable)
	FDefaultEventSignature OnPawnDiedDelegate;

protected:
	virtual void OnPossess(APawn* InPawn) override; // AController
	virtual void OnUnPossess() override; // AController

	UFUNCTION()
	void BroadcastPawnHealthChanged(const FGameplayAttribute& InAttribute, float InPrevValue, float InNewValue);

	UFUNCTION()
	void BroadcastPawnMaxHealthChanged(const FGameplayAttribute& InAttribute, float InPrevValue, float InNewValue);

	UFUNCTION()
	void BroadcastPawnDied();

	FScriptDelegate OnPawnHealthChangedBind;
	FScriptDelegate OnPawnMaxHealthChangedBind;
	FScriptDelegate OnPawnDiedBind;
//~ End Pawn

//~ Begin Team
public:
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; } // IGenericTeamAgentInterface
	virtual void SetGenericTeamId(const FGenericTeamId& InNewTeamID) override; // IGenericTeamAgentInterface
private:
	FGenericTeamId TeamId;
//~ End Team

//~ Begin Input
public:

	UFUNCTION(Category = "Input", BlueprintCallable, BlueprintCosmetic, meta = (DefaultToSelf = "InSource", KeyWords = "HasMouse, HasCursor"))
	bool HasShowMouseCursorSource(UObject* InSource) const { return ShowMouseCursorSourceSet.Contains(InSource); }

	UFUNCTION(Category = "Input", BlueprintCallable, BlueprintCosmetic, meta = (DefaultToSelf = "InSource", KeyWords = "AddMouse, AddCursor"))
	void AddShowMouseCursorSource(UObject* InSource) { ShowMouseCursorSourceSet.Add(InSource); }
	
	UFUNCTION(Category = "Input", BlueprintCallable, BlueprintCosmetic, meta = (DefaultToSelf = "InSource", KeyWords = "RemoveMouse, RemoveCursor"))
	void RemoveShowMouseCursorSource(UObject* InSource) { ShowMouseCursorSourceSet.Remove(InSource); }

	UFUNCTION(Category = "Input", BlueprintCallable, BlueprintCosmetic, meta = (DefaultToSelf = "InSource", KeyWords = "HasBlock, HasIgnore"))
	bool HasMovementInputBlockSource(UObject* InSource) const { return MovementInputBlockSourceSet.Contains(InSource); }

	UFUNCTION(Category = "Input", BlueprintCallable, BlueprintCosmetic, meta = (DefaultToSelf = "InSource", KeyWords = "AddBlock, AddIgnore"))
	void AddMovementInputBlockSource(UObject* InSource) { MovementInputBlockSourceSet.Add(InSource); }

	UFUNCTION(Category = "Input", BlueprintCallable, BlueprintCosmetic, meta = (DefaultToSelf = "InSource", KeyWords = "RemoveBlock, RemoveIgnore"))
	void RemoveMovementInputBlockSource(UObject* InSource) { MovementInputBlockSourceSet.Remove(InSource); }

	UFUNCTION(Category = "Input", BlueprintCallable, BlueprintCosmetic, meta = (DefaultToSelf = "InSource", KeyWords = "HasBlock, HasIgnore"))
	bool HasLookInputBlockSource(UObject* InSource) const { return LookInputBlockSourceSet.Contains(InSource); }

	UFUNCTION(Category = "Input", BlueprintCallable, BlueprintCosmetic, meta = (DefaultToSelf = "InSource", KeyWords = "AddBlock, AddIgnore"))
	void AddLookInputBlockSource(UObject* InSource) { LookInputBlockSourceSet.Add(InSource); }

	UFUNCTION(Category = "Input", BlueprintCallable, BlueprintCosmetic, meta = (DefaultToSelf = "InSource", KeyWords = "RemoveBlock, RemoveIgnore"))
	void RemoveLookInputBlockSource(UObject* InSource) { LookInputBlockSourceSet.Remove(InSource); }

protected:
	virtual bool ShouldShowMouseCursor() const { return Super::ShouldShowMouseCursor() || !ShowMouseCursorSourceSet.IsEmpty(); } // APlayerController
	virtual bool IsMoveInputIgnored() const { return Super::IsMoveInputIgnored() || !MovementInputBlockSourceSet.IsEmpty(); } // AController
	virtual bool IsLookInputIgnored() const { return Super::IsLookInputIgnored() || !LookInputBlockSourceSet.IsEmpty(); } // AController

	UPROPERTY(Transient)
	TSet<TObjectPtr<UObject>> ShowMouseCursorSourceSet;

	UPROPERTY(Transient)
	TSet<TObjectPtr<UObject>> MovementInputBlockSourceSet;

	UPROPERTY(Transient)
	TSet<TObjectPtr<UObject>> LookInputBlockSourceSet;
//~ End Input
};
