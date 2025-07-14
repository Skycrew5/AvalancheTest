// Scientific Ways

#pragma once

#include "CoreMinimal.h"
#include "Containers/ArrayView.h"
#include "Delegates/IDelegateInstance.h"
#include "Delegates/DelegateCombinations.h"

#include "InstancedStaticMeshDelegates.h"
#include "Components/InstancedStaticMeshComponent.h"

#include "UnrealCommons.h"

DECLARE_LOG_CATEGORY_EXTERN(LogATGameplay, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogVoxels, Log, All);

DECLARE_STATS_GROUP(TEXT("Voxels memory and other stats."), STATGROUP_Voxels, STATCAT_Performance);

DECLARE_MEMORY_STAT(TEXT("Voxel Data: Queued_Point_To_VoxelInstanceData_Map"), STAT_VoxelData_Queued_Point_To_VoxelInstanceData_Map, STATGROUP_Voxels);
DECLARE_MEMORY_STAT(TEXT("Voxel Data: Point_To_VoxelInstanceData_Map"), STAT_VoxelData_Point_To_VoxelInstanceData_Map, STATGROUP_Voxels);

DECLARE_MEMORY_STAT(TEXT("Simulation Tasks: QueuedPoints"), STAT_SimulationTasks_QueuedPoints, STATGROUP_Voxels);
DECLARE_MEMORY_STAT(TEXT("Simulation Tasks: StabilityRecursive PointsCache"), STAT_SimulationTasks_StabilityRecursiveCache, STATGROUP_Voxels);

DECLARE_MEMORY_STAT(TEXT("Voxel Components: Point_To_MeshIndex_Map"), STAT_VoxelComponents_Point_To_MeshIndex_Map, STATGROUP_Voxels);

#define SIZE_T_TO_MB(InSizeType) ((static_cast<double>((sizeof InSizeType) * InSizeType) / 8.0) / (1024.0 * 1024.0))

#define DEBUG_VOXELS 1
//#define DEBUG_VOXELS 0
