#pragma once

#include "AvalancheToolsModule.h"

#if WITH_GAMEPLAY_DEBUGGER

#include "World/ATTypes_World.h"

class FGameplayDebuggerCategory_Voxels : public FGameplayDebuggerCategory
{
public:

	FGameplayDebuggerCategory_Voxels();

	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

//~ Begin Debug
protected:
	virtual void CollectData(APlayerController* InPlayerController, AActor* InDebugActor) override; // FGameplayDebuggerCategory
	virtual void DrawData(APlayerController* InPlayerController, class FGameplayDebuggerCanvasContext& InOutCanvasContext) override; // FGameplayDebuggerCategory
//~ End Debug

//~ Begin Data
protected:
	bool bUnderCursorTarget;
	FVoxelChunkDebugData DebugData;
//~ End Data
};

#endif
