// Scientific Ways

#include "World/ATVoxelChunk.h"

#include "Framework/ScWPlayerController.h"

#include "World/ATVoxelISMC.h"
#include "World/ATVoxelTree.h"
#include "World/ATVoxelTypeData.h"
#include "World/ATWorldFunctionLibrary.h"

#include "FastNoise2/FastNoise2Generators.h"

#if DEBUG_VOXELS
	#pragma optimize("", off)
#endif // DEBUG_VOXELS

AATVoxelChunk::AATVoxelChunk(const FObjectInitializer& InObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	VoxelComponentClass = UATVoxelISMC::StaticClass();
}

//~ Begin Initialize
void AATVoxelChunk::PostInitializeComponents() // AActor
{
	Super::PostInitializeComponents();


}

void AATVoxelChunk::OnConstruction(const FTransform& InTransform) // AActor
{
	Super::OnConstruction(InTransform);

	UWorld* World = GetWorld();
	ensureReturn(World);
	
	if (World->IsEditorWorld())
	{
		for (const auto& SampleTypeDataAndVoxelComponent : PerTypeVoxelComponentMap)
		{
			ensureContinue(SampleTypeDataAndVoxelComponent.Value);
			SampleTypeDataAndVoxelComponent.Value->ForceUpdateAllVoxelsVisibilityState();
		}
	}
}

void AATVoxelChunk::BeginPlay() // AActor
{
	Super::BeginPlay();

	
}

void AATVoxelChunk::EndPlay(const EEndPlayReason::Type InReason) // AActor
{
	

	Super::EndPlay(InReason);
}

void AATVoxelChunk::BP_InitChunk_Implementation(AATVoxelTree* InOwnerTree, const FIntVector& InChunkCoords)
{
	UWorld* World = GetWorld();
	ensureReturn(World);

	OwnerTree = InOwnerTree;
	ChunkCoords = InChunkCoords;

	/*if (World->IsEditorWorld())
	{

	}
	else if (World->IsGameWorld())
	{

	}*/
}
//~ End Initialize

//~ Begin Voxel Tree
int32 AATVoxelChunk::GetChunkSize() const
{
	UWorld* World = GetWorld();
	ensureReturn(World, 16);

	if (World->IsEditorWorld() && !OwnerTree)
	{
		return 16;
	}
	ensureReturn(OwnerTree, 16);
	return OwnerTree->GetChunkSize();
}

float AATVoxelChunk::GetVoxelSize() const
{
	UWorld* World = GetWorld();
	ensureReturn(World, 16.0f);

	if (World->IsEditorWorld() && !OwnerTree)
	{
		return 16.0f;
	}
	ensureReturn(OwnerTree, 16.0f);
	return OwnerTree->GetVoxelSize();
}

int32 AATVoxelChunk::GetChunkSeed() const
{
	ensureReturn(OwnerTree, 0);
	return OwnerTree->GetTreeSeed() + ChunkCoords.X * ChunkCoords.X * ChunkCoords.X + ChunkCoords.Y * ChunkCoords.Y + ChunkCoords.Z;
}

bool AATVoxelChunk::IsChunkOnTreeSide(const bool bInIgnoreBottom) const
{
	ensureReturn(OwnerTree, false);
	return OwnerTree->IsChunkCoordsOnTreeSide(ChunkCoords, bInIgnoreBottom);
}
//~ End Voxel Tree

//~ Begin Voxel Components
UATVoxelISMC* AATVoxelChunk::GetVoxelComponentAtPoint(const FIntVector& InPoint) const
{
	ensureReturn(OwnerTree, nullptr);

	const FVoxelInstanceData& SampleVoxelInstanceData = OwnerTree->GetVoxelInstanceDataAtPoint(InPoint, false);
	if (SampleVoxelInstanceData.IsTypeDataValid())
	{
		ensureReturn(PerTypeVoxelComponentMap.Contains(SampleVoxelInstanceData.TypeData), nullptr);
		return PerTypeVoxelComponentMap[SampleVoxelInstanceData.TypeData];
	}
	else
	{
		for (const auto& SampleTypeDataAndVoxelComponent : PerTypeVoxelComponentMap)
		{
			ensureContinue(SampleTypeDataAndVoxelComponent.Value);
			if (SampleTypeDataAndVoxelComponent.Value->CouldCreateMeshAtPoint(InPoint))
			{
				return SampleTypeDataAndVoxelComponent.Value;
			}
		}
	}
	return nullptr;
}

