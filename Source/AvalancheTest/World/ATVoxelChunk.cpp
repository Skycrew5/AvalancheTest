// Scientific Ways

#include "World/ATVoxelChunk.h"

#include "Framework/ScWPlayerController.h"

#include "World/ATVoxelISMC.h"
#include "World/ScWTypes_World.h"
#include "World/ATVoxelTypeData.h"

#if DEBUG_VOXELS
	#pragma optimize("", off)
#endif // DEBUG_VOXELS

void FVoxelChunkPendingUpdates::QueuePointIfRelevant(const FIntVector& InPoint)
{
	if (bIsInsideUpdateSequence)
	{
		if (IsInstanceWaitingToUpdateThisTick(InPoint))
		{
			// Index is going to be updated soon anyway - no need to queue it
		}
		else
		{
			NextTickNewPendingIndices.Add(InPoint);
		}
	}
	else
	{
		PendingPoints.Add(InPoint);
	}
}

void FVoxelChunkPendingUpdates::MarkPointAsUpdatedThisTick(const FIntVector& InPoint)
{
	ensure(!ThisTickAlreadyUpdatedPoints.Contains(InPoint));
	ThisTickAlreadyUpdatedPoints.Add(InPoint);
}

bool FVoxelChunkPendingUpdates::PrepareThisTickSelectedPoints(int32 InDesiredUpdatesNum)
{
	if (bDebugDeMarkThisTickSelectedIndices && !ThisTickSelectedPoints.IsEmpty())
	{
		for (const FIntVector& SamplePoint : ThisTickSelectedPoints.GetConstArray())
		{
			const FVoxelInstanceData& SampleData = TargetISMC->GetVoxelInstanceDataAtPoint(SamplePoint, false);
			if (SampleData.HasMesh())
			{
				TargetISMC->SetCustomDataValue(SampleData.SMI_Index, DebugVoxelCustomData_ThisTickSelected, 0.0f, false);
			}
		}
		TargetISMC->MarkRenderStateDirty();
	}
	int32 UpdatesNum = (TargetISMC->GetOwnerChunk()->MaxUpdatesPerSecond > 0) ? FMath::Min(InDesiredUpdatesNum, PendingPoints.Num()) : PendingPoints.Num();

	ThisTickSelectedPoints.Empty(UpdatesNum);
	PendingPoints.AddHeadTo(UpdatesNum, ThisTickSelectedPoints);

	bIsInsideUpdateSequence = !ThisTickSelectedPoints.IsEmpty();
	return bIsInsideUpdateSequence;
}

bool FVoxelChunkPendingUpdates::IsInstanceWaitingToUpdateThisTick(const FIntVector& InPoint) const
{
	return ThisTickSelectedPoints.Contains(InPoint) && !ThisTickAlreadyUpdatedPoints.Contains(InPoint);
}

void FVoxelChunkPendingUpdates::ResolveThisTickSelectedPoints()
{
	if (bDebugMarkThisTickSelectedIndices && !ThisTickSelectedPoints.IsEmpty())
	{
		for (const FIntVector& SamplePoint : ThisTickSelectedPoints.GetConstArray())
		{
			const FVoxelInstanceData& SampleData = TargetISMC->GetVoxelInstanceDataAtPoint(SamplePoint, false);
			if (SampleData.HasMesh())
			{
				TargetISMC->SetCustomDataValue(SampleData.SMI_Index, DebugVoxelCustomData_ThisTickSelected, 1.0f, false);
			}
		}
		TargetISMC->MarkRenderStateDirty();
	}
	PendingPoints.RemoveFromOther(ThisTickSelectedPoints);
	PendingPoints.AddFromOther(NextTickNewPendingIndices);

	NextTickNewPendingIndices.Empty();
	bIsInsideUpdateSequence = false;
}

int32 FVoxelChunkPendingUpdates::ResolveThisTickAlreadyUpdatedPoints()
{
	int32 OutPointsNum = ThisTickAlreadyUpdatedPoints.Num();
	ThisTickAlreadyUpdatedPoints.Empty();
	return OutPointsNum;
}

FDelegateHandle AATVoxelChunk::InstanceIndexUpdatedDelegateHandle = FDelegateHandle();

AATVoxelChunk::AATVoxelChunk()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	VoxelISMC = CreateDefaultSubobject<UATVoxelISMC>(TEXT("VoxelISMC"));
	VoxelISMC->SetNumCustomDataFloats(4);
	VoxelISMC->SetupAttachment(RootComponent);

	bHandleISMCUpdatesTick = true;

	bHandleVoxelDataUpdatesTick = true;
	ChunkSize = 12;
	VoxelBaseSize = 16.0f;
	bIsOnFoundation = true;
	MaxUpdatesPerSecond = 10000;

	AttachmentUpdates.TargetISMC = VoxelISMC;
	AttachmentUpdates.DebugVoxelCustomData_ThisTickSelected = 1;

	StabilityUpdates.TargetISMC = VoxelISMC;
	StabilityUpdates.DebugVoxelCustomData_ThisTickSelected = 2;

	StabilityUpdatePropagationThreshold = 0.1f;
	StabilityUpdatePropagationSkipProbability = 0.0f;

	bDebugInstancesStabilityValues = true;
	bDebugInstancesHealthValues = true;

	DebugVoxelCustomData_Stability = 0;
	DebugVoxelCustomData_Health = 3;
}

//~ Begin Initialize
void AATVoxelChunk::PostInitializeComponents() // AActor
{
	if (!InstanceIndexUpdatedDelegateHandle.IsValid())
	{
		InstanceIndexUpdatedDelegateHandle = FInstancedStaticMeshDelegates::OnInstanceIndexUpdated.AddStatic(&AATVoxelChunk::Static_OnISMInstanceIndicesUpdated);
	}
	Super::PostInitializeComponents();
}

void AATVoxelChunk::OnConstruction(const FTransform& InTransform) // AActor
{
	BP_CommonInitChunk();

	Super::OnConstruction(InTransform);

	if (UWorld* World = GetWorld())
	{
		if (VoxelISMC && World->IsEditorWorld())
		{
			VoxelISMC->UpdateVoxelsVisibilityState();
		}
	}
}

void AATVoxelChunk::BeginPlay() // AActor
{
	BP_CommonInitChunk();

	Super::BeginPlay();
}

void AATVoxelChunk::Tick(float InDeltaSeconds) // AActor
{
	Super::Tick(InDeltaSeconds);

	int32 UpdatesLeft = FMath::CeilToInt((float)MaxUpdatesPerSecond * InDeltaSeconds);

	if (bHandleISMCUpdatesTick)
	{
		HandleISMCUpdatesTick(UpdatesLeft);
	}
	if (bHandleVoxelDataUpdatesTick)
	{
		HandleVoxelDataUpdatesTick(UpdatesLeft);
	}
	if (bDebugInstancesAttachmentDirection)
	{
		HandleDebugInstancesAttachmentDirections();
	}
	if (bDebugInstancesStabilityValues)
	{
		HandleDebugInstancesStabilityValues();
	}
	if (bDebugInstancesHealthValues)
	{
		HandleDebugInstancesHealthValues();
	}
}

