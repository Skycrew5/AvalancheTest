// Scientific Ways


#include "World/ATVoxelPMC.h"

#include "World/ATVoxelChunk.h"
#include "World/ATVoxelTypeData.h"
#include "World/ATWorldFunctionLibrary.h"

#if DEBUG_VOXELS
#pragma optimize("", off)
#endif // DEBUG_VOXELS

//FDelegateHandle UATVoxelPMC::InstanceIndexUpdatedDelegateHandle = FDelegateHandle();

UATVoxelPMC::UATVoxelPMC(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;

	//bSupportRemoveAtSwap = true;

	SetCollisionProfileName(TEXT("Voxel"));

	


	bDebugStabilityValues = true;
	bDebugHealthValues = true;

	DebugVoxelCustomData_Stability = 0;
	DebugVoxelCustomData_Health = 3;
}

//~ Begin Initialize
void UATVoxelPMC::OnRegister() // UActorComponent
{
	//if (!InstanceIndexUpdatedDelegateHandle.IsValid())
	//{
	//	InstanceIndexUpdatedDelegateHandle = FInstancedStaticMeshDelegates::OnInstanceIndexUpdated.AddStatic(&UATVoxelPMC::Static_OnISMInstanceIndicesUpdated);
	//}
	Super::OnRegister();
}

