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

	VoxelComponent = CreateDefaultSubobject<UATVoxelISMC>(TEXT("VoxelComponent"));
	VoxelComponent->SetNumCustomDataFloats(4);
	VoxelComponent->SetupAttachment(RootComponent);
}

//~ Begin Initialize
void AATVoxelChunk::PostInitializeComponents() // AActor
{
	Super::PostInitializeComponents();
}

void AATVoxelChunk::OnConstruction(const FTransform& InTransform) // AActor
{
	Super::OnConstruction(InTransform);

	if (UWorld* World = GetWorld())
	{
		if (VoxelComponent && World->IsEditorWorld())
		{
			VoxelComponent->UpdateAllVoxelsVisibilityState();
		}
	}
}

void AATVoxelChunk::BeginPlay() // AActor
{
	Super::BeginPlay();

	HandleProceduralGeneration();
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
	ensureReturn(VoxelComponent);
	VoxelComponent->BP_InitComponent(this);
}
//~ End Initialize

//~ Begin Voxel Components
UATVoxelISMC* AATVoxelChunk::GetVoxelComponentAtPoint(const FIntVector& InPoint) const
{
	return VoxelComponent;
}
//~ End Voxel Components

//~ Begin Voxel Getters
FVector AATVoxelChunk::GetChunkCenterWorldLocation() const
{
	return GetActorLocation() + FVector((float)GetChunkSize() * GetVoxelSize() * 0.5f);
}

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
//~ End Voxel Getters

//~ Begin Voxel Setters
void AATVoxelChunk::HandleSetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const FVoxelInstanceData& InVoxelInstanceData)
{
	if (UATVoxelISMC* TargetComponent = GetVoxelComponentAtPoint(InPoint))
	{
		TargetComponent->HandleSetVoxelInstanceDataAtPoint(InPoint, InVoxelInstanceData);
	}
}

void AATVoxelChunk::HandleBreakVoxelAtPoint(const FIntVector& InPoint, const bool bInNotify)
{
	if (UATVoxelISMC* TargetComponent = GetVoxelComponentAtPoint(InPoint))
	{
		TargetComponent->HandleBreakVoxelAtPoint(InPoint, bInNotify);
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
void AATVoxelChunk::HandleUpdates(int32& InOutUpdatesLeft)
{
	VoxelComponent->HandleUpdates(InOutUpdatesLeft);
}
//~ End Voxel Data

//~ Begin Voxel Generation
void AATVoxelChunk::HandleProceduralGeneration()
{
	ensureReturn(OwnerTree);
	ensureReturn(OwnerTree->DefaultVoxelTypeData);
	ensureReturn(OwnerTree->WeakVoxelTypeData);

	int32 BaseSeed = OwnerTree->GetBaseSeed();
	int32 ChunkSeed = BaseSeed + ChunkCoords.X * ChunkCoords.X * ChunkCoords.X + ChunkCoords.Y * ChunkCoords.Y + ChunkCoords.Z;

	UFastNoise2PerlinGenerator* PerlinGenerator = OwnerTree->GetVoxelPerlinGenerator();

	TArray<float> PerlinValues;
	FIntVector PerlinStart = GetChunkBackLeftCornerPoint();
	FIntVector PerlinSize = FIntVector(GetChunkSize());
	PerlinGenerator->GenUniformGrid3D(PerlinValues, PerlinStart, PerlinSize, 0.01f, BaseSeed);

	for (int32 SampleArrayIndex = 0; SampleArrayIndex < PerlinValues.Num(); ++SampleArrayIndex)
	{
		float SamplePerlinValue = PerlinValues[SampleArrayIndex];

		if (SamplePerlinValue > 0.0f)
		//if (FMath::RandBool())
		{
			FIntVector SamplePoint = PerlinStart + UATWorldFunctionLibrary::ArrayIndex_To_Point(SampleArrayIndex, PerlinSize);

			if (SamplePerlinValue > 0.1f)
			{
				OwnerTree->SetVoxelAtPoint(SamplePoint, OwnerTree->WeakVoxelTypeData);
			}
			else
			{
				OwnerTree->SetVoxelAtPoint(SamplePoint, OwnerTree->DefaultVoxelTypeData);
			}
		}
	}
}
//~ End Voxel Generation

//~ Begin Debug
void AATVoxelChunk::BP_CollectDataForGameplayDebugger_Implementation(APlayerController* ForPlayerController, FVoxelChunkDebugData& InOutData) const
{
	ensureReturn(VoxelComponent);

	// Common
	InOutData.ChunkHighlightTransform = FTransform(FRotator::ZeroRotator, GetChunkCenterWorldLocation(), FVector((float)GetChunkSize() * GetVoxelSize() * 0.5f));

	InOutData.Label = GetActorLabel();
	InOutData.LabelColor = FColor::MakeRandomSeededColor(GetActorGuid()[0]);

	InOutData.CommonEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Instances Num"), VoxelComponent->GetVoxelInstancesNum()));
	InOutData.CommonEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Chunk Size"), GetChunkSize()));
	InOutData.CommonEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Voxel Base Size"), GetVoxelSize()));
	//InOutData.CommonEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Max Updates per Second"), MaxUpdatesPerSecond));

	// Stability
	//InOutData.StabilityEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Pending Updates Num"), StabilityUpdates.GetPendingPointsConstArray().Num()));
	//InOutData.StabilityEntries.Add(FVoxelChunkDebugData_Entry(TEXT("This Tick Updates Num"), StabilityUpdates.GetThisTickSelectedPointsConstArray().Num()));

	// Instance under cursor
	AScWPlayerController* ScWPlayerController = Cast<AScWPlayerController>(ForPlayerController);

	FHitResult ScreenCenterHitResult;
	ScWPlayerController->GetHitResultUnderScreenCenter(TraceTypeQuery_Visibility, false, ScreenCenterHitResult);

	UInstancedStaticMeshComponent* TargetISMC = Cast<UInstancedStaticMeshComponent>(ScreenCenterHitResult.GetComponent());
	if (TargetISMC == VoxelComponent)
	{
		int32 TargetInstanceIndex = ScreenCenterHitResult.Item;
		const FIntVector& TargetPoint = VoxelComponent->GetPointOfMeshIndex(TargetInstanceIndex);
		InOutData.InstanceLabel = FString::Printf(TEXT("Looking at Voxel at %s, instance index %d"), *TargetPoint.ToString(), TargetInstanceIndex);
		
		const FVoxelInstanceData& TargetData = VoxelComponent->GetVoxelInstanceDataAtPoint(TargetPoint);
		if (TargetData.IsTypeDataValid())
		{
			InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Type Data"), TargetData.TypeData.GetName()));
			InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Health"), TargetData.Health));
			InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Stability"), TargetData.Stability));
			//InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Attachment Directions"), EATAttachmentDirection_Utils::CreateStringFromAttachmentDirections(TargetData.AttachmentDirections)));
		}
		InOutData.InstanceHighlightTransform = FTransform(FRotator::ZeroRotator, UATWorldFunctionLibrary::GetVoxelCenterWorldLocation(TargetPoint, GetVoxelSize()), FVector(GetVoxelSize() * 0.5f));
	}
}
//~ End Debug

#if DEBUG_VOXELS
	#pragma optimize("", on)
#endif // DEBUG_VOXELS
