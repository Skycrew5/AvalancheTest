// Scientific Ways

#pragma once

#include "AvalancheToolsModule.h"

#include "RegenerateWorldTool.generated.h"

/**
 *	Builder for URegenerateWorldTool
 */
UCLASS()
class AVALANCHETOOLS_API URegenerateWorldToolBuilder : public UInteractiveToolBuilder
{
	GENERATED_BODY()

public:

	URegenerateWorldToolBuilder();

//~ Begin Initialize
public:
	virtual bool CanBuildTool(const FToolBuilderState& InSceneState) const override; // UInteractiveToolBuilder
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& InSceneState) const override; // UInteractiveToolBuilder
//~ End Initialize
};

/**
 *	Settings UObject for URegenerateWorldTool. This UClass inherits from UInteractiveToolPropertySet,
 *	which provides an OnModified delegate that the Tool will listen to for changes in property values.
 */
UCLASS(Transient)
class AVALANCHETOOLS_API URegenerateWorldToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()

public:

	URegenerateWorldToolProperties();

//~ Begin Procedural
public:

	UPROPERTY(Category = "Procedural", EditAnywhere, meta = (DisplayName = "Only Selected VoxelTree"))
	bool bOnlySelectedVoxelTree;

	UPROPERTY(Category = "Procedural", EditAnywhere, meta = (DisplayName = "Async"))
	bool bAsync;

	UFUNCTION(Category = "Procedural", CallInEditor)
	void GenerateVoxelTreeData();
//~ End Procedural
};

/**
 * URegenerateWorldTool is an example Tool that drops an empty Actor at each position the user 
 * clicks left mouse button. The Actors are placed at the first ray intersection in the scene,
 * or on a ground plane if no scene objects are hit. All the action is in the ::OnClicked handler.
 */
UCLASS()
class AVALANCHETOOLS_API URegenerateWorldTool : public USingleClickTool
{
	GENERATED_BODY()

public:

	static const FString Identifier;

	URegenerateWorldTool();

//~ Begin Initialize
public:
	virtual void SetWorld(UWorld* InWorld);
	virtual void Setup() override; // USingleClickTool
//~ End Initialize

//~ Begin Regenerate
public:
	virtual void HandleRegenerate();
protected:

	UPROPERTY(Transient)
	TObjectPtr<URegenerateWorldToolProperties> Properties;

	/** target World we will find VoxelTrees in */
	UPROPERTY(Transient)
	TObjectPtr<UWorld> TargetWorld;
//~ End Regenerate
};