void AATVoxelChunk::EndPlay(const EEndPlayReason::Type InReason) // AActor
{
	

	Super::EndPlay(InReason);
}

void AATVoxelChunk::BP_CommonInitChunk_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		RemoveAllVoxels();

		if (bIsOnFoundation)
		{
			CreateFoundation();
		}
		if (World->IsEditorWorld())
		{
			
		}
		else if (World->IsGameWorld())
		{

		}
	}
}
//~ End Initialize

//~ Begin Components
void AATVoxelChunk::HandleISMCUpdatesTick(int32& InOutMaxUpdates)
{
	VoxelISMC->UpdateVoxelsVisibilityState();
}
//~ End Components

//~ Begin Locations
FIntVector AATVoxelChunk::RelativeLocation_To_LocalPoint(const FVector& InRelativeLocation) const
{
	FVector VoxelScaledRelativeLocation = (InRelativeLocation / VoxelBaseSize);
	return FIntVector(FMath::CeilToInt(VoxelScaledRelativeLocation.X), FMath::CeilToInt(VoxelScaledRelativeLocation.Y), FMath::CeilToInt(VoxelScaledRelativeLocation.Z));
}

FIntVector AATVoxelChunk::WorldLocation_To_LocalPoint(const FVector& InWorldLocation) const
{
	return RelativeLocation_To_LocalPoint(GetActorTransform().InverseTransformPosition(InWorldLocation));
}

FVector AATVoxelChunk::LocalPoint_To_RelativeLocation(const FIntVector& InPoint) const
{
	if (bOffsetMeshToCenter)
	{
		return FVector(InPoint * VoxelBaseSize) + FVector(VoxelBaseSize * 0.5f, VoxelBaseSize * 0.5f, VoxelBaseSize * 0.5f);
	}
	else
	{
		return FVector(InPoint * VoxelBaseSize);
	}
}

FVector AATVoxelChunk::LocalPoint_To_WorldLocation(const FIntVector& InPoint) const
{
	return GetActorTransform().TransformPosition(LocalPoint_To_RelativeLocation(InPoint));
}

FVector AATVoxelChunk::GetChunkCenterWorldLocation() const
{
	return GetActorLocation() + FVector((float)ChunkSize * VoxelBaseSize * 0.5f);
}

FVector AATVoxelChunk::GetVoxelCenterWorldLocation(const FIntVector& InPoint) const
{
	return GetActorLocation() + FVector(InPoint * VoxelBaseSize) + FVector(VoxelBaseSize * 0.5f, VoxelBaseSize * 0.5f, VoxelBaseSize * 0.5f);
}
//~ End Locations

//~ Begin Getters
const FVoxelInstanceData& AATVoxelChunk::GetVoxelInstanceDataAtPoint(const FIntVector& InPoint) const
{
	return VoxelISMC->GetVoxelInstanceDataAtPoint(InPoint);
}

bool AATVoxelChunk::HasVoxelAtPoint(const FIntVector& InPoint) const
{
	return VoxelISMC->HasVoxelAtPoint(InPoint);
}

int32 AATVoxelChunk::GetVoxelNeighborsNumAtLocalPoint(const FIntVector& InPoint) const
{
	int32 OutNum = 0;

	if (HasVoxelAtPoint(InPoint + FIntVector(1, 0, 0))) OutNum += 1;
	if (HasVoxelAtPoint(InPoint + FIntVector(-1, 0, 0))) OutNum += 1;
	if (HasVoxelAtPoint(InPoint + FIntVector(0, 1, 0))) OutNum += 1;
	if (HasVoxelAtPoint(InPoint + FIntVector(0, -1, 0))) OutNum += 1;
	if (HasVoxelAtPoint(InPoint + FIntVector(0, 0, 1))) OutNum += 1;
	if (HasVoxelAtPoint(InPoint + FIntVector(0, 0, -1))) OutNum += 1;
	return OutNum;
}

bool AATVoxelChunk::IsVoxelAtPointFullyClosed(const FIntVector& InPoint) const
{
	if (!HasVoxelAtPoint(InPoint + FIntVector(1, 0, 0)))
	{
		return false;
	}
	if (!HasVoxelAtPoint(InPoint + FIntVector(-1, 0, 0)))
	{
		return false;
	}
	if (!HasVoxelAtPoint(InPoint + FIntVector(0, 1, 0)))
	{
		return false;
	}
	if (!HasVoxelAtPoint(InPoint + FIntVector(0, -1, 0)))
	{
		return false;
	}
	if (!HasVoxelAtPoint(InPoint + FIntVector(0, 0, 1)))
	{
		return false;
	}
	if (!HasVoxelAtPoint(InPoint + FIntVector(0, 0, -1)))
	{
		return false;
	}
	return true;
}

void AATVoxelChunk::GetAllPointsInRadius(const FIntVector& InCenterPoint, int32 InRadius, TArray<FIntVector>& OutPoints) const
{
	ensure(InRadius > 0);

	for (int32 OffsetX = -InRadius; OffsetX < InRadius + 1; ++OffsetX)
	{
		float Alpha = FMath::Abs((float)OffsetX) / (float)InRadius;
		int32 SliceRadius = FMath::Max(FMath::CeilToInt32((float)InRadius * (1.0f - FMath::Square(Alpha))), 1);

		for (int32 OffsetY = -SliceRadius; OffsetY < SliceRadius + 1; ++OffsetY)
		{
			float Alpha2 = FMath::Abs((float)OffsetY) / (float)SliceRadius;
			int32 SliceRadius2 = FMath::Max(FMath::CeilToInt32((float)SliceRadius * (1.0f - FMath::Square(Alpha2))), 1);

			for (int32 OffsetZ = -SliceRadius2; OffsetZ < SliceRadius2 + 1; ++OffsetZ)
			{
				OutPoints.Add(InCenterPoint + FIntVector(OffsetX, OffsetY, OffsetZ));
			}
		}
	}
}
//~ End Getters

//~ Begin Setters
bool AATVoxelChunk::SetVoxelAtLocalPoint(const FIntVector& InPoint, const UATVoxelTypeData* InTypeData, const bool bInForced)
{
	return VoxelISMC->SetVoxelAtPoint(InPoint, InTypeData, bInForced);
}

bool AATVoxelChunk::SetVoxelsAtLocalPoints(const TArray<FIntVector>& InPoints, const UATVoxelTypeData* InTypeData, const bool bInForced)
{
	return VoxelISMC->SetVoxelsAtPoints(InPoints, InTypeData, bInForced);
}

void AATVoxelChunk::FillWithVoxels(const UATVoxelTypeData* InTypeData)
{
	TArray<FIntVector> VoxelPoints;

	for (int32 SampleX = 0; SampleX < ChunkSize; ++SampleX)
	{
		for (int32 SampleY = 0; SampleY < ChunkSize; ++SampleY)
		{
			for (int32 SampleZ = 0; SampleZ < ChunkSize; ++SampleZ)
			{
				VoxelPoints.Add(FIntVector(SampleX, SampleY, SampleZ));
			}
		}
	}
	SetVoxelsAtLocalPoints(VoxelPoints, InTypeData, true);
}

