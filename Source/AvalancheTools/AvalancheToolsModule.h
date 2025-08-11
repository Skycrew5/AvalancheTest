// Scientific Ways

#pragma once

#include "CoreMinimal.h"

#include "Engine/World.h"
#include "Engine/Selection.h"
#include "UObject/NoExportTypes.h"

#include "Tools/UEdMode.h"
#include "EditorModeManager.h"

#include "CollisionQueryParams.h"
#include "PrimitiveDrawingUtils.h"
#include "Framework/Commands/Commands.h"
#include "BaseBehaviors/BehaviorTargetInterfaces.h"

#include "ToolBuilderUtil.h"
#include "ToolContextInterfaces.h"
#include "InteractiveToolBuilder.h"
#include "InteractiveToolManager.h"

#include "PropertyHandle.h"
#include "PropertyEditorModule.h"

#include "EditorStyleSet.h"
#include "SceneManagement.h"
#include "ObjectEditorUtils.h"
#include "EditorCategoryUtils.h"

#include "Styling/AppStyle.h"
#include "Widgets/SUserWidget.h"
#include "BaseTools/ClickDragTool.h"
#include "BaseTools/SingleClickTool.h"
#include "BaseBehaviors/ClickDragBehavior.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

#include "IDetailGroup.h"
#include "IDetailsView.h"
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"

#include "GameplayDebuggerCategory.h"

#include "Modules/ModuleManager.h"

#include "UnrealCommons.h"

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
