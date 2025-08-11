// Scientific Ways

#pragma once

#include "AvalancheToolsModule.h"

#include "SaveWorldTool.generated.h"

/**
 *	Builder for USaveWorldTool
 */
UCLASS()
class AVALANCHETOOLS_API USaveWorldToolBuilder : public UInteractiveToolBuilder
{
	GENERATED_BODY()

public:

	USaveWorldToolBuilder();

//~ Begin Initialize
public:
	virtual bool CanBuildTool(const FToolBuilderState& InSceneState) const override; // UInteractiveToolBuilder
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& InSceneState) const override; // UInteractiveToolBuilder
//~ End Initialize
};

/**
 *	Settings UObject for USaveWorldTool. This UClass inherits from UInteractiveToolPropertySet,
 *	which provides an OnModified delegate that the Tool will listen to for changes in property values.
 */
UCLASS(Transient)
class AVALANCHETOOLS_API USaveWorldToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()

public:

	USaveWorldToolProperties();

//~ Begin Procedural
public:

	UPROPERTY(Category = "Procedural", EditAnywhere, meta = (DisplayName = "Only Selected VoxelTree"))
	bool bOnlySelectedVoxelTree;

	UPROPERTY(Category = "Procedural", EditAnywhere, meta = (DisplayName = "Async"))
	bool bAsync;
//~ End Procedural
};

/**
 *	USaveWorldTool is an example Tool that drops an empty Actor at each position the user
 *	clicks left mouse button. The Actors are placed at the first ray intersection in the scene,
 *	or on a ground plane if no scene objects are hit. All the action is in the ::OnClicked handler.
 */
UCLASS()
class AVALANCHETOOLS_API USaveWorldTool : public USingleClickTool
{
	GENERATED_BODY()

public:

	static const FString Identifier;

	USaveWorldTool();

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
	TObjectPtr<USaveWorldToolProperties> Properties;

	/** target World we will find VoxelTrees in */
	UPROPERTY(Transient)
	TObjectPtr<UWorld> TargetWorld;
//~ End Regenerate
};