void AATVoxelChunk::RemoveAllVoxels()
{
	VoxelISMC->RemoveAllVoxels();
}

bool AATVoxelChunk::BreakVoxelAtLocalPoint(const FIntVector& InPoint, const bool bInForced, const bool bInNotify)
{
	if (bInForced)
	{
		if (VoxelISMC->RemoveVoxelAtPoint(InPoint, false))
		{
			if (bInNotify)
			{
				OnBreakVoxelAtLocalPoint.Broadcast(InPoint, bInForced);
			}
			return true;
		}
	}
	else
	{
		const FVoxelInstanceData& TargetData = VoxelISMC->GetVoxelInstanceDataAtPoint(InPoint, false);
		if (TargetData.IsTypeDataValid() && !TargetData.TypeData->IsFoundation)
		{
			if (VoxelISMC->RemoveVoxelAtPoint(InPoint))
			{
				if (bInNotify)
				{
					OnBreakVoxelAtLocalPoint.Broadcast(InPoint, bInForced);
				}
				return true;
			}
		}
	}
	return false;
}

bool AATVoxelChunk::BreakVoxelsAtLocalPoints(const TArray<FIntVector>& InPoints, const bool bInForced, const bool bInNotify)
{
	bool bAnyBroken = false;

	for (const FIntVector& SamplePoint : InPoints)
	{
		bAnyBroken |= BreakVoxelAtLocalPoint(SamplePoint, bInForced, bInNotify);
	}
	return bAnyBroken;
}

void AATVoxelChunk::CreateFoundation()
{
	ensure(FoundationVoxelTypeData);
	if (!FoundationVoxelTypeData)
	{
		return;
	}
	BreakVoxelsAtLocalPoints(VoxelISMC->FoundationLocalPoints, true);
	VoxelISMC->FoundationLocalPoints.Empty(ChunkSize * ChunkSize);

	for (int32 SampleX = 0; SampleX < ChunkSize; ++SampleX)
	{
		for (int32 SampleY = 0; SampleY < ChunkSize; ++SampleY)
		{
			VoxelISMC->FoundationLocalPoints.Add(FIntVector(SampleX, SampleY, -1));
		}
	}
	SetVoxelsAtLocalPoints(VoxelISMC->FoundationLocalPoints, FoundationVoxelTypeData);
}
//~ End Setters

//~ Begin Voxel Data
void AATVoxelChunk::QueueFullUpdate()
{
	TArray<FIntVector> AllInitialPoints;
	VoxelISMC->GetAllLocalPoints(AllInitialPoints);

	for (const FIntVector& SamplePoint : AllInitialPoints)
	{
		//AttachmentUpdates.QueuePointIfRelevant(SamplePoint);
		//StabilityUpdates.QueuePointIfRelevant(SamplePoint);
		QueueRecursiveStabilityUpdate(SamplePoint);
		VoxelISMC->QueuePointForVisibilityUpdate(SamplePoint);
	}
}

void AATVoxelChunk::HandleVoxelDataUpdatesTick(int32& InOutUpdatesLeft)
{
	//InOutMaxUpdates -= UpdatePendingAttachmentData(InOutMaxUpdates);
	//InOutMaxUpdates -= UpdatePendingStabilityData(InOutMaxUpdates);
	UpdateStabilityRecursive(InOutUpdatesLeft);
	UpdateHealth(InOutUpdatesLeft);
}

static TArray<EATAttachmentDirection> DirectionsOrder1 = { EATAttachmentDirection::Bottom, EATAttachmentDirection::Right, EATAttachmentDirection::Left, EATAttachmentDirection::Front, EATAttachmentDirection::Back, EATAttachmentDirection::Top };
static TArray<EATAttachmentDirection> DirectionsOrder2 = { EATAttachmentDirection::Top, EATAttachmentDirection::Back, EATAttachmentDirection::Front, EATAttachmentDirection::Left, EATAttachmentDirection::Right, EATAttachmentDirection::Bottom };
static TArray<EATAttachmentDirection> DirectionsOrder3 = { EATAttachmentDirection::Left, EATAttachmentDirection::Right, EATAttachmentDirection::Front, EATAttachmentDirection::Back, EATAttachmentDirection::Top, EATAttachmentDirection::Bottom };
static TArray<EATAttachmentDirection> DirectionsOrder4 = { EATAttachmentDirection::Right, EATAttachmentDirection::Left, EATAttachmentDirection::Back, EATAttachmentDirection::Front, EATAttachmentDirection::Top, EATAttachmentDirection::Bottom };
static TArray<EATAttachmentDirection> DirectionsOrder5 = { EATAttachmentDirection::Front, EATAttachmentDirection::Back, EATAttachmentDirection::Bottom, EATAttachmentDirection::Top, EATAttachmentDirection::Left, EATAttachmentDirection::Right };
static TArray<EATAttachmentDirection> DirectionsOrder6 = { EATAttachmentDirection::Back, EATAttachmentDirection::Front, EATAttachmentDirection::Top, EATAttachmentDirection::Bottom, EATAttachmentDirection::Right, EATAttachmentDirection::Left };

static TArray<TArray<EATAttachmentDirection>*> UsedDirectionsOrders = { &DirectionsOrder1, &DirectionsOrder2, &DirectionsOrder3, &DirectionsOrder4, &DirectionsOrder5, &DirectionsOrder6 };

static uint8 MaxRecursionLevel = 24u;

void AATVoxelChunk::UpdateStabilityRecursive(int32& InOutUpdatesLeft)
{
	/*if (!QueuedRecursiveStabilityUpdatePoints.IsEmpty())
	{
		VoxelISMC->RegenerateCompoundData();
	}*/

	TArraySetPair<FIntVector> SelectedUpdatePoints;
	QueuedRecursiveStabilityUpdatePoints.AddHeadTo(InOutUpdatesLeft, SelectedUpdatePoints, true);

	ParallelFor(SelectedUpdatePoints.Num(), [&](int32 InIndex)
	{
		const FIntVector& SamplePoint = SelectedUpdatePoints.GetConstArray()[InIndex];
		FVoxelInstanceData& SampleData = VoxelISMC->GetVoxelInstanceDataAtPoint(SamplePoint, false);

		//TSet<FIntVector> DummySubChain;
		//SampleData.Stability = UpdateStabilityRecursive_GetStabilityFromAllNeighbors(DummySubChain, 0, SamplePoint);
		//UpdateStabilityRecursive_CachedSubchainStabilities.Empty();

		SampleData.Stability = 0.0f;

		for (uint8 SampleOrderIndex = 0; SampleOrderIndex < UsedDirectionsOrders.Num(); ++SampleOrderIndex)
		{
			if (SampleData.Stability < 1.0f)
			{
				FRecursiveThreadData ThreadData = FRecursiveThreadData(*UsedDirectionsOrders[SampleOrderIndex]);
				SampleData.Stability = FMath::Max(UpdateStabilityRecursive_GetStabilityFromAllNeighbors(SamplePoint, ThreadData), SampleData.Stability);
			}
			else
			{
				break;
			}
		}
		//UpdateStabilityRecursive_CachedSubchainStabilities.Empty();
		//UpdateStabilityRecursive_CachedPointStabilities.Add(SamplePoint, SampleData.Stability);

	});

	for (const FIntVector& SamplePoint : SelectedUpdatePoints.GetConstArray())
	{
		FVoxelInstanceData& SampleData = VoxelISMC->GetVoxelInstanceDataAtPoint(SamplePoint, false);
		if (bDebugInstancesStabilityValues && SampleData.HasMesh())
		{
			DebugInstancesStablityValues_QueuedIndices.Add(SampleData.SMI_Index);
		}
		if (SampleData.Stability < 0.25f)
		{
			InDangerGroupHealthUpdatePoints.Add(SamplePoint);
		}
	}
	InOutUpdatesLeft -= SelectedUpdatePoints.Num();
	//UpdateStabilityRecursive_CachedPointStabilities.Empty();
}

