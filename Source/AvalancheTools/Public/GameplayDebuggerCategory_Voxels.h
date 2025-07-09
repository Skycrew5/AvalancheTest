#pragma once

#include "AvalancheToolsModule.h"

#if WITH_GAMEPLAY_DEBUGGER

#include "World/ATTypes_World.h"

class FGameplayDebuggerCategory_Voxels : public FGameplayDebuggerCategory
{
public:

	FGameplayDebuggerCategory_Voxels();

//~ Begin Initialize
public:
	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();
protected:
	virtual void OnGameplayDebuggerActivated() override; // FGameplayDebuggerAddonBase
	virtual void OnGameplayDebuggerDeactivated() override; // FGameplayDebuggerAddonBase
	void HandleToggled(const bool bInWasActivated);
//~ End Initialize

//~ Begin Debug
protected:
	virtual void CollectData(APlayerController* InPlayerController, AActor* InDebugActor) override; // FGameplayDebuggerCategory
	virtual void DrawData(APlayerController* InPlayerController, class FGameplayDebuggerCanvasContext& InOutCanvasContext) override; // FGameplayDebuggerCategory
//~ End Debug

//~ Begin Data
protected:

	bool bUnderCursorTarget;
	FVoxelChunkDebugData DebugData;

	TObjectPtr<UWorld> CachedWorld;
//~ End Data
};

#endif
