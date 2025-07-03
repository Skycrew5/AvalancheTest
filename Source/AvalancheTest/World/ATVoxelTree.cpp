// Scientific Ways

#include "World/ATVoxelTree.h"

#include "World/ATVoxelChunk.h"

#if DEBUG_VOXELS
	#pragma optimize("", off)
#endif // DEBUG_VOXELS

AATVoxelTree::AATVoxelTree(const FObjectInitializer& InObjectInitializer)
{
	SubTreesInitData = {
		FVoxelSubTreeInitData(FIntVector(4, 4, 2)),
		FVoxelSubTreeInitData(FIntVector(2, 2, 2))
	};
	SubTreeClass = AATVoxelTree::StaticClass();

	ChunkClass = AATVoxelChunk::StaticClass();
}

//~ Begin Initialize
void AATVoxelTree::PostInitializeComponents() // AActor
{
	Super::PostInitializeComponents();

	
}

void AATVoxelTree::OnConstruction(const FTransform& InTransform) // AActor
{
	Super::OnConstruction(InTransform);


}

void AATVoxelTree::BeginPlay() // AActor
{
	Super::BeginPlay();

	InitSubTreeNodesRecursive();
}

void AATVoxelTree::Tick(float InDeltaSeconds) // AActor
{
	Super::Tick(InDeltaSeconds);


}

void AATVoxelTree::EndPlay(const EEndPlayReason::Type InReason) // AActor
{
	Super::EndPlay(InReason);


}
//~ End Initialize

//~ Begin Parent Tree
void AATVoxelTree::InitFromParentTree(AATVoxelTree* InTree, const FIntVector& InInsideParentLocalPoint) // AATVoxelNode
{
	Super::InitFromParentTree(InTree, InInsideParentLocalPoint);

	SubTreesInitData = TArray<FVoxelSubTreeInitData>(InTree->SubTreesInitData);
	SubTreesInitData.RemoveAt(0);

	ChunkClass = InTree->ChunkClass;
}
//~ End Parent Tree

//~ Begin SubTree
FIntVector AATVoxelTree::GetSubTreeNodeSizeInVoxels() const
{
	FIntVector TotalSizeInVoxels = FIntVector(ChunkSize);

	for (int32 SampleSubTreeIndex = 1; SampleSubTreeIndex < SubTreesInitData.Num(); ++SampleSubTreeIndex)
	{
		TotalSizeInVoxels *= SubTreesInitData[SampleSubTreeIndex].Size;
	}
	return TotalSizeInVoxels;
}

void AATVoxelTree::InitSubTreeNodesRecursive()
{
	UWorld* World = GetWorld();
	ensureReturn(World);

	FIntVector SubTreeNodeSize = GetSubTreeNodeSizeInVoxels();
	ensureReturn(!SubTreeNodeSize.IsZero());

	ensureReturn(!SubTreesInitData.IsEmpty());
	const FVoxelSubTreeInitData NextSubTreeInitData = SubTreesInitData[0];
	TSubclassOf<AATVoxelNode> NodeClass = nullptr;

	if (SubTreesInitData.Num() > 1)
	{
		NodeClass = SubTreeClass;
	}
	else
	{
		ensure(SubTreesInitData.Num() == 1);
		NodeClass = ChunkClass;
	}
	for (int32 SampleX = 0; SampleX < NextSubTreeInitData.Size.X; ++SampleX)
	{
		for (int32 SampleY = 0; SampleY < NextSubTreeInitData.Size.Y; ++SampleY)
		{
			for (int32 SampleZ = 0; SampleZ < NextSubTreeInitData.Size.Z; ++SampleZ)
			{
				FIntVector SamplePoint = FIntVector(SampleX, SampleY, SampleZ);

				FTransform SampleTransform = FTransform(FVector(SamplePoint * SubTreeNodeSize) * VoxelSize);
				AATVoxelNode* SampleNode = World->SpawnActorDeferred<AATVoxelNode>(
					NodeClass,
					SampleTransform,
					this,
					nullptr,
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
					ESpawnActorScaleMethod::MultiplyWithRoot
				);
				ensureContinue(SampleNode);

				ensureContinue(!TreeNodesMap.Contains(SamplePoint));
				TreeNodesMap.Add(SamplePoint, SampleNode);

				SampleNode->InitFromParentTree(this, SamplePoint);
				SampleNode->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
				SampleNode->FinishSpawning(SampleTransform);
			}
		}
	}
}
//~ End SubTree

//~ Begin Voxel Bounds
FIntVector AATVoxelTree::GetNodeSizeInVoxels() const // AATVoxelNode
{
	FIntVector TotalSizeInVoxels = FIntVector(ChunkSize);

	for (int32 SampleSubTreeIndex = 0; SampleSubTreeIndex < SubTreesInitData.Num(); ++SampleSubTreeIndex)
	{
		TotalSizeInVoxels *= SubTreesInitData[SampleSubTreeIndex].Size;
	}
	return TotalSizeInVoxels;
}
//~ End Voxel Bounds

//~ Begin Voxel Components
UATVoxelISMC* AATVoxelTree::GetVoxelComponentAtPoint(const FIntVector& InThisNodeScopePoint) const
{
	FIntVector TreeNodeSize = GetSubTreeNodeSizeInVoxels();
	FIntVector TreeNodeCoords = FIntVector(InThisNodeScopePoint.X / TreeNodeSize.X, InThisNodeScopePoint.Y / TreeNodeSize.Y, InThisNodeScopePoint.Z / TreeNodeSize.Z);

	if (AATVoxelNode* SampleNode = GetVoxelChunkAtPoint(InThisNodeScopePoint))
	{
		FIntVector TreeNodeLocalPoint = InThisNodeScopePoint - (TreeNodeCoords * TreeNodeSize);
		return SampleNode->GetVoxelComponentAtPoint(TreeNodeLocalPoint);
	}
	return nullptr;
}
//~ End Voxel Components

//~ Begin Voxel Chunks
AATVoxelChunk* AATVoxelTree::GetVoxelChunkAtPoint(const FIntVector& InThisNodeScopePoint) const
{
	FIntVector TreeNodeSize = GetSubTreeNodeSizeInVoxels();
	FIntVector TreeNodeCoords = FIntVector(InThisNodeScopePoint.X / TreeNodeSize.X, InThisNodeScopePoint.Y / TreeNodeSize.Y, InThisNodeScopePoint.Z / TreeNodeSize.Z);

	if (AATVoxelNode* SampleNode = TreeNodesMap.FindRef(TreeNodeCoords, nullptr))
	{
		FIntVector TreeNodeLocalPoint = InThisNodeScopePoint - (TreeNodeCoords * TreeNodeSize);
		return SampleNode->GetVoxelChunkAtPoint(TreeNodeLocalPoint);
	}
	return nullptr;
}
//~ End Voxel Chunks

#if DEBUG_VOXELS
	#pragma optimize("", on)
#endif // DEBUG_VOXELS
