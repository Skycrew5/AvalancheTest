// Scientific Ways

#include "World/ATVoxelNode.h"

#include "World/ATVoxelTree.h"
#include "World/ATVoxelISMC.h"
#include "World/ATTypes_World.h"

AATVoxelNode::AATVoxelNode(const FObjectInitializer& InObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	ChunkSize = 16;
	VoxelSize = 16.0f;
}

//~ Begin Initialize
void AATVoxelNode::PostInitializeComponents() // AActor
{
	Super::PostInitializeComponents();


}

void AATVoxelNode::OnConstruction(const FTransform& InTransform) // AActor
{
	Super::OnConstruction(InTransform);


}

void AATVoxelNode::BeginPlay() // AActor
{
	Super::BeginPlay();


}

void AATVoxelNode::Tick(float InDeltaSeconds) // AActor
{
	Super::Tick(InDeltaSeconds);


}

void AATVoxelNode::EndPlay(const EEndPlayReason::Type InReason) // AActor
{
	Super::EndPlay(InReason);


}
//~ End Initialize

//~ Begin Parent Tree
void AATVoxelNode::InitFromParentTree(AATVoxelTree* InTree, const FIntVector& InInsideParentLocalPoint)
{
	ensure(!ParentTree);
	ParentTree = InTree;
	RootTree = InTree->GetRootTree() ? InTree->GetRootTree() : InTree;

	InsideParentLocalPoint = InInsideParentLocalPoint;

	ChunkSize = InTree->ChunkSize;
	VoxelSize = InTree->VoxelSize;

	Cache_UpdateVoxelBounds();
}

FIntVector AATVoxelNode::GetGlobalOffset() const
{
	if (ParentTree)
	{
		return ParentTree->GetGlobalOffset() + ParentTree->GetSubTreeNodeSizeInVoxels() * InsideParentLocalPoint;
	}
	else
	{
		return FIntVector::ZeroValue;
	}
}
//~ End Parent Tree

//~ Begin Voxel Chunks
AATVoxelChunk* AATVoxelNode::GetVoxelChunkAtPoint(const FIntVector& InThisNodeScopePoint) const
{
	ensureReturn(false, nullptr);
}
//~ End Voxel Chunks

//~ Begin Voxel Bounds
FIntVector AATVoxelNode::GetNodeSizeInVoxels() const
{
	ensureReturn(false, FIntVector::ZeroValue);
}

bool AATVoxelNode::IsLocalPointInVoxelBounds(const FIntVector& InLocalPoint) const
{
	return (InLocalPoint.X >= 0 && InLocalPoint.X < Cache_NodeSizeInVoxels.X)
		&& (InLocalPoint.Y >= 0 && InLocalPoint.Y < Cache_NodeSizeInVoxels.Y)
		&& (InLocalPoint.Z >= 0 && InLocalPoint.Z < Cache_NodeSizeInVoxels.Z);
}

bool AATVoxelNode::IsGlobalPointInVoxelBounds(const FIntVector& InGlobalPoint) const
{
	return (InGlobalPoint.X >= Cache_VoxelBoundsStart.X && InGlobalPoint.X < Cache_VoxelBoundsEnd.X)
		&& (InGlobalPoint.Y >= Cache_VoxelBoundsStart.Y && InGlobalPoint.Y < Cache_VoxelBoundsEnd.Y)
		&& (InGlobalPoint.Z >= Cache_VoxelBoundsStart.Z && InGlobalPoint.Z < Cache_VoxelBoundsEnd.Z);
}

void AATVoxelNode::Cache_UpdateVoxelBounds()
{
	Cache_NodeSizeInVoxels = GetNodeSizeInVoxels();

	Cache_VoxelBoundsStart = GetGlobalOffset();
	Cache_VoxelBoundsEnd = Cache_VoxelBoundsStart + Cache_NodeSizeInVoxels;
}
//~ End Voxel Bounds

//~ Begin Voxel Components
UATVoxelISMC* AATVoxelNode::GetVoxelComponentAtPoint(const FIntVector& InThisNodeScopePoint) const
{
	ensureReturn(false, nullptr);
}
//~ End Voxel Components

//~ Begin Voxel Getters
FVoxelInstanceData& AATVoxelNode::GetVoxelInstanceDataAtPoint(const FIntVector& InThisNodeScopePoint, const bool bInChecked, const bool bInCanGetDataFromOtherNode) const
{
	if (bInCanGetDataFromOtherNode && !IsLocalPointInVoxelBounds(InThisNodeScopePoint) && RootTree)
	{
		return RootTree->GetVoxelInstanceDataAtPoint(GetGlobalOffset() + InThisNodeScopePoint, bInChecked, false);
	}
	else
	{
		if (UATVoxelISMC* RelevantVoxelComponent = GetVoxelComponentAtPoint(InThisNodeScopePoint))
		{
			return RelevantVoxelComponent->GetVoxelInstanceDataAtPoint(InThisNodeScopePoint, bInChecked);
		}
	}
	ensureReturn(false, FVoxelInstanceData::Invalid_NonConst);
}

bool AATVoxelNode::HasVoxelAtPoint(const FIntVector& InThisNodeScopePoint, const bool bInCanCheckOtherNode) const
{
	if (bInCanCheckOtherNode && !IsLocalPointInVoxelBounds(InThisNodeScopePoint))
	{
		return RootTree->HasVoxelAtPoint(GetGlobalOffset() + InThisNodeScopePoint, false);
	}
	else
	{
		if (UATVoxelISMC* RelevantVoxelComponent = GetVoxelComponentAtPoint(InThisNodeScopePoint))
		{
			return RelevantVoxelComponent->HasVoxelAtPoint(InThisNodeScopePoint);
		}
	}
	ensureReturn(false, false);
}

