// Scientific Ways

#pragma once

#include "AvalancheToolsModule.h"

#include "VoxelToolsEditorMode.generated.h"

/**
 * This class provides an example of how to extend a UEdMode to add some simple tools
 * using the InteractiveTools framework. The various UEdMode input event handlers (see UEdMode.h)
 * forward events to a UEdModeInteractiveToolsContext instance, which 
 * has all the logic for interacting with the InputRouter, ToolManager, etc.
 * The functions provided here are the minimum to get started inserting some custom behavior.
 * Take a look at the UEdMode markup for more extensibility options.
 */
UCLASS()
class UVoxelToolsEditorMode : public UEdMode
{
	GENERATED_BODY()

public:

	const static FEditorModeID EM_VoxelToolsEditorModeId;

	UVoxelToolsEditorMode();
	virtual ~UVoxelToolsEditorMode();
	
//~ Begin Initialize
protected:
	virtual void Enter() override; // UEdMode
	virtual void CreateToolkit() override; // UEdMode
	virtual void ActorSelectionChangeNotify() override; // UEdMode
//~ End Initialize

//~ Begin Getters
public:
	virtual TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> GetModeCommands() const override; // UEdMode
//~ End Getters
};
