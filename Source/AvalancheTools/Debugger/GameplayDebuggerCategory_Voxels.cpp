// Scientific Ways

#include "Debugger/GameplayDebuggerCategory_Voxels.h"

#if WITH_GAMEPLAY_DEBUGGER

#include "Framework/ScWPlayerController.h"

#include "World/ATVoxelTree.h"
#include "World/ATVoxelChunk.h"

#include "DrawDebugHelpers.h"

FGameplayDebuggerCategory_Voxels::FGameplayDebuggerCategory_Voxels()
{
	CollectDataInterval = 0.1f;

	bShowOnlyWithDebugActor = true;

	bUnderCursorTarget = false;
	DebugData = FVoxelChunkDebugData();
}

//~ Begin Initialize
TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_Voxels::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategory_Voxels());
}

void FGameplayDebuggerCategory_Voxels::OnGameplayDebuggerActivated() // FGameplayDebuggerAddonBase
{
	FGameplayDebuggerCategory::OnGameplayDebuggerActivated();

	HandleToggled(true);
}

void FGameplayDebuggerCategory_Voxels::OnGameplayDebuggerDeactivated() // FGameplayDebuggerAddonBase
{
	FGameplayDebuggerCategory::OnGameplayDebuggerDeactivated();

	HandleToggled(false);
}

void FGameplayDebuggerCategory_Voxels::HandleToggled(const bool bInWasActivated)
{
	TArray<AActor*> TreeActors;
	UGameplayStatics::GetAllActorsOfClass(CachedWorld ? CachedWorld : GWorld, AATVoxelTree::StaticClass(), TreeActors);

	for (AActor* SampleTreeActor : TreeActors)
	{
		if (AATVoxelTree* SampleTree = Cast<AATVoxelTree>(SampleTreeActor))
		{
			SampleTree->BP_HandleGameplayDebuggerToggled(bInWasActivated);
		}
	}
}
//~ End Initialize

void FGameplayDebuggerCategory_Voxels::CollectData(APlayerController* InPlayerController, AActor* InDebugActor) // FGameplayDebuggerCategory
{
	CachedWorld = InPlayerController->GetWorld();
	if (CachedWorld == nullptr)
	{
		return;
	}
	AATVoxelChunk* DebugChunk = Cast<AATVoxelChunk>(InDebugActor);
	bUnderCursorTarget = false;

	if (DebugChunk == nullptr)
	{
		AScWPlayerController* ScWPlayerController = Cast<AScWPlayerController>(InPlayerController);

		FHitResult ScreenCenterHitResult;
		ScWPlayerController->GetHitResultUnderScreenCenter(TraceTypeQuery_Visibility, false, ScreenCenterHitResult);

		DebugChunk = Cast<AATVoxelChunk>(ScreenCenterHitResult.GetActor());
		bUnderCursorTarget = true;
	}
	if (DebugChunk == nullptr)
	{
		return;
	}
	DebugData.Reset();
	DebugChunk->BP_CollectDataForGameplayDebugger(InPlayerController, DebugData);
}

void FGameplayDebuggerCategory_Voxels::DrawData(APlayerController* InPlayerController, FGameplayDebuggerCanvasContext& InOutCanvasContext) // FGameplayDebuggerCategory
{
	CachedWorld = InPlayerController->GetWorld();
	if (CachedWorld == nullptr)
	{
		return;
	}
	DrawDebugBox(CachedWorld, DebugData.ChunkHighlightTransform.GetLocation(), DebugData.ChunkHighlightTransform.GetScale3D(), FColor::White, false, -1.0f, 100U, 4.0f);

	static FString SeparatorString = "=====================================";

	if (bUnderCursorTarget)
	{
		InOutCanvasContext.Print(DebugData.LabelColor, FString::Printf(TEXT("Looking at: %s"), *DebugData.Label));
	}
	else
	{
		InOutCanvasContext.Print(DebugData.LabelColor, FString::Printf(TEXT("Selected Target: %s"), *DebugData.Label));
	}
	// Common
	InOutCanvasContext.Print(FColor::White, 0.5f, SeparatorString);
	InOutCanvasContext.Print(FColor::White, 1.0f, TEXT("Commons: "));
	for (const FVoxelChunkDebugData_Entry& SampleEntry : DebugData.CommonEntries)
	{
		InOutCanvasContext.Print(FColor::White, SampleEntry.ToString());
	}

	// Attachments
	InOutCanvasContext.Print(FColor::Orange, 0.5f, SeparatorString);
	InOutCanvasContext.Print(FColor::Orange, 1.0f, TEXT("Attachments: "));
	for (const FVoxelChunkDebugData_Entry& SampleEntry : DebugData.AttachmentsEntries)
	{
		InOutCanvasContext.Print(FColor::Orange, SampleEntry.ToString());
	}

	// AvalancheValue
	InOutCanvasContext.Print(FColor::Green, 0.5f, SeparatorString);
	InOutCanvasContext.Print(FColor::Green, 1.0f, TEXT("AvalancheValue: "));
	for (const FVoxelChunkDebugData_Entry& SampleEntry : DebugData.StabilityEntries)
	{
		InOutCanvasContext.Print(FColor::Green, SampleEntry.ToString());
	}

	// Instance
	InOutCanvasContext.Print(FColor::Purple, 0.5f, SeparatorString);
	InOutCanvasContext.Print(FColor::Purple, 1.0f, DebugData.InstanceLabel);
	for (const FVoxelChunkDebugData_Entry& SampleEntry : DebugData.InstanceEntries)
	{
		InOutCanvasContext.Print(FColor::Purple, SampleEntry.ToString());
	}
	DrawDebugBox(CachedWorld, DebugData.InstanceHighlightTransform.GetLocation(), DebugData.InstanceHighlightTransform.GetScale3D(), FColor::Purple, false, -1.0f, 120U, 2.0f);
}

#endif // WITH_GAMEPLAY_DEBUGGER