void UATVoxelPMC::BeginPlay() // UActorComponent
{
	Super::BeginPlay();

	float VoxelSize = OwnerChunk->GetVoxelSize();

	VoxelSidePlaneDataArray.Add(FPlaneData(0, 1, 0, 0));
	VoxelSidePlaneDataArray.Add(FPlaneData(0, 1, 0, VoxelSize));
	VoxelSidePlaneDataArray.Add(FPlaneData(1, 0, 0, 0));
	VoxelSidePlaneDataArray.Add(FPlaneData(1, 0, 0, VoxelSize));
	VoxelSidePlaneDataArray.Add(FPlaneData(0, 0, 1, VoxelSize));
	VoxelSidePlaneDataArray.Add(FPlaneData(0, 0, 1, 0));

	EdgeSectionPlaneDataArray.Add(FPlaneData(0, -1, 1, VoxelSize * (1 - VoxelTypeData->SectionsDepth)));
	EdgeSectionPlaneDataArray.Add(FPlaneData(0, -1, 1, -VoxelSize * (1 - VoxelTypeData->SectionsDepth)));
	EdgeSectionPlaneDataArray.Add(FPlaneData(0, 1, 1, VoxelSize * VoxelTypeData->SectionsDepth));
	EdgeSectionPlaneDataArray.Add(FPlaneData(0, 1, 1, -VoxelSize * (VoxelTypeData->SectionsDepth - 2)));
	EdgeSectionPlaneDataArray.Add(FPlaneData(-1, 0, 1, VoxelSize * (1 - VoxelTypeData->SectionsDepth)));
	EdgeSectionPlaneDataArray.Add(FPlaneData(-1, 0, 1, -VoxelSize * (1 - VoxelTypeData->SectionsDepth)));
	EdgeSectionPlaneDataArray.Add(FPlaneData(1, 0, 1, VoxelSize * VoxelTypeData->SectionsDepth));
	EdgeSectionPlaneDataArray.Add(FPlaneData(1, 0, 1, VoxelSize * (2 - VoxelTypeData->SectionsDepth)));
	EdgeSectionPlaneDataArray.Add(FPlaneData(1, 1, 0, VoxelSize * VoxelTypeData->SectionsDepth));
	EdgeSectionPlaneDataArray.Add(FPlaneData(1, 1, 0, VoxelSize * (2 - VoxelTypeData->SectionsDepth)));
	EdgeSectionPlaneDataArray.Add(FPlaneData(1, -1, 0, VoxelSize * (1 - VoxelTypeData->SectionsDepth)));
	EdgeSectionPlaneDataArray.Add(FPlaneData(1, -1, 0, -VoxelSize * (1 - VoxelTypeData->SectionsDepth)));

	AngleSectionPlaneDataArray.Add(FPlaneData(1, 1, -1, -VoxelSize * (1 - VoxelTypeData->SectionsDepth)));
	AngleSectionPlaneDataArray.Add(FPlaneData(1, -1, -1, -VoxelSize * (2 - VoxelTypeData->SectionsDepth)));
	AngleSectionPlaneDataArray.Add(FPlaneData(-1, -1, -1, -VoxelSize * (3 - VoxelTypeData->SectionsDepth)));
	AngleSectionPlaneDataArray.Add(FPlaneData(-1, 1, -1, -VoxelSize * (2 - VoxelTypeData->SectionsDepth)));
	AngleSectionPlaneDataArray.Add(FPlaneData(-1, - 1, -1, -VoxelSize * VoxelTypeData->SectionsDepth));
	AngleSectionPlaneDataArray.Add(FPlaneData(-1, 1, -1, VoxelSize * (1 - VoxelTypeData->SectionsDepth)));
	AngleSectionPlaneDataArray.Add(FPlaneData(1, 1, -1, VoxelSize * (2 - VoxelTypeData->SectionsDepth)));
	AngleSectionPlaneDataArray.Add(FPlaneData(1, -1, -1, VoxelSize * (1 - VoxelTypeData->SectionsDepth)));

	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(0, 2), 8); 
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(2, 0), 8);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(0, 3), 10); 
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(3, 0), 10);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(0, 4), 0);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(4, 0), 0);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(0, 5), 2);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(5, 0), 2);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(1, 2), 11);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(2, 1), 11);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(1, 3), 9);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(3, 1), 9);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(1, 4), 3);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(4, 1), 3);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(1, 5), 1);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(5, 1), 1);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(2, 4), 4);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(4, 2), 4);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(2, 5), 6);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(5, 2), 6);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(3, 4), 7);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(4, 3), 7);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(3, 5), 5);
	SideIndexesPair_to_EdgeIndex_Map.Add(FIntPoint(5, 3), 5);

	UnattachedSideIndexesPairsArray.Add(FIntPoint(0, 2));
	UnattachedSideIndexesPairsArray.Add(FIntPoint(0, 3));
	UnattachedSideIndexesPairsArray.Add(FIntPoint(0, 4));
	UnattachedSideIndexesPairsArray.Add(FIntPoint(0, 5));
	UnattachedSideIndexesPairsArray.Add(FIntPoint(1, 2));
	UnattachedSideIndexesPairsArray.Add(FIntPoint(1, 3));
	UnattachedSideIndexesPairsArray.Add(FIntPoint(1, 4));
	UnattachedSideIndexesPairsArray.Add(FIntPoint(1, 5));
	UnattachedSideIndexesPairsArray.Add(FIntPoint(2, 4));
	UnattachedSideIndexesPairsArray.Add(FIntPoint(2, 5));
	UnattachedSideIndexesPairsArray.Add(FIntPoint(3, 4));
	UnattachedSideIndexesPairsArray.Add(FIntPoint(3, 5));

	SideIndex_to_IntersectableSideIndexesArray_Map.Add(0, { 2, 4, 3, 5 });
	SideIndex_to_IntersectableSideIndexesArray_Map.Add(1, { 2, 5, 3, 4 });
	SideIndex_to_IntersectableSideIndexesArray_Map.Add(2, { 0, 5, 1, 4 });
	SideIndex_to_IntersectableSideIndexesArray_Map.Add(3, { 1, 5, 0, 4 });
	SideIndex_to_IntersectableSideIndexesArray_Map.Add(4, { 0, 2, 1, 3 });
	SideIndex_to_IntersectableSideIndexesArray_Map.Add(5, { 0, 3, 1, 2 });

	SideIndex_to_AngleIndexesArray_Map.Add(0, { 0, 3, 7, 4 });
	SideIndex_to_AngleIndexesArray_Map.Add(1, { 1, 5, 6, 2 });
	SideIndex_to_AngleIndexesArray_Map.Add(2, { 0, 4, 5, 1 });
	SideIndex_to_AngleIndexesArray_Map.Add(3, { 2, 6, 7, 3 });
	SideIndex_to_AngleIndexesArray_Map.Add(4, { 0, 1, 2, 3 });
	SideIndex_to_AngleIndexesArray_Map.Add(5, { 4, 7, 6, 5 });

	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(0, 0), 0);
	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(0, 3), 10);
	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(0, 7), 2);
	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(0, 4), 8);

	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(1, 1), 11);
	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(1, 5), 1);
	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(1, 6), 9);
	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(1, 2), 3);

	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(2, 0), 8);
	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(2, 4), 6);
	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(2, 5), 11);
	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(2, 1), 4);

	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(3, 2), 9);
	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(3, 6), 5);
	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(3, 7), 10);
	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(3, 3), 7);

	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(4, 0), 4);
	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(4, 1), 3);
	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(4, 2), 7);
	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(4, 3), 0);

	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(5, 4), 2);
	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(5, 5), 5);
	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(5, 6), 1);
	SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Add(FIntPoint(5, 7), 6);


	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(0, 0), 8);
	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(0, 3), 0);
	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(0, 7), 10);
	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(0, 4), 2);

	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(1, 1), 3);
	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(1, 5), 11);
	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(1, 6), 1);
	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(1, 2), 9);

	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(2, 0), 4);
	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(2, 4), 8);
	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(2, 5), 6);
	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(2, 1), 11);

	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(3, 2), 7);
	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(3, 6), 9);
	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(3, 7), 5);
	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(3, 3), 10);

	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(4, 0), 0);
	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(4, 1), 4);
	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(4, 2), 3);
	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(4, 3), 7);

	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(5, 4), 6);
	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(5, 5), 2);
	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(5, 6), 5);
	SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Add(FIntPoint(5, 7), 1);

	EdgeIndex_to_IntersectableSideIndexesArray_Map.Add(0, { 0, 2, 4, 3 });
	EdgeIndex_to_IntersectableSideIndexesArray_Map.Add(1, { 1, 2, 5, 3 });
	EdgeIndex_to_IntersectableSideIndexesArray_Map.Add(2, { 0, 3, 5, 2 });
	EdgeIndex_to_IntersectableSideIndexesArray_Map.Add(3, { 4, 2, 1, 3 });
	EdgeIndex_to_IntersectableSideIndexesArray_Map.Add(4, { 2, 1, 4, 0 });
	EdgeIndex_to_IntersectableSideIndexesArray_Map.Add(5, { 3, 1, 5, 0 });
	EdgeIndex_to_IntersectableSideIndexesArray_Map.Add(6, { 2, 0, 5, 1 });
	EdgeIndex_to_IntersectableSideIndexesArray_Map.Add(7, { 4, 1, 3, 0 });
	EdgeIndex_to_IntersectableSideIndexesArray_Map.Add(8, { 0, 5, 2, 4 });
	EdgeIndex_to_IntersectableSideIndexesArray_Map.Add(9, { 1, 5, 3, 4 });
	EdgeIndex_to_IntersectableSideIndexesArray_Map.Add(10, { 0, 4, 3, 5 });
	EdgeIndex_to_IntersectableSideIndexesArray_Map.Add(11, { 2, 5, 1, 4 });

	if (VoxelTypeData->SectionsDepth <= 0.45f)
	{
		EdgeIndex_to_IntersectableEdgeIndexesArray_Map.Add(0, { 4, 7, 8, 10 });
		EdgeIndex_to_IntersectableEdgeIndexesArray_Map.Add(1, { 5, 6, 9, 11 });
		EdgeIndex_to_IntersectableEdgeIndexesArray_Map.Add(2, { 5, 6, 8, 10 });
		EdgeIndex_to_IntersectableEdgeIndexesArray_Map.Add(3, { 4, 7, 9, 11 });

		EdgeIndex_to_IntersectableEdgeIndexesArray_Map.Add(4, { 0, 3, 8, 11 });
		EdgeIndex_to_IntersectableEdgeIndexesArray_Map.Add(5, { 1, 2, 9, 10 });
		EdgeIndex_to_IntersectableEdgeIndexesArray_Map.Add(6, { 1, 2, 8, 11 });
		EdgeIndex_to_IntersectableEdgeIndexesArray_Map.Add(7, { 0, 3, 9, 10 });

		EdgeIndex_to_IntersectableEdgeIndexesArray_Map.Add(8, { 0, 2, 4, 6 });
		EdgeIndex_to_IntersectableEdgeIndexesArray_Map.Add(9, { 1, 3, 5, 7 });
		EdgeIndex_to_IntersectableEdgeIndexesArray_Map.Add(10, { 0, 2, 5, 7 });
		EdgeIndex_to_IntersectableEdgeIndexesArray_Map.Add(11, { 1, 3, 4, 6 });
	}
	else
	{
		// Each edge section plane intersected by each other except parallel and by itself
	}

	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(0, 4), 0);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(4, 0), 0);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(0, 8), 0);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(8, 0), 0);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(4, 8), 0);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(8, 4), 0);

	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(3, 4), 1);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(4, 3), 1);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(3, 11), 1);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(11, 3), 1);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(4, 11), 1);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(11, 4), 1);

	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(3, 7), 2);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(7, 3), 2);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(3, 9), 2);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(9, 3), 2);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(7, 9), 2);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(9, 7), 2);

	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(0, 7), 3);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(7, 0), 3);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(0, 10), 3);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(10, 0), 3);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(7, 10), 3);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(10, 7), 3);

	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(2, 6), 4);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(6, 2), 4);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(2, 8), 4);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(8, 2), 4);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(6, 8), 4);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(8, 6), 4);

	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(1, 6), 5);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(6, 1), 5);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(1, 11), 5);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(11, 1), 5);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(6 ,11), 5);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(11, 6), 5);

	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(1, 5), 6);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(5, 1), 6);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(1, 9), 6);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(9, 1), 6);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(5, 9), 6);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(9, 5), 6);

	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(2, 5), 7);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(5, 2), 7);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(2, 10), 7);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(10, 2), 7);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(5, 10), 7);
	EdgeIndexesPair_to_AngleIndex_Map.Add(FIntPoint(10, 5), 7);

	AngleIndex_to_SideIndexes_Map.Add(0, { 0, 2, 4 });
	AngleIndex_to_SideIndexes_Map.Add(1, { 1, 4, 2 });
	AngleIndex_to_SideIndexes_Map.Add(2, { 1, 3, 4 });
	AngleIndex_to_SideIndexes_Map.Add(3, { 0, 4, 3 });
	AngleIndex_to_SideIndexes_Map.Add(4, { 0, 5, 2 });
	AngleIndex_to_SideIndexes_Map.Add(5, { 2, 5, 1 });
	AngleIndex_to_SideIndexes_Map.Add(6, { 1, 5, 3 });
	AngleIndex_to_SideIndexes_Map.Add(7, { 0, 3, 5 });

	AngleIndex_to_EdgeIndexes_Map.Add(0, { 0, 8, 4 });
	AngleIndex_to_EdgeIndexes_Map.Add(1, { 4, 11, 3 });
	AngleIndex_to_EdgeIndexes_Map.Add(2, { 3, 9, 7 });
	AngleIndex_to_EdgeIndexes_Map.Add(3, { 7, 10, 0 });
	AngleIndex_to_EdgeIndexes_Map.Add(4, { 8, 2, 6 });
	AngleIndex_to_EdgeIndexes_Map.Add(5, { 11, 6, 1 });
	AngleIndex_to_EdgeIndexes_Map.Add(6, { 9, 1, 5 });
	AngleIndex_to_EdgeIndexes_Map.Add(7, { 10, 5, 2 });

	SideIndex_to_MeshNormal_Map.Add(0, FVector(0, -1, 0));
	SideIndex_to_MeshNormal_Map.Add(1, FVector(0, 1, 0));
	SideIndex_to_MeshNormal_Map.Add(2, FVector(-1, 0, 0));
	SideIndex_to_MeshNormal_Map.Add(3, FVector(1, 0, 0));
	SideIndex_to_MeshNormal_Map.Add(4, FVector(0, 0, 1));
	SideIndex_to_MeshNormal_Map.Add(5, FVector(0, 0, -1));

	EdgeIndex_to_MeshNormal_Map.Add(0, FVector(0, -1, 1));
	EdgeIndex_to_MeshNormal_Map.Add(1, FVector(0, 1, -1));
	EdgeIndex_to_MeshNormal_Map.Add(2, FVector(0, -1, -1));
	EdgeIndex_to_MeshNormal_Map.Add(3, FVector(0, 1, 1));
	EdgeIndex_to_MeshNormal_Map.Add(4, FVector(-1, 0, 1));
	EdgeIndex_to_MeshNormal_Map.Add(5, FVector(1, 0, -1));
	EdgeIndex_to_MeshNormal_Map.Add(6, FVector(-1, 0, -1));
	EdgeIndex_to_MeshNormal_Map.Add(7, FVector(1, 0, 1));
	EdgeIndex_to_MeshNormal_Map.Add(8, FVector(-1, -1, 0));
	EdgeIndex_to_MeshNormal_Map.Add(9, FVector(1, 1, 0));
	EdgeIndex_to_MeshNormal_Map.Add(10, FVector(1, -1, 0));
	EdgeIndex_to_MeshNormal_Map.Add(11, FVector(-1, 1, 0));

	AngleIndex_to_MeshNormal_Map.Add(0, FVector(-1, -1, 1));
	AngleIndex_to_MeshNormal_Map.Add(1, FVector(-1, 1, 1));
	AngleIndex_to_MeshNormal_Map.Add(2, FVector(1, 1, 1));
	AngleIndex_to_MeshNormal_Map.Add(3, FVector(1, -1, 1));
	AngleIndex_to_MeshNormal_Map.Add(4, FVector(-1, -1, -1));
	AngleIndex_to_MeshNormal_Map.Add(5, FVector(-1, 1, -1));
	AngleIndex_to_MeshNormal_Map.Add(6, FVector(1, 1, -1));
	AngleIndex_to_MeshNormal_Map.Add(7, FVector(1, -1, -1));

	/*TArray<TArray<int32>> UnattachedSideIndexesCombinationsArray;
	TArray<TArray<int32>> UnattachedEdgeIndexesCombinationsArray;

	GenerateUnattachedSideIndexesCombinations(UnattachedSideIndexesCombinationsArray);
	GenerateUnattachedEdgeIndexesCombinations(UnattachedEdgeIndexesCombinationsArray);

	int32 UnattachedSideIndexesCombinationIndex = 0;

	int32 UnattachedEdgeIndexesCombinationIndex = 0;

	for (TArray<int32>& UnattachedSideIndexesCombination : UnattachedSideIndexesCombinationsArray)
	{
		for (TArray<int32>& UnattachedEdgeIndexesCombination : UnattachedEdgeIndexesCombinationsArray)
		{
			CreateVoxelMeshTemplate(UnattachedSideIndexesCombination, UnattachedEdgeIndexesCombination);

			UnattachedEdgeIndexesCombinationIndex++;
		}
		UnattachedSideIndexesCombinationIndex++;
	}*/
}