float AATVoxelChunk::UpdateStabilityRecursive_GetStabilityFromAllNeighbors(const FIntVector& InTargetPoint, FRecursiveThreadData& InThreadData, EATAttachmentDirection InNeighborDirection, uint8 InCurrentRecursionLevel)
{
	++InCurrentRecursionLevel;

	if (InCurrentRecursionLevel > MaxRecursionLevel)
	{
		return 1.0f * EATAttachmentDirection_Utils::AttachmentMuls[InNeighborDirection];
	}
	FIntVector SamplePoint = InTargetPoint + EATAttachmentDirection_Utils::IntOffsets[InNeighborDirection];

	if (InThreadData.ThisOrderUpdatedPoints.Contains(SamplePoint))
	{
		return 0.0f;
	}
	FVoxelInstanceData& SampleData = VoxelISMC->GetVoxelInstanceDataAtPoint(SamplePoint, false);
	if (SampleData.IsTypeDataValid())
	{
		if (SampleData.TypeData->IsFoundation)
		{
			return 1.0f * EATAttachmentDirection_Utils::AttachmentMuls[InNeighborDirection];
		}
		else
		{
			InThreadData.ThisOrderUpdatedPoints.Add(SamplePoint);
			float AccumulatedStability = 0.0f;

			AccumulatedStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(SamplePoint, InThreadData, InThreadData.DirectionsOrder[0], InCurrentRecursionLevel);
			if (AccumulatedStability < 1.0f)
				AccumulatedStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(SamplePoint, InThreadData, InThreadData.DirectionsOrder[1], InCurrentRecursionLevel);
			if (AccumulatedStability < 1.0f)
				AccumulatedStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(SamplePoint, InThreadData, InThreadData.DirectionsOrder[2], InCurrentRecursionLevel);
			if (AccumulatedStability < 1.0f)
				AccumulatedStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(SamplePoint, InThreadData, InThreadData.DirectionsOrder[3], InCurrentRecursionLevel);
			if (AccumulatedStability < 1.0f)
				AccumulatedStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(SamplePoint, InThreadData, InThreadData.DirectionsOrder[4], InCurrentRecursionLevel);
			if (AccumulatedStability < 1.0f)
				AccumulatedStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(SamplePoint, InThreadData, InThreadData.DirectionsOrder[5], InCurrentRecursionLevel);

			float OutStability = FMath::Min(AccumulatedStability * EATAttachmentDirection_Utils::AttachmentMuls[InNeighborDirection], 1.0f);
			return OutStability;
		}
	}
	InThreadData.ThisOrderUpdatedPoints.Add(SamplePoint);
	return 0.0f;
}

void AATVoxelChunk::UpdateHealth(int32& InOutUpdatesLeft)
{
	//TArraySetPair<FIntVector> SelectedUpdatePoints;
	//InDangerGroupHealthUpdatePoints.AddHeadTo(InOutUpdatesLeft, SelectedUpdatePoints, true);

	float DeltaTime = GetWorld()->DeltaTimeSeconds;

	ParallelFor(InDangerGroupHealthUpdatePoints.Num(), [&](int32 InIndex)
	{
		const FIntVector& SamplePoint = InDangerGroupHealthUpdatePoints.GetConstArray()[InIndex];
		FVoxelInstanceData& SampleData = VoxelISMC->GetVoxelInstanceDataAtPoint(SamplePoint, false);

		//ensure(SampleData.Stability < 0.25f);

		float HealthDrainMul = FMath::Square(FMath::Max(0.25f - SampleData.Stability, 0.1f) * 8.0f);
		SampleData.Health -= HealthDrainMul * DeltaTime;
	});

	TArraySetPair<FIntVector> BrokenPoints;
	for (const FIntVector& SamplePoint : InDangerGroupHealthUpdatePoints.GetConstArray())
	{
		FVoxelInstanceData& SampleData = VoxelISMC->GetVoxelInstanceDataAtPoint(SamplePoint, false);

		if (SampleData.Health > 0.0f)
		{
			if (bDebugInstancesHealthValues && SampleData.HasMesh())
			{
				DebugInstancesHealthValues_QueuedIndices.Add(SampleData.SMI_Index);
			}
		}
		else
		{
			BrokenPoints.Add(SamplePoint);
			BreakVoxelAtLocalPoint(SamplePoint, true, true);
		}
	}
	InDangerGroupHealthUpdatePoints.RemoveFromOther(BrokenPoints);
	//InOutUpdatesLeft -= SelectedUpdatePoints.Num();
}

