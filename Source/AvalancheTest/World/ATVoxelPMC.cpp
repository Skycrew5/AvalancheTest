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

void UATVoxelPMC::GenerateCombinations(const TArray<int32>& Elements, int32 StartIndex, int32 CurrentLength, int32 MaxLength, TArray<int32>& CurrentCombination)
{
	if (CurrentCombination.Num() == MaxLength)
	{
		GenerateVoxelMeshPrototypeByUnattachedSideIndexes(CurrentCombination);

		return;
	}

	for (int32 i = StartIndex; i < Elements.Num(); ++i)
	{
		CurrentCombination.Add(Elements[i]);

		GenerateCombinations(Elements, i + 1, CurrentLength + 1, MaxLength, CurrentCombination);

		CurrentCombination.Pop(); 
	}
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

	EdgeSectionPlaneDataArray.Add(FPlaneData(0, -1, 1, VoxelSize * (1 - 2 * VoxelTypeData->SectionsDepth)));
	EdgeSectionPlaneDataArray.Add(FPlaneData(0, -1, 1, -VoxelSize * (1 - 2 * VoxelTypeData->SectionsDepth)));
	EdgeSectionPlaneDataArray.Add(FPlaneData(0, 1, 1, (VoxelSize * 2) * VoxelTypeData->SectionsDepth));
	EdgeSectionPlaneDataArray.Add(FPlaneData(0, 1, 1, -(VoxelSize * 2) * (VoxelTypeData->SectionsDepth - 1)));
	EdgeSectionPlaneDataArray.Add(FPlaneData(-1, 0, 1, VoxelSize * (1 - 2 * VoxelTypeData->SectionsDepth)));
	EdgeSectionPlaneDataArray.Add(FPlaneData(-1, 0, 1, -VoxelSize * (1 - 2 * VoxelTypeData->SectionsDepth)));
	EdgeSectionPlaneDataArray.Add(FPlaneData(1, 0, 1, (VoxelSize * 2) * VoxelTypeData->SectionsDepth));
	EdgeSectionPlaneDataArray.Add(FPlaneData(1, 0, 1, -(VoxelSize * 2) * (VoxelTypeData->SectionsDepth - 1)));
	EdgeSectionPlaneDataArray.Add(FPlaneData(1, 1, 0, (VoxelSize * 2) * VoxelTypeData->SectionsDepth));
	EdgeSectionPlaneDataArray.Add(FPlaneData(1, 1, 0, -(VoxelSize * 2) * (VoxelTypeData->SectionsDepth - 1)));
	EdgeSectionPlaneDataArray.Add(FPlaneData(1, -1, 0, VoxelSize * (1 - 2 * VoxelTypeData->SectionsDepth)));
	EdgeSectionPlaneDataArray.Add(FPlaneData(1, -1, 0, -VoxelSize * (1 - 2 * VoxelTypeData->SectionsDepth)));

	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(0, 2), 8); 
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(2, 0), 8);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(0, 3), 10); 
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(3, 0), 10);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(0, 4), 0);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(4, 0), 0);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(0, 5), 2);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(5, 0), 2);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(1, 2), 11);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(2, 1), 11);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(1, 3), 9);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(3, 1), 9);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(1, 4), 3);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(4, 1), 3);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(1, 5), 1);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(5, 1), 1);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(2, 4), 4);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(4, 2), 4);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(2, 5), 6);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(5, 2), 6);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(3, 4), 7);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(4, 3), 7);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(3, 5), 5);
	UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Add(FIntPoint(5, 3), 5);

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

	SidePlaneDataIndex_to_IntersectableSidePlaneDataIndexesArray_Map.Add(0, { 2, 4, 3, 5 });
	SidePlaneDataIndex_to_IntersectableSidePlaneDataIndexesArray_Map.Add(1, { 2, 5, 3, 4 });
	SidePlaneDataIndex_to_IntersectableSidePlaneDataIndexesArray_Map.Add(2, { 0, 5, 1, 4 });
	SidePlaneDataIndex_to_IntersectableSidePlaneDataIndexesArray_Map.Add(3, { 1, 5, 0, 4 });
	SidePlaneDataIndex_to_IntersectableSidePlaneDataIndexesArray_Map.Add(4, { 0, 2, 1, 3 });
	SidePlaneDataIndex_to_IntersectableSidePlaneDataIndexesArray_Map.Add(5, { 0, 3, 1, 2 });

	if (VoxelTypeData->SectionsDepth <= 0.5) // To make less intersections to find vertices
	{
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(0, { 0, 2, 4, 3 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(1, { 2, 5, 3, 1 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(2, { 0, 2, 5, 3 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(3, { 2, 1, 3, 4 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(4, { 0, 2, 1, 4 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(5, { 0, 3, 1, 5 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(6, { 0, 2, 1, 5 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(7, { 0, 4, 1, 3 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(8, { 0, 4, 2, 5 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(9, { 4, 1, 5, 3 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(10, { 0, 4, 3, 5 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(11, { 2, 4, 1, 5 });
	}
	else
	{
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(0, { 1, 3, 5, 2 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(1, { 3, 4, 2, 0 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(2, { 4, 3, 1, 2 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(3, { 3, 5, 2, 0 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(4, { 5, 1, 3, 0 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(5, { 4, 1, 2, 0 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(6, { 3, 1, 4, 0 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(7, { 5, 1, 2, 0 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(8, { 3, 5, 1, 4 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(9, { 5, 2, 4, 0 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(10, { 5, 1, 4, 2 });
		EdgeSectionPlaneDataIndex_to_IntersectableVoxelSidePlaneDataIndexesArray_Map.Add(11, { 5, 3, 4, 0 });
	}

	if (VoxelTypeData->SectionsDepth <= 0.25)
	{
		EdgeSectionPlaneDataIndex_to_IntersectableEdgeSectionPlaneDataIndexesArray_Map.Add(0, { 4, 7, 8, 10 });
		EdgeSectionPlaneDataIndex_to_IntersectableEdgeSectionPlaneDataIndexesArray_Map.Add(1, { 5, 6, 9, 11 });
		EdgeSectionPlaneDataIndex_to_IntersectableEdgeSectionPlaneDataIndexesArray_Map.Add(2, { 5, 6, 8, 10 });
		EdgeSectionPlaneDataIndex_to_IntersectableEdgeSectionPlaneDataIndexesArray_Map.Add(3, { 4, 7, 9, 11 });

		EdgeSectionPlaneDataIndex_to_IntersectableEdgeSectionPlaneDataIndexesArray_Map.Add(4, { 0, 3, 8, 11 });
		EdgeSectionPlaneDataIndex_to_IntersectableEdgeSectionPlaneDataIndexesArray_Map.Add(5, { 1, 2, 9, 10 });
		EdgeSectionPlaneDataIndex_to_IntersectableEdgeSectionPlaneDataIndexesArray_Map.Add(6, { 1, 2, 8, 11 });
		EdgeSectionPlaneDataIndex_to_IntersectableEdgeSectionPlaneDataIndexesArray_Map.Add(7, { 0, 3, 9, 10 });

		EdgeSectionPlaneDataIndex_to_IntersectableEdgeSectionPlaneDataIndexesArray_Map.Add(8, { 0, 2, 4, 6 });
		EdgeSectionPlaneDataIndex_to_IntersectableEdgeSectionPlaneDataIndexesArray_Map.Add(9, { 1, 3, 5, 7 });
		EdgeSectionPlaneDataIndex_to_IntersectableEdgeSectionPlaneDataIndexesArray_Map.Add(10, { 0, 2, 5, 7 });
		EdgeSectionPlaneDataIndex_to_IntersectableEdgeSectionPlaneDataIndexesArray_Map.Add(11, { 1, 3, 4, 6 });
	}
	else
	{
		// Each edge section plane intersected by each other except parallel and by itself
	}

	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(0, 4), 0);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(4, 0), 0);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(0, 8), 0);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(8, 0), 0);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(4, 8), 0);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(8, 4), 0);

	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(3, 4), 1);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(4, 3), 1);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(3, 11), 1);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(11, 3), 1);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(4, 11), 1);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(11, 4), 1);

	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(3, 7), 2);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(7, 3), 2);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(3, 9), 2);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(9, 3), 2);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(7, 9), 2);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(9, 7), 2);

	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(0, 7), 3);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(7, 0), 3);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(0, 10), 3);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(10, 0), 3);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(7, 10), 3);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(10, 7), 3);

	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(2, 6), 4);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(6, 2), 4);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(2, 8), 4);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(8, 2), 4);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(6, 8), 4);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(8, 6), 4);

	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(1, 6), 5);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(6, 1), 5);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(1, 11), 5);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(11, 1), 5);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(6 ,11), 5);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(11, 6), 5);

	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(1, 5), 6);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(5, 1), 6);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(1, 9), 6);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(9, 1), 6);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(5, 9), 6);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(9, 5), 6);

	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(2, 5), 7);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(5, 2), 7);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(2, 10), 7);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(10, 2), 7);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(5, 10), 7);
	EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Add(FIntPoint(10, 5), 7);

	AngleSectionIndex_to_SideIndexes_Map.Add(0, { 0, 2, 4 });
	AngleSectionIndex_to_SideIndexes_Map.Add(1, { 1, 4, 2 });
	AngleSectionIndex_to_SideIndexes_Map.Add(2, { 1, 3, 4 });
	AngleSectionIndex_to_SideIndexes_Map.Add(3, { 0, 4, 3 });
	AngleSectionIndex_to_SideIndexes_Map.Add(4, { 0, 5, 2 });
	AngleSectionIndex_to_SideIndexes_Map.Add(5, { 2, 5, 1 });
	AngleSectionIndex_to_SideIndexes_Map.Add(6, { 1, 5, 3 });
	AngleSectionIndex_to_SideIndexes_Map.Add(7, { 0, 3, 5 });

	SideIndex_to_MeshNormal_Map.Add(0, FVector(0, -1, 0));
	SideIndex_to_MeshNormal_Map.Add(1, FVector(0, 1, 0));
	SideIndex_to_MeshNormal_Map.Add(2, FVector(-1, 0, 0));
	SideIndex_to_MeshNormal_Map.Add(3, FVector(1, 0, 0));
	SideIndex_to_MeshNormal_Map.Add(4, FVector(0, 0, 1));
	SideIndex_to_MeshNormal_Map.Add(5, FVector(0, 0, -1));

	EdgeSectionIndex_to_MeshNormal_Map.Add(0, FVector(0, -1, 1));
	EdgeSectionIndex_to_MeshNormal_Map.Add(1, FVector(0, 1, -1));
	EdgeSectionIndex_to_MeshNormal_Map.Add(2, FVector(0, -1, -1));
	EdgeSectionIndex_to_MeshNormal_Map.Add(3, FVector(0, 1, 1));
	EdgeSectionIndex_to_MeshNormal_Map.Add(4, FVector(-1, 0, 1));
	EdgeSectionIndex_to_MeshNormal_Map.Add(5, FVector(1, 0, -1));
	EdgeSectionIndex_to_MeshNormal_Map.Add(6, FVector(-1, 0, -1));
	EdgeSectionIndex_to_MeshNormal_Map.Add(7, FVector(1, 0, 1));
	EdgeSectionIndex_to_MeshNormal_Map.Add(8, FVector(-1, -1, 0));
	EdgeSectionIndex_to_MeshNormal_Map.Add(9, FVector(1, 1, 0));
	EdgeSectionIndex_to_MeshNormal_Map.Add(10, FVector(1, -1, 0));
	EdgeSectionIndex_to_MeshNormal_Map.Add(11, FVector(-1, 1, 0));

	AngleSectionIndex_to_MeshNormal_Map.Add(0, FVector(-1, -1, 1));
	AngleSectionIndex_to_MeshNormal_Map.Add(1, FVector(-1, 1, 1));
	AngleSectionIndex_to_MeshNormal_Map.Add(2, FVector(1, 1, 1));
	AngleSectionIndex_to_MeshNormal_Map.Add(3, FVector(1, -1, 1));
	AngleSectionIndex_to_MeshNormal_Map.Add(4, FVector(-1, -1, -1));
	AngleSectionIndex_to_MeshNormal_Map.Add(5, FVector(-1, 1, -1));
	AngleSectionIndex_to_MeshNormal_Map.Add(6, FVector(1, 1, -1));
	AngleSectionIndex_to_MeshNormal_Map.Add(7, FVector(1, -1, -1));

	TArray<int32> UnattachedSideIndexesArray;

	TArray<int32> Numbers = { 0, 1, 2, 3, 4, 5 };

	for (int32 k = 1; k <= 6; ++k)
	{
		TArray<int32> TempCombination;

		GenerateCombinations(Numbers, 0, 0, k, TempCombination);
	}
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
		if (OwnerChunk->IsPointInsideTree(SamplePoint) && !HasVoxelOfAnyTypeAtPoint(SamplePoint))
		{
			return false;
		}
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
	if (InVoxelInstanceData.IsTypeDataValid())
	{
		PossibleMeshPointsSet.Add(InPoint);
	}
	else
	{
		PossibleMeshPointsSet.Remove(InPoint);
	}
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
				if (IsVoxelSidesClosed(SamplePoint)) // ...but doesn't need anymore
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