void UATVoxelPMC::EndPlay(const EEndPlayReason::Type InReason) // UActorComponent
{
	Super::EndPlay(InReason);


}

void UATVoxelPMC::BP_InitComponent_Implementation(AATVoxelChunk* InOwnerChunk, const UATVoxelTypeData* InTypeData)
{
	ensureReturn(InOwnerChunk);
	OwnerChunk = InOwnerChunk;

	ensureReturn(InTypeData);
	VoxelTypeData = InTypeData;

	//ensureReturn(InTypeData->StaticMesh);
	//SetStaticMesh(InTypeData->StaticMesh);

	SetMaterial(0, InTypeData->SidesMeshOverrideMaterial);
	SetMaterial(1, InTypeData->SectionsMeshOverrideMaterial);
	SetMaterial(2, InTypeData->SectionsMeshOverrideMaterial);
	SetMaterial(3, InTypeData->SectionsMeshOverrideMaterial);

	//SetNumCustomDataFloats(bDebugHealthValues ? 2 : (bDebugStabilityValues ? 1 : 0));
}
//~ End Initialize

//~ Begin Getters
bool UATVoxelPMC::HasVoxelOfThisTypeAtPoint(const FIntVector& InPoint) const
{
	ensureReturn(VoxelTypeData, false);
	return GetVoxelInstanceDataAtPoint(InPoint, false).TypeData == VoxelTypeData;
}

bool UATVoxelPMC::HasVoxelOfAnyTypeAtPoint(const FIntVector& InPoint) const
{
	return GetVoxelInstanceDataAtPoint(InPoint, false).IsTypeDataValid();
}

bool UATVoxelPMC::CouldCreateMeshAtPoint(const FIntVector& InPoint) const
{
	return PossibleMeshPointsSet.Contains(InPoint);
}

FVoxelInstanceData& UATVoxelPMC::GetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const bool bInChecked) const
{
	ensureReturn(OwnerChunk, const_cast<FVoxelInstanceData&>(FVoxelInstanceData::Invalid));
	return OwnerChunk->GetVoxelInstanceDataAtPoint(InPoint, bInChecked);
}

//FIntVector UATVoxelPMC::GetPointOfMeshIndex(int32 InMeshIndex, const bool bInChecked) const
//{
//	if (!IsValidInstance(InMeshIndex))
//	{
//		ensure(!bInChecked);
//		return FIntVector::ZeroValue;
//	}
//	FTransform TargetWorldTransform;
//	GetInstanceTransform(InMeshIndex, TargetWorldTransform, true);
//	return UATWorldFunctionLibrary::WorldLocation_To_Point3D(TargetWorldTransform.GetLocation(), GetVoxelSize());
//}

bool UATVoxelPMC::IsVoxelSidesClosed(const FIntVector& InPoint) const
{
	ensureReturn(OwnerChunk, false);

	for (const FIntVector& SampleOffset : FATVoxelUtils::SideOffsets)
	{
		FIntVector SamplePoint = InPoint + SampleOffset;

		if (OwnerChunk->IsPointInsideTree(SamplePoint) && !HasVoxelOfAnyTypeAtPoint(SamplePoint)) return false;
	}
	return true;
}

bool UATVoxelPMC::IsVoxelEdgesClosed(const FIntVector& InPoint) const
{
	ensureReturn(OwnerChunk, false);

	for (const FIntVector& SampleOffset : FATVoxelUtils::EdgeOffsets)
	{
		FIntVector SamplePoint = InPoint + SampleOffset;

		if (OwnerChunk->IsPointInsideTree(SamplePoint) && !HasVoxelOfAnyTypeAtPoint(SamplePoint)) return false;
	}
	return true;
}

float UATVoxelPMC::GetVoxelSize() const
{
	ensureReturn(OwnerChunk, 16.0f);
	return OwnerChunk->GetVoxelSize();
}
//~ End Getters

//~ Begin Setters
void UATVoxelPMC::HandleSetVoxelInstanceDataAtPoint(const FIntVector& InPoint, const FVoxelInstanceData& InVoxelInstanceData)
{
	if (InVoxelInstanceData.IsTypeDataValid()) PossibleMeshPointsSet.Add(InPoint);

	else PossibleMeshPointsSet.Remove(InPoint);

	QueuePointForVisibilityUpdate(InPoint);
	TryQueuePointForDebugUpdate(InPoint);
}

bool UATVoxelPMC::HandleBreakVoxelAtPoint(const FIntVector& InPoint, const FVoxelBreakData& InBreakData)
{
	HandleSetVoxelInstanceDataAtPoint(InPoint, FVoxelInstanceData::Invalid);

	if (InBreakData.bNotify)
	{
		ensureReturn(OwnerChunk, true);
		OwnerChunk->OnBreakVoxelAtPoint.Broadcast(this, InPoint, InBreakData);
	}
	return true;
}

//bool UATVoxelPMC::HandleMeshInstanceIndexRelocated(int32 InPrevIndex, int32 InNewIndex)
//{
//	if (IsValidInstance(InPrevIndex))
//	{
//		FIntVector NewPrevIndexPoint = GetPointOfMeshIndex(InPrevIndex, true);
//		Point_To_MeshIndex_Map.Add(NewPrevIndexPoint, InPrevIndex);
//	}
//	FIntVector NewNewIndexPoint = GetPointOfMeshIndex(InNewIndex, true);
//	Point_To_MeshIndex_Map.Add(NewNewIndexPoint, InNewIndex);
//	return true;
//}
//~ End Setters

//~ Begin Data
void UATVoxelPMC::HandleUpdates()
{
	if (bEnableVoxelsVisibilityUpdates)
	{
		UpdateVoxelsVisibilityState();
	}
	UpdateVoxelsDebugState();

	INC_MEMORY_STAT_BY(STAT_VoxelComponents_Point_To_MeshIndex_Map, Point_To_MeshIndex_Map.GetAllocatedSize());
}