/*float AATVoxelChunk::UpdateStabilityRecursive_GetStabilityFromAllNeighbors(const FIntVector& InTargetPoint, EATAttachmentDirection InNeighborDirection)
{
	TArray<FIntVector> SampleCompoundOrigins;
	VoxelISMC->GetAdjacentCompoundOriginsAt(InTargetPoint, InNeighborDirection, SampleCompoundOrigins);

	// Those are all compounds (their origins) on the InNeighborDirection side
	for (const FIntVector& SampleOrigin : SampleCompoundOrigins)
	{
		UpdateStabilityRecursive_GetStabilityFromAllNeighbors_HandleCompound(SampleOrigin, InNeighborDirection);
	}
	return 0.0f;
}

float AATVoxelChunk::UpdateStabilityRecursive_GetStabilityFromAllNeighbors_HandleCompound(const FIntVector& InOrigin, EATAttachmentDirection InNeighborDirection)
{
	const FVoxelCompoundData& SampleCompoundData = VoxelISMC->GetCompoundDataAt(InOrigin);

	if (UpdateStabilityRecursive_ThisOrderUpdatedPoints.Contains(InOrigin))
	{
		return 0.0f;
	}
	TArray<FIntVector> SampleCompoundPoints;
	SampleCompoundData.GetAllPoints(SampleCompoundPoints);

	FVoxelInstanceData& SampleData = VoxelISMC->GetVoxelInstanceDataAtPoint(InOrigin, false);
	if (SampleData.IsTypeDataValid())
	{
		if (SampleData.TypeData->IsFoundation)
		{
			return 1.0f * EATAttachmentDirection_Utils::AttachmentMuls[InNeighborDirection];
		}
		else
		{
			UpdateStabilityRecursive_ThisOrderUpdatedPoints.Append(SampleCompoundPoints);
			float AccumulatedStability = 0.0f;

			AccumulatedStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(SamplePoint, CurrentDirectionsOrder[0]);
			if (AccumulatedStability < 1.0f)
				AccumulatedStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(SamplePoint, CurrentDirectionsOrder[1]);
			if (AccumulatedStability < 1.0f)
				AccumulatedStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(SamplePoint, CurrentDirectionsOrder[2]);
			if (AccumulatedStability < 1.0f)
				AccumulatedStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(SamplePoint, CurrentDirectionsOrder[3]);
			if (AccumulatedStability < 1.0f)
				AccumulatedStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(SamplePoint, CurrentDirectionsOrder[4]);
			if (AccumulatedStability < 1.0f)
				AccumulatedStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(SamplePoint, CurrentDirectionsOrder[5]);

			float OutStability = FMath::Min(AccumulatedStability * EATAttachmentDirection_Utils::AttachmentMuls[InNeighborDirection], 1.0f);
			return OutStability;
		}
	}
	UpdateStabilityRecursive_ThisOrderUpdatedPoints.Append(SampleCompoundPoints);
	return 0.0f;
}*/

/*float AATVoxelChunk::UpdateStabilityRecursive_GetStabilityFromAllNeighbors(TSet<FIntVector>& InOutCurrentSubChain, int32 InSubChainHash, const FIntVector& FromPoint, EATAttachmentDirection InNeighborDirection)
{
	FIntVector SamplePoint = FromPoint + EATAttachmentDirection_Utils::IntOffsets[InNeighborDirection];

	if (InOutCurrentSubChain.Contains(SamplePoint))
	{
		return 0.0f;
	}
	InOutCurrentSubChain.Add(SamplePoint);
	int32 NewSubChainHash = HashCombineFast(InSubChainHash, GetTypeHash(SamplePoint));
	//int32 NewSubChainHash = HashCombineFast(GetTypeHash(FromPoint), GetTypeHash(SamplePoint));

	if (UpdateStabilityRecursive_CachedSubchainStabilities.Contains(NewSubChainHash))
	{
		return UpdateStabilityRecursive_CachedSubchainStabilities[NewSubChainHash];
	}
	FVoxelInstanceData& SampleData = VoxelISMC->GetVoxelInstanceDataAtPoint(SamplePoint, false);
	if (SampleData.IsTypeDataValid())
	{
		if (SampleData.TypeData->IsFoundation)
		{
			return 1.0f * EATAttachmentDirection_Utils::AttachmentMuls[InNeighborDirection];
		}
		else
		{
			float NewStability = 0.0f;

			// Bottom voxel
			{
				TSet<FIntVector> BottomSubChain = InOutCurrentSubChain;
				NewStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(BottomSubChain, NewSubChainHash, SamplePoint, EATAttachmentDirection::Bottom);
			}
			// Front voxel
			if (NewStability < 1.0f)
			{
				TSet<FIntVector> FrontSubChain = InOutCurrentSubChain;
				NewStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(FrontSubChain, NewSubChainHash, SamplePoint, EATAttachmentDirection::Front);
			}
			// Back voxel
			if (NewStability < 1.0f)
			{
				TSet<FIntVector> BackSubChain = InOutCurrentSubChain;
				NewStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(BackSubChain, NewSubChainHash, SamplePoint, EATAttachmentDirection::Back);
			}
			// Right voxel
			if (NewStability < 1.0f)
			{
				TSet<FIntVector> RightSubChain = InOutCurrentSubChain;
				NewStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(RightSubChain, NewSubChainHash, SamplePoint, EATAttachmentDirection::Right);
			}
			// Left voxel
			if (NewStability < 1.0f)
			{
				TSet<FIntVector> LeftSubChain = InOutCurrentSubChain;
				NewStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(LeftSubChain, NewSubChainHash, SamplePoint, EATAttachmentDirection::Left);
			}
			// Top voxel
			if (NewStability < 1.0f)
			{
				TSet<FIntVector> TopSubChain = InOutCurrentSubChain;
				NewStability += UpdateStabilityRecursive_GetStabilityFromAllNeighbors(TopSubChain, NewSubChainHash, SamplePoint, EATAttachmentDirection::Top);
			}
			float OutStability = FMath::Min(NewStability * EATAttachmentDirection_Utils::AttachmentMuls[InNeighborDirection], 1.0f);
			UpdateStabilityRecursive_CachedSubchainStabilities.Add(NewSubChainHash, OutStability);
			return OutStability;
		}
	}
	return 0.0f;
}*/

void AATVoxelChunk::QueueRecursiveStabilityUpdate(const FIntVector& InPoint, const bool bInQueueNeighborsToo)
{
	QueuedRecursiveStabilityUpdatePoints.Add(InPoint);

	if (bInQueueNeighborsToo)
	{
		TArray<FIntVector> PointsInRadius;
		GetAllPointsInRadius(InPoint, 6, PointsInRadius);

		for (const FIntVector& SamplePoint : PointsInRadius)
		{
			QueuedRecursiveStabilityUpdatePoints.Add(SamplePoint);
		}
		/*QueuedRecursiveStabilityUpdatePoints.Add(InPoint + FIntVector(1, 0, 0));
		QueuedRecursiveStabilityUpdatePoints.Add(InPoint + FIntVector(-1, 0, 0));
		QueuedRecursiveStabilityUpdatePoints.Add(InPoint + FIntVector(0, 1, 0));
		QueuedRecursiveStabilityUpdatePoints.Add(InPoint + FIntVector(0, -1, 0));
		QueuedRecursiveStabilityUpdatePoints.Add(InPoint + FIntVector(0, 0, 1));
		QueuedRecursiveStabilityUpdatePoints.Add(InPoint + FIntVector(0, 0, -1));*/
	}
}

