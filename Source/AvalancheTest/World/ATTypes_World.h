// Avalanche Test

#pragma once

#include "AvalancheTest.h"

#include "ATTypes_World.generated.h"

USTRUCT(BlueprintType)
struct FVoxelInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StaticMeshInstanceIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<const class UATVoxelTypeData> TypeData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 1.0f;

	static const FVoxelInstanceData Invalid;
};
