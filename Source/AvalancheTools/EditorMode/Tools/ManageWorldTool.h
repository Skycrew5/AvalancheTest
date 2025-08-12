// Scientific Ways

#pragma once

#include "AvalancheToolsModule.h"

#include "ManageWorldTool.generated.h"

/**
 *	Builder for UManageWorldTool
 */
UCLASS()
class AVALANCHETOOLS_API UManageWorldToolBuilder : public UInteractiveToolBuilder
{
	GENERATED_BODY()

public:

	UManageWorldToolBuilder();

//~ Begin Initialize
public:
	virtual bool CanBuildTool(const FToolBuilderState& InSceneState) const override; // UInteractiveToolBuilder
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& InSceneState) const override; // UInteractiveToolBuilder
//~ End Initialize
};

/**
 *	Settings UObject for UManageWorldTool. This UClass inherits from UInteractiveToolPropertySet,
 *	which provides an OnModified delegate that the Tool will listen to for changes in property values.
 */
UCLASS(Transient)
class AVALANCHETOOLS_API UManageWorldToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()

public:

	UManageWorldToolProperties();

//~ Begin Generate
public:

	UPROPERTY(Category = "Generate", EditAnywhere, meta = (DisplayName = "Only Selected VoxelTree"))
	bool bOnlySelectedVoxelTree;

	UPROPERTY(Category = "Generate", EditAnywhere, meta = (DisplayName = "Async"))
	bool bAsync;

	UPROPERTY(Category = "Generate", EditAnywhere, meta = (DisplayName = "Tree Seed"))
	int32 TreeSeed;

	UFUNCTION(Category = "Generate", CallInEditor)
	void GenerateVoxelTreeData();
//~ End Generate
	
//~ Begin Save / Load
public:

	UPROPERTY(Category = "Save / Load", EditAnywhere, meta = (DisplayName = "Save Slot"))
	FString SaveSlot;

	UFUNCTION(Category = "Save / Load", CallInEditor)
	void SaveVoxelTreeData();

	UFUNCTION(Category = "Save / Load", CallInEditor)
	void LoadVoxelTreeData();
//~ End Save / Load
};

/**
 * UManageWorldTool is an example Tool that drops an empty Actor at each position the user 
 * clicks left mouse button. The Actors are placed at the first ray intersection in the scene,
 * or on a ground plane if no scene objects are hit. All the action is in the ::OnClicked handler.
 */
UCLASS()
class AVALANCHETOOLS_API UManageWorldTool : public UInteractiveTool
{
	GENERATED_BODY()

public:

	static const FString Identifier;

	UManageWorldTool();

//~ Begin Initialize
public:
	virtual void SetWorld(UWorld* InWorld);
	virtual void Setup() override; // UInteractiveTool
//~ End Initialize

//~ Begin Generate
public:
	virtual void HandleGenerate();
//~ End Generate
	
//~ Begin Save / Load
public:
	virtual void HandleSave();
	virtual void HandleLoad();
//~ End Save / Load

//~ Begin Properties
protected:

	UPROPERTY(Transient)
	TObjectPtr<UManageWorldToolProperties> Properties;

	/** target World we will find VoxelTrees in */
	UPROPERTY(Transient)
	TObjectPtr<UWorld> TargetWorld;
//~ End Properties
};