int32 AATVoxelChunk::UpdatePendingAttachmentData(int32 InMaxUpdates)
{
	if (!AttachmentUpdates.PrepareThisTickSelectedPoints(InMaxUpdates))
	{
		AttachmentUpdates.ResolveThisTickSelectedPoints();
		return 0;
	}
	for (const FIntVector& SamplePoint : AttachmentUpdates.GetThisTickSelectedPointsConstArray())
	{
		FVoxelInstanceData& SampleData = VoxelISMC->GetVoxelInstanceDataAtPoint(SamplePoint, false);
		TMap<FIntVector, EATAttachmentDirection> AttachedNeighborsAndDirections;

		if (SampleData.IsTypeDataValid())
		{
			TArray<EATAttachmentDirection> PrevAttachmentDirections = SampleData.AttachmentDirections.Array();

			if (SampleData.TypeData->IsFoundation)
			{
				SampleData.AttachmentDirections = { EATAttachmentDirection::Bottom };
			}
			else
			{
				SampleData.AttachmentDirections.Empty();
				UpdatePendingAttachmentData_UpdateFromAllNeighbors(SamplePoint, AttachedNeighborsAndDirections); // Will update directions and queue neighbors if needed
			}
			if (SampleData.AttachmentDirections.IsEmpty() && !AttachedNeighborsAndDirections.IsEmpty())
			{
				for (const TPair<FIntVector, EATAttachmentDirection>& SamplePointAndDirection : AttachedNeighborsAndDirections)
				{
					FVoxelInstanceData& SampleAttachedNeighborData = VoxelISMC->GetVoxelInstanceDataAtPoint(SamplePointAndDirection.Key);
					SampleAttachedNeighborData.AttachmentDirections.Remove(SamplePointAndDirection.Value);
					SampleData.AttachmentDirections.Add(EATAttachmentDirection_Utils::Opposites[SamplePointAndDirection.Value]);
					AttachmentUpdates.QueuePointIfRelevant(SamplePointAndDirection.Key);
				}
			}
			else if (SampleData.AttachmentDirections.Array() != PrevAttachmentDirections)
			{
				StabilityUpdates.QueuePointIfRelevant(SamplePoint);
			}
			if (bDebugInstancesAttachmentDirection && SampleData.HasMesh())
			{
				DebugInstancesAttachmentDirections_QueuedIndices.Add(SampleData.SMI_Index);
			}
		}
		else
		{
			UpdatePendingAttachmentData_UpdateFromAllNeighbors(SamplePoint, AttachedNeighborsAndDirections); // Will queue neighbors if needed
		}
		AttachmentUpdates.MarkPointAsUpdatedThisTick(SamplePoint);
	}
	AttachmentUpdates.ResolveThisTickSelectedPoints();
	return AttachmentUpdates.ResolveThisTickAlreadyUpdatedPoints();
}

void AATVoxelChunk::UpdatePendingAttachmentData_UpdateFromAllNeighbors(const FIntVector& InTargetPoint, TMap<FIntVector, EATAttachmentDirection>& InOutAttachedNeighborsAndDirections)
{
	static const float SideAttachmentMul = 0.5f;
	static const float TopAttachmentMul = 0.25f;
	static const float BottomAttachmentMul = 1.0f;

	// Front voxel
	UpdatePendingAttachmentData_UpdateFromNeighbor(InTargetPoint, FIntVector(1, 0, 0), EATAttachmentDirection::Back, EATAttachmentDirection::Front, InOutAttachedNeighborsAndDirections);

	// Back voxel
	UpdatePendingAttachmentData_UpdateFromNeighbor(InTargetPoint, FIntVector(-1, 0, 0), EATAttachmentDirection::Front, EATAttachmentDirection::Back, InOutAttachedNeighborsAndDirections);

	// Right voxel
	UpdatePendingAttachmentData_UpdateFromNeighbor(InTargetPoint, FIntVector(0, 1, 0), EATAttachmentDirection::Left, EATAttachmentDirection::Right, InOutAttachedNeighborsAndDirections);

	// Left voxel
	UpdatePendingAttachmentData_UpdateFromNeighbor(InTargetPoint, FIntVector(0, -1, 0), EATAttachmentDirection::Right, EATAttachmentDirection::Left, InOutAttachedNeighborsAndDirections);

	// Top voxel
	UpdatePendingAttachmentData_UpdateFromNeighbor(InTargetPoint, FIntVector(0, 0, 1), EATAttachmentDirection::Bottom, EATAttachmentDirection::Top, InOutAttachedNeighborsAndDirections);

	// Bottom voxel
	UpdatePendingAttachmentData_UpdateFromNeighbor(InTargetPoint, FIntVector(0, 0, -1), EATAttachmentDirection::Top, EATAttachmentDirection::Bottom, InOutAttachedNeighborsAndDirections);
}

void AATVoxelChunk::UpdatePendingAttachmentData_UpdateFromNeighbor(const FIntVector& InTargetPoint, const FIntVector& InNeighborOffset, EATAttachmentDirection ToTargetDirection, EATAttachmentDirection ToNeighborDirection, TMap<FIntVector, EATAttachmentDirection>& InOutAttachedNeighborsAndDirections)
{
	FVoxelInstanceData& TargetData = VoxelISMC->GetVoxelInstanceDataAtPoint(InTargetPoint, false);

	const FIntVector& NeighborPoint = InTargetPoint + InNeighborOffset;
	FVoxelInstanceData& NeighborData = VoxelISMC->GetVoxelInstanceDataAtPoint(NeighborPoint, false);

	bool bAddNeighborToPending = false;

	if (TargetData.IsTypeDataValid() && NeighborData.IsTypeDataValid())
	{
		if (NeighborData.TypeData->IsFoundation) // Simple case with foundaion neighbor
		{
			TargetData.AttachmentDirections.Add(ToNeighborDirection);
		}
		else
		{
			if (NeighborData.IsAttachmentDataValid()) // Neighbor has some attachment data
			{
				if (NeighborData.IsAttachedTo(ToTargetDirection)) // Neighbor is attached to Target, avoid mutual attachments
				{
					// But also Target is not stable enough to hold Neighbor, need to detatch Neighbor and queue it to update attachments
					if (TargetData.Stability < NeighborData.Stability)
					{
						//NeighborData.AttachmentDirections.Remove(ToNeighborDirection);
						AttachmentUpdates.QueuePointIfRelevant(NeighborPoint);
					}
					else
					{
						InOutAttachedNeighborsAndDirections.Add(NeighborPoint, ToTargetDirection);
					}
				}
				else
				{
					if (NeighborData.Stability > 0.0f && TargetData.Stability <= NeighborData.Stability) // Attach Target to Neighbor only if Neighbor is stable enough
					{
						TargetData.AttachmentDirections.Add(ToNeighborDirection);
					}
					else // Otherwise Neighbor needs to be attached to Target, queue for it if is not already
					{
						AttachmentUpdates.QueuePointIfRelevant(NeighborPoint);
					}
				}
			}
			else
			{
				AttachmentUpdates.QueuePointIfRelevant(NeighborPoint);
			}
		}

	}
	else if (NeighborData.IsTypeDataValid()) // Target is not valid, while Neighbor is
	{
		// Probably update from removing Target, spread it to Neighbor
		AttachmentUpdates.QueuePointIfRelevant(NeighborPoint);
	}
}