UATVoxelISMC* AATVoxelChunk::GetOrInitVoxelComponentForType(const UATVoxelTypeData* InTypeData)
{
	ensureReturn(InTypeData, nullptr);

	if (PerTypeVoxelComponentMap.Contains(InTypeData))
	{
		return PerTypeVoxelComponentMap[InTypeData];
	}
	UATVoxelISMC* NewVoxelComponent = NewObject<UATVoxelISMC>(this, VoxelComponentClass, FName(TEXT("VoxelComponent_") + InTypeData->GetName()));
	ensureReturn(NewVoxelComponent, nullptr);

	NewVoxelComponent->BP_InitComponent(this, InTypeData);
	NewVoxelComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	NewVoxelComponent->RegisterComponent();
	NewVoxelComponent->bEnableVoxelsVisibilityUpdates = bReadyToUpdateVoxelsVisibility;

	PerTypeVoxelComponentMap.Add(InTypeData, NewVoxelComponent);
	return PerTypeVoxelComponentMap[InTypeData];
}
//~ End Voxel Components

//~ Begin Voxel Getters
FVector AATVoxelChunk::GetChunkCenterWorldLocation() const
{
	return GetActorLocation() + FVector((float)GetChunkSize() * GetVoxelSize() * 0.5f);
}

bool AATVoxelChunk::HasVoxelInstanceDataAtPoint(const FIntVector& InPoint, const bool bInIgnoreQueued) const
{
	ensureReturn(OwnerTree, false);
	return OwnerTree->HasVoxelInstanceDataAtPoint(InPoint, bInIgnoreQueued);
}

FVoxelInstanceData& AATVoxelChunk::GetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const bool bInChecked, const bool bInIgnoreQueued) const
{
	ensureReturn(OwnerTree, const_cast<FVoxelInstanceData&>(FVoxelInstanceData::Invalid));
	return OwnerTree->GetVoxelInstanceDataAtPoint(InPoint, bInChecked, bInIgnoreQueued);
}

bool AATVoxelChunk::IsPointInsideTree(const FIntVector& InPoint) const
{
	ensureReturn(OwnerTree, false);
	return OwnerTree->IsPointInsideTree(InPoint);
}
//~ End Voxel Getters

//~ Begin Voxel Setters
void AATVoxelChunk::HandleSetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const FVoxelInstanceData& InVoxelInstanceData)
{
	if (InVoxelInstanceData.IsTypeDataValid())
	{
		if (UATVoxelISMC* TargetComponent = GetOrInitVoxelComponentForType(InVoxelInstanceData.TypeData))
		{
			TargetComponent->HandleSetVoxelInstanceDataAtPoint(InPoint, InVoxelInstanceData);
		}
	}
	else
	{
		if (UATVoxelISMC* TargetComponent = GetVoxelComponentAtPoint(InPoint))
		{
			TargetComponent->HandleSetVoxelInstanceDataAtPoint(InPoint, InVoxelInstanceData);
		}
	}
}

void AATVoxelChunk::HandleBreakVoxelAtPoint(const FIntVector& InPoint, const FVoxelBreakData& InBreakData)
{
	if (UATVoxelISMC* TargetComponent = GetVoxelComponentAtPoint(InPoint))
	{
		TargetComponent->HandleBreakVoxelAtPoint(InPoint, InBreakData);
	}
}

/*void AATVoxelChunk::HandleSetVoxelStabilityAtPoint(const FIntVector& InPoint, float InNewStability)
{
	if (bDebugInstancesStabilityValues)
	{
		if (UATVoxelISMC* TargetComponent = GetVoxelComponentAtPoint(InPoint))
		{
			TargetComponent->Debug_UpdateStabilityValueAtPoint(InPoint, InNewStability);
		}
	}
}

void AATVoxelChunk::HandleSetVoxelHealthAtPoint(const FIntVector& InPoint, float InNewHealth)
{
	if (bDebugInstancesHealthValues)
	{
		if (UATVoxelISMC* TargetComponent = GetVoxelComponentAtPoint(InPoint))
		{
			TargetComponent->Debug_UpdateHealthValueAtPoint(InPoint, InNewHealth);
		}
	}
}*/
//~ End Voxel Setters

//~ Begin Voxel Data
bool AATVoxelChunk::IsThisTickUpdatesTimeBudgetExceeded() const
{
	ensureReturn(OwnerTree, false);
	return OwnerTree->IsThisTickUpdatesTimeBudgetExceeded();
}

void AATVoxelChunk::MarkChunkAsSimulationReady()
{
	ensure(!bChunkSimulationReady);
	bChunkSimulationReady = true;
	OnBecomeSimulationReady.Broadcast();

	UpdateReadyToUpdateVoxelsVisibilityState();

	for (const FIntVector& SampleOffset : FATVoxelUtils::SideOffsets)
	{
		if (AATVoxelChunk* SampleNeighborChunk = OwnerTree->GetVoxelChunkAtCoords(ChunkCoords + SampleOffset))
		{
			SampleNeighborChunk->UpdateReadyToUpdateVoxelsVisibilityState();
		}
	}
}