void UATVoxelPMC::QueuePointForVisibilityUpdate(const FIntVector& InPoint, const bool bInQueueNeighborsToo)
{
	QueuedVisibilityUpdatePoints.Add(InPoint);

	if (bInQueueNeighborsToo)
	{
		QueuedVisibilityUpdatePoints.Add(InPoint + FIntVector(1, 0, 0));
		QueuedVisibilityUpdatePoints.Add(InPoint + FIntVector(-1, 0, 0));
		QueuedVisibilityUpdatePoints.Add(InPoint + FIntVector(0, 1, 0));
		QueuedVisibilityUpdatePoints.Add(InPoint + FIntVector(0, -1, 0));
		QueuedVisibilityUpdatePoints.Add(InPoint + FIntVector(0, 0, 1));
		QueuedVisibilityUpdatePoints.Add(InPoint + FIntVector(0, 0, -1));
	}
}

void UATVoxelPMC::UpdateVoxelsVisibilityState(const bool bInIgnoreTimeBugdet)
{
	if (QueuedVisibilityUpdatePoints.IsEmpty()) return;

	TArray<FIntVector> VisibleVoxelPointsArray;
	//TArray<FIntVector> PointsToAdd;

	TArray<int32> MeshInstancesToRemove;
	TArray<FIntVector> PointsToRemove;

	ensureReturn(OwnerChunk);
	while (/*(bInIgnoreTimeBugdet || !OwnerChunk->IsThisTickUpdatesTimeBudgetExceeded()) && */!QueuedVisibilityUpdatePoints.IsEmpty())
	{
		FIntVector SamplePoint = QueuedVisibilityUpdatePoints.Pop();

		if (CouldCreateMeshAtPoint(SamplePoint))
		{
			//if (MeshedPointsSet.Contains(SamplePoint)) // Has mesh...
			//{
				if (IsVoxelSidesClosed(SamplePoint) && IsVoxelEdgesClosed(SamplePoint)) // ...but doesn't need anymore
				{
					//MeshInstancesToRemove.Add(*SampleIndexPtr);
					//PointsToRemove.Add(SamplePoint);
				}
			//}
			//else // Does not have mesh...
			//{
				else //if (!IsVoxelSidesClosed(SamplePoint)) // ...but needs now
				{
					VisibleVoxelPointsArray.Add(SamplePoint);
					//PointsToAdd.Add(SamplePoint);
				}
			//}
		}
		else // Point is empty or not of this component
		{
			//if (int32* SampleIndexPtr = Point_To_MeshIndex_Map.Find(SamplePoint)) // But has mesh to remove
			//{
			//	MeshInstancesToRemove.Add(*SampleIndexPtr);
			//	PointsToRemove.Add(SamplePoint);
			//}
		}
	}
	// Remove
	//ensure(MeshInstancesToRemove.Num() == PointsToRemove.Num());

	//for (const FIntVector& SamplePoint : PointsToRemove)
	//{
		//Point_To_MeshIndex_Map.Remove(SamplePoint);
	//}
	//RemoveInstances(MeshInstancesToRemove);

	// Add
	CreateMesh(VisibleVoxelPointsArray);
	//ensure(AddedIndices.Num() == PointsToAdd.Num());
}
//~ End Data

//~ Begin Meshes
void UATVoxelPMC::Static_OnISMInstanceIndicesUpdated(UInstancedStaticMeshComponent* InUpdatedComponent, TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData> InIndexUpdates)
{
	/*ensureReturn(InUpdatedComponent);

	if (UATVoxelPMC* TargetVoxelComponent = Cast<UATVoxelPMC>(InUpdatedComponent))
	{
		TargetVoxelComponent->HandleInstanceIndicesUpdates(InIndexUpdates);
	}*/
}

void UATVoxelPMC::HandleInstanceIndicesUpdates(const TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData>& InIndexUpdates)
{
	//for (const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData& SampleUpdate : InIndexUpdates)
	//{
	//	bool bQueueDebug = false;

	//	switch (SampleUpdate.Type)
	//	{
	//	case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Added:
	//	{
	//		FIntVector TargetPoint = GetPointOfMeshIndex(SampleUpdate.Index);

	//		ensure(!Point_To_MeshIndex_Map.Contains(TargetPoint));
	//		Point_To_MeshIndex_Map.Add(TargetPoint, SampleUpdate.Index);

	//		bQueueDebug = true;
	//		break;
	//	}
	//	case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Removed:
	//	case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Cleared:
	//	case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Destroyed:
	//	{
	//		//FIntVector TargetPoint = GetPointOfMeshIndex(SampleUpdate.Index);
	//		//Point_To_MeshIndex_Map.Remove(TargetPoint);
	//		break;
	//	}
	//	case FInstancedStaticMeshDelegates::EInstanceIndexUpdateType::Relocated:
	//	{
	//		/*HandleMeshInstanceIndexRelocated(SampleUpdate.OldIndex, SampleUpdate.Index);
	//		bQueueDebug = true;
	//		break;*/
	//	}
	//	}
	//	if (bQueueDebug)
	//	{
	//		TryQueueMeshIndexForDebugUpdate(SampleUpdate.Index);
	//	}
	//}
}

void UATVoxelPMC::GenerateUnattachedSideIndexesCombinations(TArray<TArray<int32>>& OutCombinationsArray)
{
	TArray<int32> AllSideIndexesArray = { 0, 1, 2, 3, 4, 5 };

	TArray<int32> TempCombination;

	for (int32 i = 1; i <= 6; ++i)
	{
		TempCombination.Reset();

		GenerateCombinations(AllSideIndexesArray, 0, 0, i, TempCombination, OutCombinationsArray);
	}
}

void UATVoxelPMC::GenerateUnattachedEdgeIndexesCombinations(TArray<TArray<int32>>& OutCombinationsArray)
{
	TArray<int32> AllEdgeIndexesArray = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

	TArray<int32> TempCombination;

	for (int32 i = 1; i <= 12; ++i)
	{
		TempCombination.Reset();

		GenerateCombinations(AllEdgeIndexesArray, 0, 0, i, TempCombination, OutCombinationsArray);
	}
}

void UATVoxelPMC::GenerateCombinations(const TArray<int32>& InElements, int32 InStartIndex, int32 InCurrentLength, int32 InMaxLength, TArray<int32>& InCurrentCombination, TArray<TArray<int32>>& OutCombinationsArray)
{
	if (InCurrentCombination.Num() == InMaxLength)
	{
		OutCombinationsArray.Add(InCurrentCombination);

		//GenerateVoxelMeshTemplate(InCurrentCombination);

		return;
	}

	for (int32 i = InStartIndex; i < InElements.Num(); ++i)
	{
		InCurrentCombination.Add(InElements[i]);

		GenerateCombinations(InElements, i + 1, InCurrentLength + 1, InMaxLength, InCurrentCombination, OutCombinationsArray);

		InCurrentCombination.Pop();
	}
}