int32 AATVoxelChunk::UpdatePendingStabilityData(int32 InMaxUpdates)
{
	if (!StabilityUpdates.PrepareThisTickSelectedPoints(InMaxUpdates))
	{
		return 0;
	}
	static const float SideAttachmentMul = 0.8f;
	static const float TopAttachmentMul = 0.4f;
	static const float BottomAttachmentMul = 1.0f;

	for (const FIntVector& SamplePoint : StabilityUpdates.GetThisTickSelectedPointsConstArray())
	{
		FVoxelInstanceData& SampleData = VoxelISMC->GetVoxelInstanceDataAtPoint(SamplePoint, false);

		if (SampleData.IsTypeDataValid())
		{
			float PrevStability = SampleData.Stability;
			TArray<FIntVector> AttachedOrInvalidNeighbors;
			
			if (SampleData.TypeData->IsFoundation)
			{
				SampleData.Stability = 1.0f;
			}
			else
			{
				SampleData.Stability = 0.0f;

				// Front voxel
				UpdatePendingStabilityData_UpdateFromNeighbor(SamplePoint, FIntVector(1, 0, 0), SideAttachmentMul, EATAttachmentDirection::Back, EATAttachmentDirection::Front, AttachedOrInvalidNeighbors);

				// Back voxel
				UpdatePendingStabilityData_UpdateFromNeighbor(SamplePoint, FIntVector(-1, 0, 0), SideAttachmentMul, EATAttachmentDirection::Front, EATAttachmentDirection::Back, AttachedOrInvalidNeighbors);

				// Right voxel
				UpdatePendingStabilityData_UpdateFromNeighbor(SamplePoint, FIntVector(0, 1, 0), SideAttachmentMul, EATAttachmentDirection::Left, EATAttachmentDirection::Right, AttachedOrInvalidNeighbors);

				// Left voxel
				UpdatePendingStabilityData_UpdateFromNeighbor(SamplePoint, FIntVector(0, -1, 0), SideAttachmentMul, EATAttachmentDirection::Right, EATAttachmentDirection::Left, AttachedOrInvalidNeighbors);

				// Top voxel
				UpdatePendingStabilityData_UpdateFromNeighbor(SamplePoint, FIntVector(0, 0, 1), TopAttachmentMul, EATAttachmentDirection::Bottom, EATAttachmentDirection::Top, AttachedOrInvalidNeighbors);

				// Bottom voxel
				UpdatePendingStabilityData_UpdateFromNeighbor(SamplePoint, FIntVector(0, 0, -1), BottomAttachmentMul, EATAttachmentDirection::Top, EATAttachmentDirection::Bottom, AttachedOrInvalidNeighbors);
				
				SampleData.Stability = FMath::Min(FMath::Floor(SampleData.Stability * 100.0f) / 100.0f, 1.0f);
			}
			if (FMath::IsNearlyEqual(SampleData.Stability, PrevStability, StabilityUpdatePropagationThreshold))
			{
				// Stability remained unchanged (mostly), don't notify neighbors
			}
			else if (StabilityUpdatePropagationSkipProbability <= 0.0f || FMath::FRand() > StabilityUpdatePropagationSkipProbability)
			{
				// Notify all attached neighbors to check their stability because target's stability changed
				for (const FIntVector& SampleNeighborPoint : AttachedOrInvalidNeighbors)
				{
					StabilityUpdates.QueuePointIfRelevant(SampleNeighborPoint);
				}
			}
			if (bDebugInstancesStabilityValues && SampleData.HasMesh())
			{
				DebugInstancesStablityValues_QueuedIndices.Add(SampleData.SMI_Index);
			}
		}
		StabilityUpdates.MarkPointAsUpdatedThisTick(SamplePoint);
	}
	StabilityUpdates.ResolveThisTickSelectedPoints();
	return StabilityUpdates.ResolveThisTickAlreadyUpdatedPoints();
}

void AATVoxelChunk::UpdatePendingStabilityData_UpdateFromNeighbor(const FIntVector& InTargetPoint, const FIntVector& InNeighborOffset, float InAttachmentStrengthMul, EATAttachmentDirection ToTargetDirection, EATAttachmentDirection ToNeighborDirection, TArray<FIntVector>& InOutAttachedOrInvalidNeighbors)
{
	FVoxelInstanceData& TargetData = VoxelISMC->GetVoxelInstanceDataAtPoint(InTargetPoint);

	const FIntVector& NeighborPoint = InTargetPoint + InNeighborOffset;
	FVoxelInstanceData& NeighborData = VoxelISMC->GetVoxelInstanceDataAtPoint(NeighborPoint, false);

	// Only consider update if both instances are valid
	if (TargetData.IsTypeDataValid() && NeighborData.IsTypeDataValid())
	{
		if (NeighborData.TypeData->IsFoundation)
		{
			TargetData.Stability += 1.0f * InAttachmentStrengthMul;
		}
		else
		{
			if (NeighborData.IsAttachmentDataValid())
			{
				if (NeighborData.IsAttachedTo(ToTargetDirection))
				{
					// Neighbor is attached to Target, Target shouldn't get Stability from Neighbor
					InOutAttachedOrInvalidNeighbors.Add(NeighborPoint);
				}
				else if (TargetData.IsAttachedTo(ToNeighborDirection))
				{
					// Target is attached to Neighbor, Target should get Stability from Neighbor
					TargetData.Stability += NeighborData.Stability * InAttachmentStrengthMul;
				}
			}
			else
			{
				InOutAttachedOrInvalidNeighbors.Add(NeighborPoint);
			}
		}
	}
	else if (NeighborData.IsTypeDataValid()) // Target is not valid, while Neighbor is
	{
		// Probably update from removing Target, spread it to Neighbor
		StabilityUpdates.QueuePointIfRelevant(NeighborPoint);
	}
}

void AATVoxelChunk::Static_OnISMInstanceIndicesUpdated(UInstancedStaticMeshComponent* InUpdatedComponent, TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData> InIndexUpdates)
{
	check(InUpdatedComponent);

	if (AATVoxelChunk* TargetChunk = InUpdatedComponent->GetOwner<AATVoxelChunk>())
	{
		TargetChunk->HandleInstanceIndicesUpdates(InIndexUpdates);
	}
}

void AATVoxelChunk::HandleInstanceIndicesUpdates(const TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData>& InIndexUpdates)
{
	for (const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData& SampleUpdate : InIndexUpdates)
	{
		bool bQueueDebug = false;

		switch (SampleUpdate.Type)
		{
			case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Added:
			{
				bQueueDebug = true;
				break;
			}
			case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Removed:
			case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Cleared:
			case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Destroyed:
			{
				break;
			}
			case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Relocated:
			{
				VoxelISMC->RelocateInstanceIndex(SampleUpdate.OldIndex, SampleUpdate.Index);
				bQueueDebug = true;
				break;
			}
		}
		if (bQueueDebug)
		{
			if (bDebugInstancesAttachmentDirection)
			{
				DebugInstancesAttachmentDirections_QueuedIndices.Add(SampleUpdate.Index);
			}
			if (bDebugInstancesStabilityValues)
			{
				DebugInstancesStablityValues_QueuedIndices.Add(SampleUpdate.Index);
			}
			if (bDebugInstancesHealthValues)
			{
				DebugInstancesHealthValues_QueuedIndices.Add(SampleUpdate.Index);
			}
		}
	}
}
//~ End Voxel Data

