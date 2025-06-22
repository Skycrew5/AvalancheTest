// Scientific Ways

#pragma once

#include "CoreMinimal.h"

#include "IDetailGroup.h"
#include "IDetailsView.h"
#include "Engine/World.h"
#include "Tools/UEdMode.h"
#include "PropertyHandle.h"
#include "EditorStyleSet.h"
#include "DetailWidgetRow.h"
#include "ToolBuilderUtil.h"
#include "SceneManagement.h"
#include "ToolBuilderUtil.h"
#include "Engine/Selection.h"
#include "EditorModeManager.h"
#include "ObjectEditorUtils.h"
#include "EditorCategoryUtils.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/SUserWidget.h"
#include "PropertyEditorModule.h"
#include "CollisionQueryParams.h"
#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "UObject/NoExportTypes.h"
#include "Modules/ModuleManager.h"
#include "InteractiveToolBuilder.h"
#include "InteractiveToolManager.h"
#include "InteractiveToolManager.h"
#include "BaseTools/ClickDragTool.h"
#include "GameplayDebuggerCategory.h"
#include "BaseTools/SingleClickTool.h"
#include "Framework/Commands/Commands.h"
#include "BaseBehaviors/ClickDragBehavior.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

#include "Modules/ModuleManager.h"

/** Structure of built-in placement categories. Defined as functions to enable external use without linkage */
struct FAvalancheToolsPlacementCategories
{
	static FName Voxels() { static FName Name("Voxels"); return Name; }
};

/**
 * This is the module definition for the editor mode. You can implement custom functionality
 * as your plugin module starts up and shuts down. See IModuleInterface for more extensibility options.
 */
class FAvalancheToolsModule : public IModuleInterface
{

public:

	virtual void StartupModule() override = 0; // IModuleInterface
	virtual void ShutdownModule() override = 0; // IModuleInterface
};