void AATVoxelChunk::UpdateReadyToUpdateVoxelsVisibilityState()
{
	if (bReadyToUpdateVoxelsVisibility)
	{
		return;
	}
	ensureReturn(OwnerTree);
	for (const FIntVector& SampleOffset : FATVoxelUtils::SideOffsets)
	{
		const FIntVector& SampleNeighborChunkCoords = ChunkCoords + SampleOffset;

		if (OwnerTree->IsChunkCoordsInsideTree(SampleNeighborChunkCoords))
		{
			if (AATVoxelChunk* SampleNeighborChunk = OwnerTree->GetVoxelChunkAtCoords(SampleNeighborChunkCoords))
			{
				if (SampleNeighborChunk->IsChunkSimulationReady())
				{
					// Good
				}
				else
				{
					return;
				}
			}
			else
			{
				return;
			}
		}
		else
		{
			// Good
		}
	}
	bReadyToUpdateVoxelsVisibility = true;
	OnBecomeReadyToUpdateVoxelsVisibility.Broadcast();

	for (const auto& SampleTypeDataAndVoxelComponent : PerTypeVoxelComponentMap)
	{
		ensureContinue(SampleTypeDataAndVoxelComponent.Value);
		SampleTypeDataAndVoxelComponent.Value->bEnableVoxelsVisibilityUpdates = bReadyToUpdateVoxelsVisibility;
	}
}

void AATVoxelChunk::HandleUpdates()
{
	for (const auto& SampleTypeDataAndVoxelComponent : PerTypeVoxelComponentMap)
	{
		ensureContinue(SampleTypeDataAndVoxelComponent.Value);
		SampleTypeDataAndVoxelComponent.Value->HandleUpdates();

		if (IsThisTickUpdatesTimeBudgetExceeded())
		{
			break;
		}
	}
}
//~ End Voxel Data

//~ Begin Debug
void AATVoxelChunk::BP_CollectDataForGameplayDebugger_Implementation(APlayerController* ForPlayerController, FVoxelChunkDebugData& InOutData) const
{
	ensureReturn(OwnerTree);

	// Common
	InOutData.ChunkHighlightTransform = FTransform(FRotator::ZeroRotator, GetChunkCenterWorldLocation(), FVector((float)GetChunkSize() * GetVoxelSize() * 0.5f));

	InOutData.Label = GetActorNameOrLabel();
	InOutData.LabelColor = FColor::MakeRandomSeededColor(GetChunkSeed());

	InOutData.CommonEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Chunk Size"), GetChunkSize()));
	InOutData.CommonEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Voxel Base Size"), GetVoxelSize()));
	//InOutData.CommonEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Max Updates per Second"), MaxUpdatesPerSecond));

	// AvalancheValue
	//InOutData.StabilityEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Pending Updates Num"), StabilityUpdates.GetPendingPointsConstArray().Num()));
	//InOutData.StabilityEntries.Add(FVoxelChunkDebugData_Entry(TEXT("This Tick Updates Num"), StabilityUpdates.GetThisTickSelectedPointsConstArray().Num()));

	// Instance under cursor
	AScWPlayerController* ScWPlayerController = Cast<AScWPlayerController>(ForPlayerController);

	FHitResult ScreenCenterHitResult;
	ScWPlayerController->GetHitResultUnderScreenCenter(TraceTypeQuery_Visibility, false, ScreenCenterHitResult);

	UATVoxelISMC* TargetComponent = Cast<UATVoxelISMC>(ScreenCenterHitResult.GetComponent());
	if (auto TargetTypeDataPtr = PerTypeVoxelComponentMap.FindKey(TargetComponent))
	{
		InOutData.CommonEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Voxel Component's Instances Num"), TargetComponent->GetVoxelInstancesNum()));

		int32 TargetInstanceIndex = ScreenCenterHitResult.Item;
		const FIntVector& TargetPoint = TargetComponent->GetPointOfMeshIndex(TargetInstanceIndex);
		InOutData.InstanceLabel = FString::Printf(TEXT("Looking at Voxel at %s, instance index %d"), *TargetPoint.ToString(), TargetInstanceIndex);
		
		const FVoxelInstanceData& TargetData = OwnerTree->GetVoxelInstanceDataAtPoint(TargetPoint, false);
		if (TargetData.IsTypeDataValid())
		{
			InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Type Data"), TargetTypeDataPtr->GetName()));
			InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Health"), TargetData.Health));
			InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("AvalancheValue"), TargetData.AvalancheValue));
			//InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Attachment Directions"), FATVoxelUtils::CreateStringFromAttachmentDirections(TargetData.AttachmentDirections)));
		}
		InOutData.InstanceHighlightTransform = FTransform(FRotator::ZeroRotator, UATWorldFunctionLibrary::GetVoxelCenterWorldLocation(TargetPoint, GetVoxelSize()), FVector(GetVoxelSize() * 0.5f));
	}
}
//~ End Debug

#if DEBUG_VOXELS
	#pragma optimize("", on)
#endif // DEBUG_VOXELS
