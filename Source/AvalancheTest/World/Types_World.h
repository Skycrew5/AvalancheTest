// Avalanche Test

#pragma once

#include "AvalancheTest.h"

#include "Types_World.generated.h"

USTRUCT(BlueprintType)
struct FVoxelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TypeID = FVoxelData::AirTypeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 1.0f;

	static const FVoxelData Invalid;
	static const FVoxelData Air;

	static const int32 InvalidTypeID = -1;
	static const int32 AirTypeID = 0;
};