void UATVoxelPMC::GenerateVoxelMeshPrototypeByUnattachedSideIndexes(TArray<int32>& InUnattachedSideIndexesArray)
{
	TMap<int32, TArray<FVector>>& VoxelSidePlaneDataIndex_to_BelongVertexesArray_Map = UnattachedSideIndexes_to_VoxelSidePlaneDataIndexToBelongVertexesMap_Map.Add(InUnattachedSideIndexesArray, TMap<int32, TArray<FVector>>());

	TMap<int32, TArray<FVector>>& EdgeSectionPlaneDataIndex_to_BelongVertexesArray_Map = UnattachedSideIndexes_to_EdgeSectionPlaneDataIndexToBelongVertexesMap_Map.Add(InUnattachedSideIndexesArray, TMap<int32, TArray<FVector>>());

	TArray<TPair<int32, TArray<FVector>>>* AngleSectionIndexToVerticesPairsArray = nullptr;

	if (InUnattachedSideIndexesArray.Num() > 2) AngleSectionIndexToVerticesPairsArray = &UnattachedSideIndexes_to_AngleSectionIndexToVerticesPairsArray_Map.Add(InUnattachedSideIndexesArray, TArray<TPair<int32, TArray<FVector>>>());

	FVector CurrentIntersectionPoint = FVector::ZeroVector;

	int32 CurrentIntersectionPointIndex = NULL;
	
	EVoxelPlaneType FirstAdjacentPlaneType;

	int32* FirstAdjacentPlaneDataIndex = nullptr;

	EVoxelPlaneType SecondAdjacentPlaneType;

	int32* SecondAdjacentPlaneDataIndex = nullptr;

	int32 FirstPreviousEdgeSectionIndex = -1;

	int32 SecondPreviousEdgeSectionIndex = -1;

	for (int32& UnattachedSideIndex : InUnattachedSideIndexesArray)
	{
		if (TArray<int32>* IntersectableSidePlaneDataIndexesArray = SidePlaneDataIndex_to_IntersectableSidePlaneDataIndexesArray_Map.Find(UnattachedSideIndex))
		{
			for (int32 SideAngleIndex = 0; SideAngleIndex < 4; SideAngleIndex++)
			{
				FirstAdjacentPlaneDataIndex = nullptr;

				SecondAdjacentPlaneDataIndex = nullptr;

				if (InUnattachedSideIndexesArray.Contains((*IntersectableSidePlaneDataIndexesArray)[SideAngleIndex]))
				{
					FirstAdjacentPlaneDataIndex = UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Find(FIntPoint(UnattachedSideIndex, (*IntersectableSidePlaneDataIndexesArray)[SideAngleIndex]));
				}

				if (FirstAdjacentPlaneDataIndex) FirstAdjacentPlaneType = EVoxelPlaneType::EdgeSection;

				else
				{
					FirstAdjacentPlaneType = EVoxelPlaneType::Side;

					FirstAdjacentPlaneDataIndex = &(*IntersectableSidePlaneDataIndexesArray)[SideAngleIndex];
				}

				if (InUnattachedSideIndexesArray.Contains((*IntersectableSidePlaneDataIndexesArray)[SideAngleIndex == 3 ? 0 : SideAngleIndex + 1]))
				{
					SecondAdjacentPlaneDataIndex = UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Find(FIntPoint(UnattachedSideIndex, (*IntersectableSidePlaneDataIndexesArray)[SideAngleIndex == 3 ? 0 : SideAngleIndex + 1]));
				}

				if (SecondAdjacentPlaneDataIndex) SecondAdjacentPlaneType = EVoxelPlaneType::EdgeSection; 

				else
				{
					SecondAdjacentPlaneType = EVoxelPlaneType::Side;

					SecondAdjacentPlaneDataIndex = &(*IntersectableSidePlaneDataIndexesArray)[SideAngleIndex == 3 ? 0 : SideAngleIndex + 1];
				}

				if (TryIntersectThreePlanes(VoxelSidePlaneDataArray[UnattachedSideIndex],
					FirstAdjacentPlaneType == EVoxelPlaneType::Side ? VoxelSidePlaneDataArray[*FirstAdjacentPlaneDataIndex] : EdgeSectionPlaneDataArray[*FirstAdjacentPlaneDataIndex],
					SecondAdjacentPlaneType == EVoxelPlaneType::Side ? VoxelSidePlaneDataArray[*SecondAdjacentPlaneDataIndex] : EdgeSectionPlaneDataArray[*SecondAdjacentPlaneDataIndex],
					CurrentIntersectionPoint))
				{
					if (TArray<FVector>* UnattachedSideVertexesArray = VoxelSidePlaneDataIndex_to_BelongVertexesArray_Map.Find(UnattachedSideIndex)) UnattachedSideVertexesArray->Add(CurrentIntersectionPoint);

					else VoxelSidePlaneDataIndex_to_BelongVertexesArray_Map.Add(UnattachedSideIndex, { CurrentIntersectionPoint });

					if (FirstAdjacentPlaneType == EVoxelPlaneType::EdgeSection)
					{
						if (TArray<FVector>* EdgeSectionPlaneBelongVertexesArray = EdgeSectionPlaneDataIndex_to_BelongVertexesArray_Map.Find(*FirstAdjacentPlaneDataIndex))
						{
							if (EdgeSectionPlaneBelongVertexesArray->Num() % 2 != 0 && *FirstAdjacentPlaneDataIndex != SecondPreviousEdgeSectionIndex)
							{
								EdgeSectionPlaneBelongVertexesArray->Insert(CurrentIntersectionPoint, EdgeSectionPlaneBelongVertexesArray->Num() == 3 ? 2 : 0);
							}
							else EdgeSectionPlaneBelongVertexesArray->Add(CurrentIntersectionPoint);
						} 
						else EdgeSectionPlaneDataIndex_to_BelongVertexesArray_Map.Add(*FirstAdjacentPlaneDataIndex, { CurrentIntersectionPoint });

						FirstPreviousEdgeSectionIndex = *FirstAdjacentPlaneDataIndex;
					}
					else if (FirstPreviousEdgeSectionIndex != -1) FirstPreviousEdgeSectionIndex = -1;

					if (SecondAdjacentPlaneType == EVoxelPlaneType::EdgeSection)
					{
						if (TArray<FVector>* EdgeSectionPlaneBelongVertexesArray = EdgeSectionPlaneDataIndex_to_BelongVertexesArray_Map.Find(*SecondAdjacentPlaneDataIndex))
						{
							if (EdgeSectionPlaneBelongVertexesArray->Num() % 2 != 0 && *SecondAdjacentPlaneDataIndex != FirstPreviousEdgeSectionIndex)
							{
								EdgeSectionPlaneBelongVertexesArray->Insert(CurrentIntersectionPoint, EdgeSectionPlaneBelongVertexesArray->Num() == 3 ? 2 : 0);
							}
							else EdgeSectionPlaneBelongVertexesArray->Add(CurrentIntersectionPoint);
						}
						else EdgeSectionPlaneDataIndex_to_BelongVertexesArray_Map.Add(*SecondAdjacentPlaneDataIndex, { CurrentIntersectionPoint });

						SecondPreviousEdgeSectionIndex = *SecondAdjacentPlaneDataIndex;
					}
					else if (SecondPreviousEdgeSectionIndex != -1) SecondPreviousEdgeSectionIndex = -1;

					if (AngleSectionIndexToVerticesPairsArray && FirstAdjacentPlaneType == EVoxelPlaneType::EdgeSection && SecondAdjacentPlaneType == EVoxelPlaneType::EdgeSection)
					{
						if (int32* AngleSectionIndex = EdgeSectionIndexesPair_to_AngleSectionIndex_Map.Find(FIntPoint(*FirstAdjacentPlaneDataIndex, *SecondAdjacentPlaneDataIndex)))
						{
							if (TPair<int32, TArray<FVector>>* AngleSectionIndexToVerticesPair = AngleSectionIndexToVerticesPairsArray->FindByPredicate([AngleSectionIndex](TPair<int32, TArray<FVector>>& InAngleSectionIndexToVerticesPair) { return InAngleSectionIndexToVerticesPair.Key == *AngleSectionIndex; }))
							{
								if (TArray<int32>* AngleSectionAdjacentSideIndexesArray = AngleSectionIndex_to_SideIndexes_Map.Find(*AngleSectionIndex))
								{
									AngleSectionIndexToVerticesPair->Value[AngleSectionAdjacentSideIndexesArray->IndexOfByKey(UnattachedSideIndex)] = CurrentIntersectionPoint;
								}
							}
							else
							{
								AngleSectionIndexToVerticesPair = &(*AngleSectionIndexToVerticesPairsArray)[AngleSectionIndexToVerticesPairsArray->AddDefaulted()];

								AngleSectionIndexToVerticesPair->Key = *AngleSectionIndex;

								AngleSectionIndexToVerticesPair->Value.Init(FVector::ZeroVector, 3);

								if (TArray<int32>* AngleSectionAdjacentSideIndexesArray = AngleSectionIndex_to_SideIndexes_Map.Find(*AngleSectionIndex))
								{
									AngleSectionIndexToVerticesPair->Value[AngleSectionAdjacentSideIndexesArray->IndexOfByKey(UnattachedSideIndex)] = CurrentIntersectionPoint;
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

	TArray<FVector> SidesMeshVerticesArray;
	TArray<FVector> EdgeSectionPlanesMeshVerticesArray;
	TArray<FVector> AngleSectionsMeshVerticesArray;
	
	TArray<int32> SidesMeshTriangleVertexIndexesArray;
	TArray<int32> EdgeSectionPlanesMeshTriangleVertexIndexesArray;
	TArray<int32> AngleSectionsMeshTriangleVertexIndexesArray;

	TArray<FVector> SidesMeshNormalsArray;
	TArray<FVector> EdgeSectionsMeshNormalsArray;
	TArray<FVector> AngleSectionsMeshNormalsArray;

	TArray<FVector2D> SidesMeshUVPointsArray;
	TArray<FVector2D> EdgeSectionsMeshUVPointsArray;
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
	TArray<FProcMeshTangent> AngleSectionsMeshTangentsArray;

	int32 AddedSidesNum = 0;

	int32 AddedEdgeSectionsNum = 0;

	int32 CurrentFaceStartVertexIndex = NULL;

	int32 CurrentEdgeSectionVertexIndex = 0;

	for (FIntVector& VisibleVoxelPoint : InVisibleVoxelPointsArray)
	{
		VoxelRelativeLocation = UATWorldFunctionLibrary::Point3D_To_RelativeLocation(this, VisibleVoxelPoint);

		GetUnattachedSideIndexes(VisibleVoxelPoint, UnattachedSideIndexesArray);

		UnattachedSideIndexesArray.Sort();

		if (TMap<int32, TArray<FVector>>* VoxelSidePlaneDataIndex_to_BelongVertexesArray_Map = UnattachedSideIndexes_to_VoxelSidePlaneDataIndexToBelongVertexesMap_Map.Find(UnattachedSideIndexesArray))
		{
			for (int32& UnattachedSideIndex : UnattachedSideIndexesArray)
			{
				if (TArray<FVector>* VoxelSidePlaneVertexesArray = VoxelSidePlaneDataIndex_to_BelongVertexesArray_Map->Find(UnattachedSideIndex))
				{
					for (int32 SideVertexIndex = 0; SideVertexIndex < 4; SideVertexIndex++)
					{
						SidesMeshVerticesArray.Add((*VoxelSidePlaneVertexesArray)[SideVertexIndex] + VoxelRelativeLocation);

						if (FVector* SideMeshNormal = SideIndex_to_MeshNormal_Map.Find(UnattachedSideIndex)) SidesMeshNormalsArray.Add(*SideMeshNormal);
						
						SidesMeshUVPointsArray.Add(DefaultUVPointsArray[SideVertexIndex]);

						SidesMeshTangentsArray.Add(FProcMeshTangent(1, 0, 0));
					}
				}

				CurrentFaceStartVertexIndex = AddedSidesNum * 4;

				SidesMeshTriangleVertexIndexesArray.Add(CurrentFaceStartVertexIndex + 0);
				SidesMeshTriangleVertexIndexesArray.Add(CurrentFaceStartVertexIndex + 1);
				SidesMeshTriangleVertexIndexesArray.Add(CurrentFaceStartVertexIndex + 2);

				SidesMeshTriangleVertexIndexesArray.Add(CurrentFaceStartVertexIndex + 0);
				SidesMeshTriangleVertexIndexesArray.Add(CurrentFaceStartVertexIndex + 2);
				SidesMeshTriangleVertexIndexesArray.Add(CurrentFaceStartVertexIndex + 3);

				AddedSidesNum++;
			}
		}

		if (TMap<int32, TArray<FVector>>* EdgeSectionPlaneDataIndex_to_BelongVertexes_Map = UnattachedSideIndexes_to_EdgeSectionPlaneDataIndexToBelongVertexesMap_Map.Find(UnattachedSideIndexesArray))
		{
			for (int32 i = 0; i < UnattachedSideIndexesPairsArray.Num(); i++)
			{
				if (UnattachedSideIndexesArray.Contains(UnattachedSideIndexesPairsArray[i].X) && UnattachedSideIndexesArray.Contains(UnattachedSideIndexesPairsArray[i].Y))
				{
					if (int32* EdgeSectionPlaneDataIndex = UnattachedSideIndexesPair_to_EdgeSectionPlaneDataIndex_Map.Find(UnattachedSideIndexesPairsArray[i]))
					{
						if (TArray<FVector>* EdgeSectionPlaneVertexesArray = EdgeSectionPlaneDataIndex_to_BelongVertexes_Map->Find(*EdgeSectionPlaneDataIndex)) // EdgeSectionPlaneVertexesArray - Contains vertexes in clock-wize allocation
						{
							EdgeSectionPlanesMeshVerticesArray += *EdgeSectionPlaneVertexesArray;

							CurrentEdgeSectionVertexIndex = 0;

							for (int j = EdgeSectionPlanesMeshVerticesArray.Num() - 4; j < EdgeSectionPlanesMeshVerticesArray.Num(); j++)
							{
								EdgeSectionPlanesMeshVerticesArray[j] += VoxelRelativeLocation;

								if (FVector* EdgeSectionMeshNormal = EdgeSectionIndex_to_MeshNormal_Map.Find(*EdgeSectionPlaneDataIndex)) EdgeSectionsMeshNormalsArray.Add(*EdgeSectionMeshNormal);

								EdgeSectionsMeshUVPointsArray.Add(ClockwiseDefaultUVPointsArray[CurrentEdgeSectionVertexIndex]);

								EdgeSectionsMeshTangentsArray.Add(FProcMeshTangent(1, 0, 0));

								CurrentEdgeSectionVertexIndex++;
							}
						}
					}
				}
				CurrentFaceStartVertexIndex = AddedEdgeSectionsNum * 4;

				EdgeSectionPlanesMeshTriangleVertexIndexesArray.Add(CurrentFaceStartVertexIndex + 1);
				EdgeSectionPlanesMeshTriangleVertexIndexesArray.Add(CurrentFaceStartVertexIndex + 0);
				EdgeSectionPlanesMeshTriangleVertexIndexesArray.Add(CurrentFaceStartVertexIndex + 2);

				EdgeSectionPlanesMeshTriangleVertexIndexesArray.Add(CurrentFaceStartVertexIndex + 2);
				EdgeSectionPlanesMeshTriangleVertexIndexesArray.Add(CurrentFaceStartVertexIndex + 0);
				EdgeSectionPlanesMeshTriangleVertexIndexesArray.Add(CurrentFaceStartVertexIndex + 3);

				AddedEdgeSectionsNum++;
			}
		}

		if (TArray<TPair<int32, TArray<FVector>>>* AngleSectionIndexToVerticesPairsArray = UnattachedSideIndexes_to_AngleSectionIndexToVerticesPairsArray_Map.Find(UnattachedSideIndexesArray))
		{
			for (TPair<int32, TArray<FVector>>& AngleSectionIndexToVerticesPair : *AngleSectionIndexToVerticesPairsArray)
			{
				if (!AngleSectionIndexToVerticesPair.Value.IsEmpty())
				{
					AngleSectionsMeshVerticesArray += AngleSectionIndexToVerticesPair.Value;

					for (int32 i = 0; i < 3; i++)
					{
						AngleSectionsMeshVerticesArray[AngleSectionsMeshTriangleVertexIndexesArray.Num()] += VoxelRelativeLocation;

						AngleSectionsMeshTriangleVertexIndexesArray.Add(AngleSectionsMeshTriangleVertexIndexesArray.Num());

						if (FVector* AngleSectionMeshNormal = AngleSectionIndex_to_MeshNormal_Map.Find(AngleSectionIndexToVerticesPair.Key)) AngleSectionsMeshNormalsArray.Add(*AngleSectionMeshNormal);

						AngleSectionsMeshUVPointsArray.Add(DefaultUVPointsArray[i]);

						AngleSectionsMeshTangentsArray.Add(FProcMeshTangent(1, 0, 0));
					}
				}
			}
		}
	}

	CreateMeshSection(0, SidesMeshVerticesArray, SidesMeshTriangleVertexIndexesArray, SidesMeshNormalsArray, SidesMeshUVPointsArray, TArray<FColor>(), SidesMeshTangentsArray, true);

	CreateMeshSection(1, EdgeSectionPlanesMeshVerticesArray, EdgeSectionPlanesMeshTriangleVertexIndexesArray, EdgeSectionsMeshNormalsArray, EdgeSectionsMeshUVPointsArray, TArray<FColor>(), EdgeSectionsMeshTangentsArray, true);

	CreateMeshSection(2, AngleSectionsMeshVerticesArray, AngleSectionsMeshTriangleVertexIndexesArray, AngleSectionsMeshNormalsArray, AngleSectionsMeshUVPointsArray, TArray<FColor>(), AngleSectionsMeshTangentsArray, true);
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

bool UATVoxelPMC::IsEdgeSectionPlanesIntersectable(const int32& InFirstEdgeSectionPlaneIndex, const int32& InSecondEdgeSectionPlaneIndex) const
{
	if (VoxelTypeData->SectionsDepth <= 0.25)
	{
		const TArray<int32>* IntersectableEdgeSectionPlaneDataIndexesArray = EdgeSectionPlaneDataIndex_to_IntersectableEdgeSectionPlaneDataIndexesArray_Map.Find(InFirstEdgeSectionPlaneIndex);
		
		return IntersectableEdgeSectionPlaneDataIndexesArray && IntersectableEdgeSectionPlaneDataIndexesArray->Contains(InSecondEdgeSectionPlaneIndex);
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