//~ Begin Debug
void AATVoxelChunk::HandleDebugInstancesAttachmentDirections()
{
	if (!DebugInstancesAttachmentDirections_QueuedIndices.IsEmpty())
	{
		for (int32 SampleQueuedInstanceIndex : DebugInstancesAttachmentDirections_QueuedIndices)
		{
			DebugInstanceAttachmentDirection(SampleQueuedInstanceIndex);
		}
		DebugInstancesAttachmentDirections_QueuedIndices.Empty();
		VoxelISMC->MarkRenderStateDirty();
	}
}

void AATVoxelChunk::DebugInstanceAttachmentDirection(int32 InInstanceIndex)
{
	const FVoxelInstanceData& SampleData = VoxelISMC->GetVoxelInstanceDataAtPoint(VoxelISMC->GetVoxelInstancePointAtIndex(InInstanceIndex, false), false);
	if (SampleData.HasMesh())
	{
		FTransform NewInstanceTransform;
		VoxelISMC->GetInstanceTransform(SampleData.SMI_Index, NewInstanceTransform);

		FVector AttachmentVector = EATAttachmentDirection_Utils::CreateVectorFromAttachmentDirections(SampleData.AttachmentDirections);
		NewInstanceTransform.SetRotation(AttachmentVector.ToOrientationQuat());
		NewInstanceTransform.SetScale3D(FVector(0.1f + AttachmentVector.Length(), 0.5f, 0.5f));
		VoxelISMC->UpdateInstanceTransform(SampleData.SMI_Index, NewInstanceTransform);
	}
}

void AATVoxelChunk::HandleDebugInstancesStabilityValues()
{
	if (!DebugInstancesStablityValues_QueuedIndices.IsEmpty())
	{
		for (int32 SampleQueuedInstanceIndex : DebugInstancesStablityValues_QueuedIndices)
		{
			DebugInstanceStabilityValue(SampleQueuedInstanceIndex);
		}
		DebugInstancesStablityValues_QueuedIndices.Empty();
		VoxelISMC->MarkRenderStateDirty();
	}
}

void AATVoxelChunk::DebugInstanceStabilityValue(int32 InInstanceIndex)
{
	const FVoxelInstanceData& SampleData = VoxelISMC->GetVoxelInstanceDataAtPoint(VoxelISMC->GetVoxelInstancePointAtIndex(InInstanceIndex, false), false);
	if (SampleData.HasMesh())
	{
		VoxelISMC->SetCustomDataValue(SampleData.SMI_Index, DebugVoxelCustomData_Stability, SampleData.Stability, false);
	}
}

void AATVoxelChunk::HandleDebugInstancesHealthValues()
{
	if (!DebugInstancesHealthValues_QueuedIndices.IsEmpty())
	{
		for (int32 SampleQueuedInstanceIndex : DebugInstancesHealthValues_QueuedIndices)
		{
			DebugInstanceHealthValue(SampleQueuedInstanceIndex);
		}
		DebugInstancesHealthValues_QueuedIndices.Empty();
		VoxelISMC->MarkRenderStateDirty();
	}
}

void AATVoxelChunk::DebugInstanceHealthValue(int32 InInstanceIndex)
{
	const FVoxelInstanceData& SampleData = VoxelISMC->GetVoxelInstanceDataAtPoint(VoxelISMC->GetVoxelInstancePointAtIndex(InInstanceIndex, false), false);
	if (SampleData.HasMesh())
	{
		VoxelISMC->SetCustomDataValue(SampleData.SMI_Index, DebugVoxelCustomData_Health, SampleData.Health, false);
	}
}

void AATVoxelChunk::BP_CollectDataForGameplayDebugger_Implementation(APlayerController* ForPlayerController, FVoxelChunkDebugData& InOutData) const
{
	// Common
	InOutData.ChunkHighlightTransform = FTransform(FRotator::ZeroRotator, GetChunkCenterWorldLocation(), FVector((float)ChunkSize * VoxelBaseSize * 0.5f));

	InOutData.Label = GetActorLabel();
	InOutData.LabelColor = FColor::MakeRandomSeededColor(GetActorGuid()[0]);

	InOutData.CommonEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Instances Num"), VoxelISMC->GetLocalPoint_To_InstanceData_Map().Num()));
	InOutData.CommonEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Chunk Size"), ChunkSize));
	InOutData.CommonEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Voxel Base Size"), VoxelBaseSize));
	InOutData.CommonEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Max Updates per Second"), MaxUpdatesPerSecond));

	// Attachments
	InOutData.AttachmentsEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Pending Updates Num"), AttachmentUpdates.GetPendingPointsConstArray().Num()));
	InOutData.AttachmentsEntries.Add(FVoxelChunkDebugData_Entry(TEXT("This Tick Updates Num"), AttachmentUpdates.GetThisTickSelectedPointsConstArray().Num()));

	// Stability
	InOutData.StabilityEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Pending Updates Num"), StabilityUpdates.GetPendingPointsConstArray().Num()));
	InOutData.StabilityEntries.Add(FVoxelChunkDebugData_Entry(TEXT("This Tick Updates Num"), StabilityUpdates.GetThisTickSelectedPointsConstArray().Num()));

	// Instance under cursor
	AScWPlayerController* ScWPlayerController = Cast<AScWPlayerController>(ForPlayerController);

	FHitResult ScreenCenterHitResult;
	ScWPlayerController->GetHitResultUnderScreenCenter(TraceTypeQuery_Visibility, false, ScreenCenterHitResult);

	UInstancedStaticMeshComponent* TargetISMC = Cast<UInstancedStaticMeshComponent>(ScreenCenterHitResult.GetComponent());
	if (TargetISMC == VoxelISMC)
	{
		int32 TargetInstanceIndex = ScreenCenterHitResult.Item;
		const FIntVector& TargetPoint = VoxelISMC->GetVoxelInstancePointAtIndex(TargetInstanceIndex);
		InOutData.InstanceLabel = FString::Printf(TEXT("Looking at Voxel at %s, instance index %d"), *TargetPoint.ToString(), TargetInstanceIndex);
		
		const FVoxelInstanceData& TargetData = VoxelISMC->GetVoxelInstanceDataAtPoint(TargetPoint);
		if (TargetData.IsTypeDataValid())
		{
			InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Type Data"), TargetData.TypeData.GetName()));
			InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Health"), TargetData.Health));
			InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Stability"), TargetData.Stability));
			InOutData.InstanceEntries.Add(FVoxelChunkDebugData_Entry(TEXT("Attachment Directions"), EATAttachmentDirection_Utils::CreateStringFromAttachmentDirections(TargetData.AttachmentDirections)));
		}
		InOutData.InstanceHighlightTransform = FTransform(FRotator::ZeroRotator, GetVoxelCenterWorldLocation(TargetPoint), FVector(VoxelBaseSize * 0.5f));
	}
}
//~ End Debug

#if DEBUG_VOXELS
	#pragma optimize("", on)
#endif // DEBUG_VOXELS
