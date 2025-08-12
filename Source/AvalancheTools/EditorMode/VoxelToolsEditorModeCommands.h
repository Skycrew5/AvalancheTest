// Scientific Ways

#pragma once

#include "AvalancheToolsModule.h"

class FVoxelToolsEditorModeCommands : public TCommands<FVoxelToolsEditorModeCommands>
{
public:

	FVoxelToolsEditorModeCommands();
	
//~ Begin Initialize
public:
	virtual void RegisterCommands() override; // TCommands<>

	TSharedPtr<FUICommandInfo> ManageWorldTool;
//~ End Initialize

//~ Begin Getters
public:
	static TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> GetCommands();
protected:
	TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> Commands;
//~ End Getters
};
