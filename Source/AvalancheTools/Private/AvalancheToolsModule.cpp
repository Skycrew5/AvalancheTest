// Scientific Ways

#include "AvalancheToolsModule.h"

#include "GameplayDebugger.h"
#include "IPlacementModeModule.h"
#include "PropertyEditorModule.h"

#include "VoxelChunkDetails.h"
#include "GameplayDebuggerCategory_Voxels.h"

#include "World/ATVoxelChunk.h"

#define LOCTEXT_NAMESPACE "AvalancheToolsModule"

/**
 * Implements the AvalancheTools module.
 */
class FAvalancheToolsModuleImplementation : public FAvalancheToolsModule
{

public:

	//FDelegateHandle OnPostEngineInitDelegateHandle;

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	virtual void StartupModule() override // IModuleInterface
	{
		//OnPostEngineInitDelegateHandle = FCoreDelegates::OnPostEngineInit.AddRaw(this, &FAvalancheToolsModuleImplementation::OnPostEngineInit);

		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

		// ATVoxelChunk
		{
			PropertyModule.RegisterCustomClassLayout("ATVoxelChunk", FOnGetDetailCustomizationInstance::CreateStatic(&FVoxelChunkDetails::MakeInstance));
			
			TSharedRef<FPropertySection> VoxelsSection = PropertyModule.FindOrCreateSection("ATVoxelChunk", "Voxels", LOCTEXT("Voxels", "Voxels"));
			VoxelsSection->AddCategory("Data");
		}
#if WITH_GAMEPLAY_DEBUGGER
		// If the gameplay debugger is available, register the category and notify the editor about the changes
		if (IGameplayDebugger::IsAvailable())
		{
			IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
			GameplayDebuggerModule.RegisterCategory("Voxels", IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategory_Voxels::MakeInstance), EGameplayDebuggerCategoryState::EnabledInGameAndSimulate);
			GameplayDebuggerModule.NotifyCategoriesChanged();
		}
#endif
	}
	// This function may be called during shutdown to clean up your module. For modules that support dynamic reloading,
	// we call this function before unloading the module.
	virtual void ShutdownModule() override // IModuleInterface
	{
		//FCoreDelegates::OnPostEngineInit.Remove(OnPostEngineInitDelegateHandle);

		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner("VoxelsEditor");

		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
		PropertyModule.UnregisterCustomClassLayout("ATVoxelChunk");

		/*if (IPlacementModeModule* PlacementModeModulePtr = FModuleManager::GetModulePtr<IPlacementModeModule>(TEXT("PlacementMode")))
		{
			PlacementModeModulePtr->UnregisterPlacementCategory(FAvalancheToolsPlacementCategories::Voxels());
			PlacementModeModulePtr->OnPlacementModeCategoryRefreshed().Remove(PlacementModeCategoryRefreshHandle);
		}*/
#if WITH_GAMEPLAY_DEBUGGER
		//If the gameplay debugger is available, unregister the category
		if (IGameplayDebugger::IsAvailable())
		{
			IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
			GameplayDebuggerModule.UnregisterCategory("Voxels");
		}
#endif
	}

	/*void OnPostEngineInit()
	{
		IPlacementModeModule& PlacementModeModule = FModuleManager::GetModuleChecked<IPlacementModeModule>(TEXT("PlacementMode"));

		// Voxels placement category
		{
			int32 SortOrder = 0;

			PlacementModeModule.RegisterPlacementCategory
			(
				FPlacementCategoryInfo
				(
					NSLOCTEXT("PlacementMode", "Voxels", "Voxels"),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "PlacementBrowser.Icons.Voxels"),
					FAvalancheToolsPlacementCategories::Voxels(),
					TEXT("PMVoxels"),
					0
				)
			);
			PlacementModeCategoryRefreshHandle = PlacementModeModule.OnPlacementModeCategoryRefreshed().AddRaw(this, &FAvalancheToolsModuleImplementation::RegenerateItemsForCategory);
		}
	}
private:

	void RegenerateItemsForCategory(FName InCategory)
	{
		if (InCategory == FAvalancheToolsPlacementCategories::Voxels())
		{
			RefreshVoxelsCategory();
		}
	}

	void RefreshVoxelsCategory()
	{
		IPlacementModeModule& PlacementModeModule = FModuleManager::GetModuleChecked<IPlacementModeModule>(TEXT("PlacementMode"));

		for (const FPlacementModeID& SampleItemID : VoxelsCategoryItemIDs)
		{
			PlacementModeModule.UnregisterPlaceableItem(SampleItemID);
		}
		VoxelsCategoryItemIDs.Empty();

		// Add loaded classes
		for (TObjectIterator<UClass> ClassIterator; ClassIterator; ++ClassIterator)
		{
			const UClass* SampleClass = *ClassIterator;

			if (!SampleClass->HasAllClassFlags(CLASS_NotPlaceable) &&
				!SampleClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists) &&
				(SampleClass->IsChildOf(AATVoxelChunk::StaticClass())))
			{
				UActorFactory* Factory = GEditor->FindActorFactoryByClassForActorClass(UActorFactory::StaticClass(), SampleClass);

				TOptional<FPlacementModeID> RegisteredItemID = PlacementModeModule.RegisterPlaceableItem
				(
					FAvalancheToolsPlacementCategories::Voxels(),
					MakeShareable(new FPlaceableItem(Factory, FAssetData(SampleClass)))
				);
				if (RegisteredItemID.IsSet())
				{
					VoxelsCategoryItemIDs.Add(RegisteredItemID.GetValue());
				}
			}
		}
	}
	TArray<FPlacementModeID> VoxelsCategoryItemIDs;
	FDelegateHandle PlacementModeCategoryRefreshHandle;*/
};

IMPLEMENT_MODULE(FAvalancheToolsModuleImplementation, AvalancheTools)

#undef LOCTEXT_NAMESPACE