void UATVoxelPMC::CreateVoxelMeshTemplate(TArray<int32>& InUnattachedSideIndexesArray, TArray<int32>& InUnattachedEdgeIndexesArray)
{
	TArray<TArray<int32>> UnattachedSideAndEdgeIndexesArraysArray = { InUnattachedSideIndexesArray, InUnattachedEdgeIndexesArray };

	if (UnattachedSideAndEdgeIndexes_to_VoxelSideIndexToBelongVerticesMap_Map.Contains(UnattachedSideAndEdgeIndexesArraysArray)) return;

	TMap<int32, TArray<FVector>>& VoxelSideIndex_to_BelongVerticesArray_Map = UnattachedSideAndEdgeIndexes_to_VoxelSideIndexToBelongVerticesMap_Map.Add(UnattachedSideAndEdgeIndexesArraysArray, TMap<int32, TArray<FVector>>());

	TMap<int32, TArray<FVector>>& EdgeIndex_to_BelongVerticesArray_Map = UnattachedSideAndEdgeIndexes_to_VoxelEdgeIndexToBelongVerticesMap_Map.Add(UnattachedSideAndEdgeIndexesArraysArray, TMap<int32, TArray<FVector>>());

	TMap<int32, TArray<FVector>>& AngleIndex_to_BelongVerticesArray_Map = UnattachedSideAndEdgeIndexes_to_AngleIndexToBelongVerticesMap_Map.Add(UnattachedSideAndEdgeIndexesArraysArray, TMap<int32, TArray<FVector>>());

	TArray<TPair<int32, TArray<FVector>>>* AngleIndexToVerticesPairsArray = nullptr;

	if (InUnattachedSideIndexesArray.Num() > 2) AngleIndexToVerticesPairsArray = &UnattachedSideAndEdgeIndexes_to_AngleIndexToVerticesPairsArray_Map.Add(UnattachedSideAndEdgeIndexesArraysArray, TArray<TPair<int32, TArray<FVector>>>());

	FVector CurrentIntersectionPoint = FVector::ZeroVector;

	int32 CurrentIntersectionPointIndex = NULL;
	
	EVoxelPlaneType FirstAdjacentPlaneType;

	int32 FirstAdjacentPlaneDataIndex = -1;

	EVoxelPlaneType SecondAdjacentPlaneType;

	int32 SecondAdjacentPlaneDataIndex = -1;

	int32 FirstPreviousEdgeIndex = -1;

	int32 SecondPreviousEdgeIndex = -1;

	int32 CurrentAngleIndex = NULL;

	int32 CurrentAngleAttachedSidesNum = NULL;

	bool bCurrentSideAngleVerticesAdded = false;

	if (InUnattachedSideIndexesArray.Num() == 3 && InUnattachedSideIndexesArray[0] == 1 && InUnattachedSideIndexesArray[1] == 3 && InUnattachedSideIndexesArray[2] == 4)
	{
		bCurrentSideAngleVerticesAdded = false;
	}

	for (int32& UnattachedSideIndex : InUnattachedSideIndexesArray)
	{
		TArray<FVector>& UnattachedSideVerticesArray = VoxelSideIndex_to_BelongVerticesArray_Map.Add(UnattachedSideIndex, TArray<FVector>());

		if (TArray<int32>* IntersectableSideIndexesArray = SideIndex_to_IntersectableSideIndexesArray_Map.Find(UnattachedSideIndex))
		{
			for (int32 SideAngleOrderIndex = 0; SideAngleOrderIndex < 4; SideAngleOrderIndex++)
			{
				FirstAdjacentPlaneDataIndex = -1;

				SecondAdjacentPlaneDataIndex = -1;

				FirstAdjacentPlaneType = EVoxelPlaneType::EdgeSection;

				SecondAdjacentPlaneType = EVoxelPlaneType::EdgeSection;

				CurrentAngleIndex = (*SideIndex_to_AngleIndexesArray_Map.Find(UnattachedSideIndex))[SideAngleOrderIndex];

				if (int32* FirstAdjacentEdgeIndex = SideAndAngleIndexes_to_FirstAdjacentEdgeIndex_Map.Find(FIntPoint(UnattachedSideIndex, CurrentAngleIndex)))
				{
					if (TArray<int32>* EdgeIntersectablelSideIndexesArray = EdgeIndex_to_IntersectableSideIndexesArray_Map.Find(*FirstAdjacentEdgeIndex))
					{
						if ((*EdgeIntersectablelSideIndexesArray)[0] != UnattachedSideIndex && InUnattachedSideIndexesArray.Contains((*EdgeIntersectablelSideIndexesArray)[0])) FirstAdjacentPlaneDataIndex = *FirstAdjacentEdgeIndex;

						else if ((*EdgeIntersectablelSideIndexesArray)[2] != UnattachedSideIndex && InUnattachedSideIndexesArray.Contains((*EdgeIntersectablelSideIndexesArray)[2])) FirstAdjacentPlaneDataIndex = *FirstAdjacentEdgeIndex;

						else
						{
							FirstAdjacentPlaneType = EVoxelPlaneType::Side;

							if ((*EdgeIntersectablelSideIndexesArray)[0] != UnattachedSideIndex) FirstAdjacentPlaneDataIndex = (*EdgeIntersectablelSideIndexesArray)[0];

							else FirstAdjacentPlaneDataIndex = (*EdgeIntersectablelSideIndexesArray)[2];
						}
					}
				}
				
				if (int32* SecondAdjacentEdgeIndex = SideAndAngleIndexes_to_SecondAdjacentEdgeIndex_Map.Find(FIntPoint(UnattachedSideIndex, CurrentAngleIndex)))
				{
					if (TArray<int32>* EdgeIntersectablelSideIndexesArray = EdgeIndex_to_IntersectableSideIndexesArray_Map.Find(*SecondAdjacentEdgeIndex))
					{
						if ((*EdgeIntersectablelSideIndexesArray)[0] != UnattachedSideIndex && InUnattachedSideIndexesArray.Contains((*EdgeIntersectablelSideIndexesArray)[0])) SecondAdjacentPlaneDataIndex = *SecondAdjacentEdgeIndex;

						else if ((*EdgeIntersectablelSideIndexesArray)[2] != UnattachedSideIndex && InUnattachedSideIndexesArray.Contains((*EdgeIntersectablelSideIndexesArray)[2])) SecondAdjacentPlaneDataIndex = *SecondAdjacentEdgeIndex;

						else
						{
							SecondAdjacentPlaneType = EVoxelPlaneType::Side;

							if ((*EdgeIntersectablelSideIndexesArray)[0] != UnattachedSideIndex) SecondAdjacentPlaneDataIndex = (*EdgeIntersectablelSideIndexesArray)[0];

							else SecondAdjacentPlaneDataIndex = (*EdgeIntersectablelSideIndexesArray)[2];
						}
					}
				}

				bCurrentSideAngleVerticesAdded = false;

				if (FirstAdjacentPlaneType == EVoxelPlaneType::Side && SecondAdjacentPlaneType == EVoxelPlaneType::Side)
				{
					if (TArray<int32>* CurrentAngleEdgeIndexesArray = AngleIndex_to_EdgeIndexes_Map.Find(CurrentAngleIndex))
					{
						if (std::all_of(CurrentAngleEdgeIndexesArray->begin(), CurrentAngleEdgeIndexesArray->end(), [&InUnattachedEdgeIndexesArray](int32& EdgeIndex) { return InUnattachedEdgeIndexesArray.Contains(EdgeIndex); }))
						{
							if (TArray<int32>* AngleSideIndexesArray = AngleIndex_to_SideIndexes_Map.Find(CurrentAngleIndex))
							{
								for (int32 i = 0; i < 3; i++)
								{
									if (TryIntersectThreePlanes(AngleSectionPlaneDataArray[CurrentAngleIndex],
										VoxelSidePlaneDataArray[(*AngleSideIndexesArray)[i]],
										VoxelSidePlaneDataArray[(*AngleSideIndexesArray)[i == 2 ? 0 : i + 1]],
										CurrentIntersectionPoint))
									{
										if ((*AngleSideIndexesArray)[i] == UnattachedSideIndex || (*AngleSideIndexesArray)[i == 2 ? 0 : i + 1] == UnattachedSideIndex)
										{
											UnattachedSideVerticesArray.Add(CurrentIntersectionPoint);

											bCurrentSideAngleVerticesAdded = true;
										}

										if (TArray<FVector>* AngleVerticesArray = AngleIndex_to_BelongVerticesArray_Map.Find(CurrentAngleIndex))
										{
											AngleVerticesArray->Add(CurrentIntersectionPoint);
										}
										else AngleIndex_to_BelongVerticesArray_Map.Add(CurrentAngleIndex, { CurrentIntersectionPoint });
									}
								}

								UnattachedSideVerticesArray.SwapMemory(UnattachedSideVerticesArray.Num() - 2, UnattachedSideVerticesArray.Num() - 1);
							}
						}
					}
				}
				
				if (!bCurrentSideAngleVerticesAdded 
				&& TryIntersectThreePlanes(VoxelSidePlaneDataArray[UnattachedSideIndex],
					FirstAdjacentPlaneType == EVoxelPlaneType::Side ? VoxelSidePlaneDataArray[FirstAdjacentPlaneDataIndex] : EdgeSectionPlaneDataArray[FirstAdjacentPlaneDataIndex],
					SecondAdjacentPlaneType == EVoxelPlaneType::Side ? VoxelSidePlaneDataArray[SecondAdjacentPlaneDataIndex] : EdgeSectionPlaneDataArray[SecondAdjacentPlaneDataIndex],
					CurrentIntersectionPoint))
				{
					UnattachedSideVerticesArray.Add(CurrentIntersectionPoint);

					if (FirstAdjacentPlaneType == EVoxelPlaneType::EdgeSection)
					{
						if (TArray<FVector>* EdgeSectionPlaneBelongVerticesArray = EdgeIndex_to_BelongVerticesArray_Map.Find(FirstAdjacentPlaneDataIndex))
						{
							if (EdgeSectionPlaneBelongVerticesArray->Num() % 2 != 0 && FirstAdjacentPlaneDataIndex != SecondPreviousEdgeIndex)
							{
								EdgeSectionPlaneBelongVerticesArray->Insert(CurrentIntersectionPoint, EdgeSectionPlaneBelongVerticesArray->Num() == 3 ? 2 : 0);
							}
							else EdgeSectionPlaneBelongVerticesArray->Add(CurrentIntersectionPoint);
						}
						else EdgeIndex_to_BelongVerticesArray_Map.Add(FirstAdjacentPlaneDataIndex, { CurrentIntersectionPoint });
					}

					if (SecondAdjacentPlaneType == EVoxelPlaneType::EdgeSection)
					{
						if (TArray<FVector>* EdgeSectionPlaneBelongVerticesArray = EdgeIndex_to_BelongVerticesArray_Map.Find(SecondAdjacentPlaneDataIndex))
						{
							if (EdgeSectionPlaneBelongVerticesArray->Num() % 2 != 0 && SecondAdjacentPlaneDataIndex != FirstPreviousEdgeIndex)
							{
								EdgeSectionPlaneBelongVerticesArray->Insert(CurrentIntersectionPoint, EdgeSectionPlaneBelongVerticesArray->Num() == 3 ? 2 : 0);
							}
							else EdgeSectionPlaneBelongVerticesArray->Add(CurrentIntersectionPoint);
						}
						else EdgeIndex_to_BelongVerticesArray_Map.Add(SecondAdjacentPlaneDataIndex, { CurrentIntersectionPoint });
					}

					if (FirstAdjacentPlaneType == EVoxelPlaneType::EdgeSection) FirstPreviousEdgeIndex = FirstAdjacentPlaneDataIndex;

					else if (FirstPreviousEdgeIndex != -1) FirstPreviousEdgeIndex = -1;

					if (FirstAdjacentPlaneType == EVoxelPlaneType::EdgeSection) SecondPreviousEdgeIndex = SecondAdjacentPlaneDataIndex;

					else if (SecondPreviousEdgeIndex != -1) SecondPreviousEdgeIndex = -1;

					if (AngleIndexToVerticesPairsArray && FirstAdjacentPlaneType == EVoxelPlaneType::EdgeSection && SecondAdjacentPlaneType == EVoxelPlaneType::EdgeSection)
					{
						if (int32* AngleIndex = EdgeIndexesPair_to_AngleIndex_Map.Find(FIntPoint(FirstAdjacentPlaneDataIndex, SecondAdjacentPlaneDataIndex)))
						{
							if (TPair<int32, TArray<FVector>>* AngleIndexToVerticesPair = AngleIndexToVerticesPairsArray->FindByPredicate([AngleIndex](TPair<int32, TArray<FVector>>& InAngleIndexToVerticesPair) { return InAngleIndexToVerticesPair.Key == *AngleIndex; }))
							{
								if (TArray<int32>* AngleSectionAdjacentSideIndexesArray = AngleIndex_to_SideIndexes_Map.Find(*AngleIndex))
								{
									AngleIndexToVerticesPair->Value[AngleSectionAdjacentSideIndexesArray->IndexOfByKey(UnattachedSideIndex)] = CurrentIntersectionPoint;
								}
							}
							else
							{
								AngleIndexToVerticesPair = &(*AngleIndexToVerticesPairsArray)[AngleIndexToVerticesPairsArray->AddDefaulted()];

								AngleIndexToVerticesPair->Key = *AngleIndex;

								AngleIndexToVerticesPair->Value.Init(FVector::ZeroVector, 3);

								if (TArray<int32>* AngleSectionAdjacentSideIndexesArray = AngleIndex_to_SideIndexes_Map.Find(*AngleIndex))
								{
									AngleIndexToVerticesPair->Value[AngleSectionAdjacentSideIndexesArray->IndexOfByKey(UnattachedSideIndex)] = CurrentIntersectionPoint;
								}
							}
						}
					}
				}

				CurrentAngleAttachedSidesNum = 0;

				TArray<FVector>* AngleVerticesArray = AngleIndex_to_BelongVerticesArray_Map.Find(CurrentAngleIndex);

				if (!AngleVerticesArray)
				{
					if (TArray<int32>* CurrentAngleEdgeIndexesArray = AngleIndex_to_EdgeIndexes_Map.Find(CurrentAngleIndex))
					{
						if (std::all_of(CurrentAngleEdgeIndexesArray->begin(), CurrentAngleEdgeIndexesArray->end(), [&InUnattachedEdgeIndexesArray](int32& EdgeIndex) { return InUnattachedEdgeIndexesArray.Contains(EdgeIndex); }))
						{
							if (TArray<int32>* AngleSideIndexesArray = AngleIndex_to_SideIndexes_Map.Find(CurrentAngleIndex))
							{
								for (int32 i = 0; i < 3; i++)
								{
									if (!InUnattachedSideIndexesArray.Contains((*AngleSideIndexesArray)[i]))
									{
										CurrentAngleAttachedSidesNum++;

										if (CurrentAngleAttachedSidesNum > 1)
										{
											AngleIndex_to_BelongVerticesArray_Map.Add(CurrentAngleIndex, TArray<FVector>());

											for (int32 IntersectableSideIndex = 0; IntersectableSideIndex < 3; IntersectableSideIndex++)
											{
												if (TryIntersectThreePlanes(AngleSectionPlaneDataArray[CurrentAngleIndex],
													VoxelSidePlaneDataArray[(*AngleSideIndexesArray)[IntersectableSideIndex]],
													VoxelSidePlaneDataArray[(*AngleSideIndexesArray)[IntersectableSideIndex == 2 ? 0 : IntersectableSideIndex + 1]],
													CurrentIntersectionPoint))
												{
													AngleVerticesArray->Add(CurrentIntersectionPoint);
												}
											}
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void UATVoxelPMC::CreateMesh(TArray<FIntVector>& InVisibleVoxelPointsArray)
{
	ClearAllMeshSections();

	FVector VoxelRelativeLocation = FVector::ZeroVector;

	TArray<int32> UnattachedSideIndexesArray;
	TArray<int32> UnattachedEdgeIndexesArray;

	TArray<TArray<int32>> UnattachedSideAndEdgeIndexesArraysArray;
	UnattachedSideAndEdgeIndexesArraysArray.AddDefaulted(2);

	TArray<FVector> SidesMeshVerticesArray;
	TArray<FVector> EdgeSectionPlanesMeshVerticesArray;
	TArray<FVector> UnintentionalAngleSectionsMeshVerticesArray;
	TArray<FVector> AngleSectionsMeshVerticesArray;
	
	TArray<int32> SidesMeshTriangleVertexIndexesArray;
	TArray<int32> EdgeSectionPlanesMeshTriangleVertexIndexesArray;
	TArray<int32> UnintentionalAngleSectionsMeshTriangleVertexIndexesArray;
	TArray<int32> AngleSectionsMeshTriangleVertexIndexesArray;

	TArray<FVector> SidesMeshNormalsArray;
	TArray<FVector> EdgeSectionsMeshNormalsArray;
	TArray<FVector> UnintentionalAngleSectionsMeshNormalsArray;
	TArray<FVector> AngleSectionsMeshNormalsArray;

	TArray<FVector2D> SidesMeshUVPointsArray;
	TArray<FVector2D> EdgeSectionsMeshUVPointsArray;
	TArray<FVector2D> UnintentionalAngleSectionsMeshUVPointsArray;
	TArray<FVector2D> AngleSectionsMeshUVPointsArray;

	TArray<FVector2D> DefaultUVPointsArray = {
		FVector2D(0, 0),
		FVector2D(1, 0),
		FVector2D(1, 1),
		FVector2D(0, 1)
	};

	TArray<FVector2D> ClockwiseDefaultUVPointsArray = {
		FVector2D(1, 0),
		FVector2D(0, 0),
		FVector2D(1, 1),
		FVector2D(0, 1)
	};

	TArray<FProcMeshTangent> SidesMeshTangentsArray;
	TArray<FProcMeshTangent> EdgeSectionsMeshTangentsArray;
	TArray<FProcMeshTangent> UnintentionalAngleSectionsMeshTangentsArray;
	TArray<FProcMeshTangent> AngleSectionsMeshTangentsArray;

	int32 AddedSidesNum = 0;

	int32 AddedEdgeSectionsNum = 0;

	int32 CurrentVoxelSideStartVertexIndex = 0;

	int32 CurrentVoxelFaceStartVertexIndex = 0;

	int32 CurrentEdgeSectionVertexIndex = 0;

	//bool bIsFirst

	for (FIntVector& VisibleVoxelPoint : InVisibleVoxelPointsArray)
	{
		VoxelRelativeLocation = UATWorldFunctionLibrary::Point3D_To_RelativeLocation(this, VisibleVoxelPoint);

		GetUnattachedSideIndexes(VisibleVoxelPoint, UnattachedSideIndexesArray);
		GetUnattachedEdgeIndexes(VisibleVoxelPoint, UnattachedEdgeIndexesArray);

		UnattachedSideIndexesArray.Sort();
		UnattachedEdgeIndexesArray.Sort();

		UnattachedSideAndEdgeIndexesArraysArray[0] = UnattachedSideIndexesArray;
		UnattachedSideAndEdgeIndexesArraysArray[1] = UnattachedEdgeIndexesArray;

		CreateVoxelMeshTemplate(UnattachedSideIndexesArray, UnattachedEdgeIndexesArray);

		if (TMap<int32, TArray<FVector>>* VoxelSideIndex_to_BelongVerticesArray_Map = UnattachedSideAndEdgeIndexes_to_VoxelSideIndexToBelongVerticesMap_Map.Find(UnattachedSideAndEdgeIndexesArraysArray))
		{
			for (int32& UnattachedSideIndex : UnattachedSideIndexesArray)
			{
				if (TArray<FVector>* VoxelSideVerticesArray = VoxelSideIndex_to_BelongVerticesArray_Map->Find(UnattachedSideIndex))
				{
					for (int32 SideVertexOrderIndex = 0; SideVertexOrderIndex < VoxelSideVerticesArray->Num(); SideVertexOrderIndex++)
					{
						SidesMeshVerticesArray.Add((*VoxelSideVerticesArray)[SideVertexOrderIndex] + VoxelRelativeLocation);

						if (FVector* SideMeshNormal = SideIndex_to_MeshNormal_Map.Find(UnattachedSideIndex)) SidesMeshNormalsArray.Add(*SideMeshNormal);
						
						//SidesMeshUVPointsArray.Add(DefaultUVPointsArray[SideVertexOrderIndex]);

						SidesMeshTangentsArray.Add(FProcMeshTangent(1, 0, 0));
					}

					for (int32 i = 1; i < VoxelSideVerticesArray->Num() - 1; i++)
					{
						SidesMeshTriangleVertexIndexesArray.Add(CurrentVoxelSideStartVertexIndex);
						SidesMeshTriangleVertexIndexesArray.Add(CurrentVoxelSideStartVertexIndex + i);
						SidesMeshTriangleVertexIndexesArray.Add(CurrentVoxelSideStartVertexIndex + i + 1);
					}

					CurrentVoxelSideStartVertexIndex += VoxelSideVerticesArray->Num();
				}
			}
		}

		if (TMap<int32, TArray<FVector>>* EdgeIndex_to_BelongVertices_Map = UnattachedSideAndEdgeIndexes_to_VoxelEdgeIndexToBelongVerticesMap_Map.Find(UnattachedSideAndEdgeIndexesArraysArray))
		{
			for (int32 i = 0; i < UnattachedSideIndexesPairsArray.Num(); i++)
			{
				if (UnattachedSideIndexesArray.Contains(UnattachedSideIndexesPairsArray[i].X) && UnattachedSideIndexesArray.Contains(UnattachedSideIndexesPairsArray[i].Y))
				{
					if (int32* EdgeIndex = SideIndexesPair_to_EdgeIndex_Map.Find(UnattachedSideIndexesPairsArray[i]))
					{
						if (TArray<FVector>* EdgeSectionPlaneVerticesArray = EdgeIndex_to_BelongVertices_Map->Find(*EdgeIndex)) // EdgeSectionPlaneVerticesArray - Contains Vertices in clock-wize allocation
						{
							EdgeSectionPlanesMeshVerticesArray += *EdgeSectionPlaneVerticesArray;

							CurrentEdgeSectionVertexIndex = 0;

							for (int j = EdgeSectionPlanesMeshVerticesArray.Num() - 4; j < EdgeSectionPlanesMeshVerticesArray.Num(); j++)
							{
								EdgeSectionPlanesMeshVerticesArray[j] += VoxelRelativeLocation;

								if (FVector* EdgeSectionMeshNormal = EdgeIndex_to_MeshNormal_Map.Find(*EdgeIndex)) EdgeSectionsMeshNormalsArray.Add(*EdgeSectionMeshNormal);

								//EdgeSectionsMeshUVPointsArray.Add(ClockwiseDefaultUVPointsArray[CurrentEdgeSectionVertexIndex]);

								EdgeSectionsMeshTangentsArray.Add(FProcMeshTangent(1, 0, 0));

								CurrentEdgeSectionVertexIndex++;
							}
						}
					}

					CurrentVoxelFaceStartVertexIndex = AddedEdgeSectionsNum * 4;

					EdgeSectionPlanesMeshTriangleVertexIndexesArray.Add(CurrentVoxelFaceStartVertexIndex + 1);
					EdgeSectionPlanesMeshTriangleVertexIndexesArray.Add(CurrentVoxelFaceStartVertexIndex + 0);
					EdgeSectionPlanesMeshTriangleVertexIndexesArray.Add(CurrentVoxelFaceStartVertexIndex + 2);

					EdgeSectionPlanesMeshTriangleVertexIndexesArray.Add(CurrentVoxelFaceStartVertexIndex + 2);
					EdgeSectionPlanesMeshTriangleVertexIndexesArray.Add(CurrentVoxelFaceStartVertexIndex + 0);
					EdgeSectionPlanesMeshTriangleVertexIndexesArray.Add(CurrentVoxelFaceStartVertexIndex + 3);

					AddedEdgeSectionsNum++;
				}
			}
		}

		if (TArray<TPair<int32, TArray<FVector>>>* AngleIndexToVerticesPairsArray = UnattachedSideAndEdgeIndexes_to_AngleIndexToVerticesPairsArray_Map.Find(UnattachedSideAndEdgeIndexesArraysArray))
		{
			for (TPair<int32, TArray<FVector>>& UnintentionalAngleIndexToVerticesPair : *AngleIndexToVerticesPairsArray)
			{
				if (!UnintentionalAngleIndexToVerticesPair.Value.IsEmpty())
				{
					UnintentionalAngleSectionsMeshVerticesArray += UnintentionalAngleIndexToVerticesPair.Value;

					for (int32 i = 0; i < 3; i++)
					{
						UnintentionalAngleSectionsMeshVerticesArray[UnintentionalAngleSectionsMeshTriangleVertexIndexesArray.Num()] += VoxelRelativeLocation;

						UnintentionalAngleSectionsMeshTriangleVertexIndexesArray.Add(UnintentionalAngleSectionsMeshTriangleVertexIndexesArray.Num());

						if (FVector* UnintentionalAngleSectionMeshNormal = AngleIndex_to_MeshNormal_Map.Find(UnintentionalAngleIndexToVerticesPair.Key)) UnintentionalAngleSectionsMeshNormalsArray.Add(*UnintentionalAngleSectionMeshNormal);

						//UnintentionalAngleSectionsMeshUVPointsArray.Add(DefaultUVPointsArray[i]);

						UnintentionalAngleSectionsMeshTangentsArray.Add(FProcMeshTangent(1, 0, 0));
					}
				}
			}
		}
	
		if (TMap<int32, TArray<FVector>>* AngleIndex_to_BelongVertices_Map = UnattachedSideAndEdgeIndexes_to_AngleIndexToBelongVerticesMap_Map.Find(UnattachedSideAndEdgeIndexesArraysArray))
		{
			for (TPair<int32, TArray<FVector>>& AngleIndexToVerticesPair : *AngleIndex_to_BelongVertices_Map)
			{
				AngleSectionsMeshVerticesArray += AngleIndexToVerticesPair.Value;

				for (int32 i = 0; i < 3; i++)
				{
					AngleSectionsMeshVerticesArray[AngleSectionsMeshTriangleVertexIndexesArray.Num()] += VoxelRelativeLocation;

					AngleSectionsMeshTriangleVertexIndexesArray.Add(AngleSectionsMeshTriangleVertexIndexesArray.Num());

					if (FVector* AngleSectionMeshNormal = AngleIndex_to_MeshNormal_Map.Find(AngleIndexToVerticesPair.Key)) AngleSectionsMeshNormalsArray.Add(*AngleSectionMeshNormal);

					//AngleSectionsMeshUVPointsArray.Add(DefaultUVPointsArray[i]);

					AngleSectionsMeshTangentsArray.Add(FProcMeshTangent(1, 0, 0));
				}
			}
		}
	}

	CreateMeshSection(0, SidesMeshVerticesArray, SidesMeshTriangleVertexIndexesArray, SidesMeshNormalsArray, SidesMeshUVPointsArray, TArray<FColor>(), SidesMeshTangentsArray, true);

	CreateMeshSection(1, EdgeSectionPlanesMeshVerticesArray, EdgeSectionPlanesMeshTriangleVertexIndexesArray, EdgeSectionsMeshNormalsArray, EdgeSectionsMeshUVPointsArray, TArray<FColor>(), EdgeSectionsMeshTangentsArray, true);

	CreateMeshSection(2, UnintentionalAngleSectionsMeshVerticesArray, UnintentionalAngleSectionsMeshTriangleVertexIndexesArray, UnintentionalAngleSectionsMeshNormalsArray, UnintentionalAngleSectionsMeshUVPointsArray, TArray<FColor>(), UnintentionalAngleSectionsMeshTangentsArray, true);

	CreateMeshSection(3, AngleSectionsMeshVerticesArray, AngleSectionsMeshTriangleVertexIndexesArray, AngleSectionsMeshNormalsArray, AngleSectionsMeshUVPointsArray, TArray<FColor>(), AngleSectionsMeshTangentsArray, true);
}

void UATVoxelPMC::GetUnattachedSideIndexes(FIntVector& InPoint, TArray<int32>& OutUnattachedSideIndexesArray)
{
	OutUnattachedSideIndexesArray.Reset();

	ensureReturn(OwnerChunk);

	for (int32 i = 0; i < FATVoxelUtils::SideOffsets.Num(); i++)
	{
		FIntVector SamplePoint = InPoint + FATVoxelUtils::SideOffsets[i];

		if (OwnerChunk->IsPointInsideTree(SamplePoint) && !HasVoxelOfAnyTypeAtPoint(SamplePoint)) OutUnattachedSideIndexesArray.Add(i);
	}
}

void UATVoxelPMC::GetUnattachedEdgeIndexes(FIntVector& InPoint, TArray<int32>& OutUnattachedEdgeIndexesArray)
{
	OutUnattachedEdgeIndexesArray.Reset();

	ensureReturn(OwnerChunk);

	for (int32 i = 0; i < FATVoxelUtils::EdgeOffsets.Num(); i++)
	{
		FIntVector SamplePoint = InPoint + FATVoxelUtils::EdgeOffsets[i];

		if (OwnerChunk->IsPointInsideTree(SamplePoint) && !HasVoxelOfAnyTypeAtPoint(SamplePoint)) OutUnattachedEdgeIndexesArray.Add(i);
	}
}

bool UATVoxelPMC::IsEdgeSectionPlanesIntersectable(const int32& InFirstEdgeSectionPlaneIndex, const int32& InSecondEdgeSectionPlaneIndex) const
{
	if (VoxelTypeData->SectionsDepth <= 0.25)
	{
		const TArray<int32>* IntersectableEdgeIndexesArray = EdgeIndex_to_IntersectableEdgeIndexesArray_Map.Find(InFirstEdgeSectionPlaneIndex);
		
		return IntersectableEdgeIndexesArray && IntersectableEdgeIndexesArray->Contains(InSecondEdgeSectionPlaneIndex);
	}
	else return InFirstEdgeSectionPlaneIndex % 2 == 0 ? InFirstEdgeSectionPlaneIndex + 1 != InSecondEdgeSectionPlaneIndex : InFirstEdgeSectionPlaneIndex - 1 != InSecondEdgeSectionPlaneIndex;
}

bool UATVoxelPMC::IsSidePlanesIntersectable(const int32& InFirstSidePlaneIndex, const int32& InSecondSidePlaneIndex) const
{
	return InFirstSidePlaneIndex % 2 == 0 ? InFirstSidePlaneIndex + 1 != InSecondSidePlaneIndex : InFirstSidePlaneIndex - 1 != InSecondSidePlaneIndex;
}

bool UATVoxelPMC::TryIntersectThreePlanes(const FPlaneData& InFirstPlane, const FPlaneData& InSecondPlane, const FPlaneData& InThirdPlane, FVector& OutPoint) const
{
	const float Det =
		InFirstPlane.B * (InSecondPlane.A * InThirdPlane.C - InThirdPlane.A * InSecondPlane.C)
		- InSecondPlane.B * (InFirstPlane.A * InThirdPlane.C - InThirdPlane.A * InFirstPlane.C)
		+ InThirdPlane.B * (InFirstPlane.A * InSecondPlane.C - InSecondPlane.A * InFirstPlane.C);

	if (FMath::Abs(Det) < SMALL_NUMBER) return false;

	const float InvDet = 1.f / Det;

	OutPoint.Y =
		-(InFirstPlane.D * (InSecondPlane.A * InThirdPlane.C - InThirdPlane.A * InSecondPlane.C)
		- InSecondPlane.D * (InFirstPlane.A * InThirdPlane.C - InThirdPlane.A * InFirstPlane.C)
		+ InThirdPlane.D * (InFirstPlane.A * InSecondPlane.C - InSecondPlane.A * InFirstPlane.C)) * -InvDet;

	OutPoint.X =
		-(InFirstPlane.B * (InSecondPlane.D * InThirdPlane.C - InThirdPlane.D * InSecondPlane.C)
		- InSecondPlane.B * (InFirstPlane.D * InThirdPlane.C - InThirdPlane.D * InFirstPlane.C)
		+ InThirdPlane.B * (InFirstPlane.D * InSecondPlane.C - InSecondPlane.D * InFirstPlane.C)) * -InvDet;

	OutPoint.Z =
		-(InFirstPlane.B * (InSecondPlane.A * InThirdPlane.D - InThirdPlane.A * InSecondPlane.D)
		- InSecondPlane.B * (InFirstPlane.A * InThirdPlane.D - InThirdPlane.A * InFirstPlane.D)
		+ InThirdPlane.B * (InFirstPlane.A * InSecondPlane.D - InSecondPlane.A * InFirstPlane.D)) * -InvDet;

	return true;
}
//~ End Meshes

//~ Begin Debug
void UATVoxelPMC::UpdateVoxelsDebugState()
{
	if (QueuedDebugUpdatePoints.IsEmpty())
	{
		return;
	}
	ensureReturn(OwnerChunk);
	while (!OwnerChunk->IsThisTickUpdatesTimeBudgetExceeded() && !QueuedDebugUpdatePoints.IsEmpty())
	{
		FIntVector SamplePoint = QueuedDebugUpdatePoints.Pop();

		Debug_UpdateStabilityValueAtPoint(SamplePoint);
		Debug_UpdateHealthValueAtPoint(SamplePoint);
	}
}

void UATVoxelPMC::Debug_UpdateStabilityValueAtPoint(const FIntVector& InPoint)
{
	/*if (bDebugStabilityValues && HasMeshAtPoint(InPoint))
	{
		const FVoxelInstanceData& VoxeInstanceData = GetVoxelInstanceDataAtPoint(InPoint, false);
		SetCustomDataValue(GetMeshIndexAtPoint(InPoint), DebugVoxelCustomData_Stability, VoxeInstanceData.Stability, true);
	}*/
}

void UATVoxelPMC::Debug_UpdateHealthValueAtPoint(const FIntVector& InPoint)
{
	/*if (bDebugHealthValues && HasMeshAtPoint(InPoint))
	{
		const FVoxelInstanceData& VoxeInstanceData = GetVoxelInstanceDataAtPoint(InPoint, false);
		SetCustomDataValue(GetMeshIndexAtPoint(InPoint), DebugVoxelCustomData_Health, VoxeInstanceData.Health, true);
	}*/
}
//~ End Debug

#if DEBUG_VOXELS
#pragma optimize("", on)
#endif // DEBUG_VOXELS

