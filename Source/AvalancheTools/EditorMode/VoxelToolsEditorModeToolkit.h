// Scientific Ways

#pragma once

#include "AvalancheToolsModule.h"

class IDetailsView;

/**
 * This FModeToolkit just creates a basic UI panel that allows various InteractiveTools to
 * be initialized, and a DetailsView used to show properties of the active Tool.
 */
class FVoxelToolsEditorModeToolkit : public FModeToolkit
{
public:

	FVoxelToolsEditorModeToolkit();
	
//~ Begin Initialize
public:
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode) override; // FModeToolkit
//~ End Initialize

//~ Begin Info
public:
	virtual void GetToolPaletteNames(TArray<FName>& PaletteNames) const override; // FModeToolkit
	virtual FName GetToolkitFName() const override; // IToolkit
	virtual FText GetBaseToolkitName() const override; // IToolkit
//~ End Info
};