int32 AATVoxelNode::GetVoxelNeighborsNumAtLocalPoint(const FIntVector& InThisNodeScopePoint) const
{
	int32 OutNum = 0;

	if (HasVoxelAtPoint(InThisNodeScopePoint + FIntVector(1, 0, 0))) OutNum += 1;
	if (HasVoxelAtPoint(InThisNodeScopePoint + FIntVector(-1, 0, 0))) OutNum += 1;
	if (HasVoxelAtPoint(InThisNodeScopePoint + FIntVector(0, 1, 0))) OutNum += 1;
	if (HasVoxelAtPoint(InThisNodeScopePoint + FIntVector(0, -1, 0))) OutNum += 1;
	if (HasVoxelAtPoint(InThisNodeScopePoint + FIntVector(0, 0, 1))) OutNum += 1;
	if (HasVoxelAtPoint(InThisNodeScopePoint + FIntVector(0, 0, -1))) OutNum += 1;
	return OutNum;
}

bool AATVoxelNode::IsVoxelAtPointFullyClosed(const FIntVector& InThisNodeScopePoint) const
{
	if (!HasVoxelAtPoint(InThisNodeScopePoint + FIntVector(1, 0, 0)))
	{
		return false;
	}
	if (!HasVoxelAtPoint(InThisNodeScopePoint + FIntVector(-1, 0, 0)))
	{
		return false;
	}
	if (!HasVoxelAtPoint(InThisNodeScopePoint + FIntVector(0, 1, 0)))
	{
		return false;
	}
	if (!HasVoxelAtPoint(InThisNodeScopePoint + FIntVector(0, -1, 0)))
	{
		return false;
	}
	if (!HasVoxelAtPoint(InThisNodeScopePoint + FIntVector(0, 0, 1)))
	{
		return false;
	}
	if (!HasVoxelAtPoint(InThisNodeScopePoint + FIntVector(0, 0, -1)))
	{
		return false;
	}
	return true;
}

void AATVoxelNode::GetAllVoxelPointsInRadius(const FIntVector& InThisNodeScopeCenterPoint, int32 InRadius, TArray<FIntVector>& OutPoints) const
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
				OutPoints.Add(InThisNodeScopeCenterPoint + FIntVector(OffsetX, OffsetY, OffsetZ));
			}
		}
	}
}
//~ End Voxel Getters

//~ Begin Voxel Setters
bool AATVoxelNode::SetVoxelAtLocalPoint(const FIntVector& InThisNodeScopePoint, const UATVoxelTypeData* InTypeData, const bool bInForced, const bool bInCanSetInOtherNode)
{
	if (bInCanSetInOtherNode && !IsLocalPointInVoxelBounds(InThisNodeScopePoint) && RootTree)
	{
		return RootTree->SetVoxelAtLocalPoint(GetGlobalOffset() + InThisNodeScopePoint, InTypeData, bInForced, false);
	}
	else
	{
		if (UATVoxelISMC* RelevantVoxelComponent = GetVoxelComponentAtPoint(InThisNodeScopePoint))
		{
			return RelevantVoxelComponent->SetVoxelAtPoint(InThisNodeScopePoint, InTypeData, bInForced);
		}
	}
	ensureReturn(false, false);
}

bool AATVoxelNode::SetVoxelsAtLocalPoints(const TArray<FIntVector>& InThisNodeScopePoints, const UATVoxelTypeData* InTypeData, const bool bInForced, const bool bInCanSetInOtherNode)
{
	bool bAnySet = false;

	for (const FIntVector& SamplePoint : InThisNodeScopePoints)
	{
		bAnySet |= SetVoxelAtLocalPoint(SamplePoint, InTypeData, bInForced, bInCanSetInOtherNode);
	}
	return bAnySet;
}

bool AATVoxelNode::BreakVoxelAtLocalPoint(const FIntVector& InThisNodeScopePoint, const bool bInForced, const bool bInNotify, const bool bInCanBreakInOtherNode)
{
	if (bInCanBreakInOtherNode && !IsLocalPointInVoxelBounds(InThisNodeScopePoint) && RootTree)
	{
		return RootTree->BreakVoxelAtLocalPoint(GetGlobalOffset() + InThisNodeScopePoint, bInForced, bInNotify, false);
	}
	else
	{
		if (UATVoxelISMC* RelevantVoxelComponent = GetVoxelComponentAtPoint(InThisNodeScopePoint))
		{
			return RelevantVoxelComponent->BreakVoxelAtPoint(InThisNodeScopePoint, bInForced, bInNotify);
		}
	}
	ensureReturn(false, false);
}

bool AATVoxelNode::BreakVoxelsAtLocalPoints(const TArray<FIntVector>& InThisNodeScopePoints, const bool bInForced, const bool bInNotify, const bool bInCanBreakInOtherNode)
{
	bool bAnyBroken = false;

	for (const FIntVector& SamplePoint : InThisNodeScopePoints)
	{
		bAnyBroken |= BreakVoxelAtLocalPoint(SamplePoint, bInForced, bInNotify);
	}
	return bAnyBroken;
}
//~ End Voxel Setters
