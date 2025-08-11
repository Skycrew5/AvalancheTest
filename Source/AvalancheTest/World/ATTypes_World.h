// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "ATTypes_World.generated.h"

#if DEBUG_VOXELS
	#pragma optimize("", off)
#endif // DEBUG_VOXELS

UENUM(BlueprintType, meta = (DisplayName = "[AT] Attachment Direction"))
enum class EATAttachmentDirection : uint8
{
	None,
	Front,
	Back,
	Right,
	Left,
	Top,
	Bottom
};

namespace FATVoxelUtils
{
	const TArray<EATAttachmentDirection> AttachmentDirectionsArray = {
		EATAttachmentDirection::None,
		EATAttachmentDirection::Front,
		EATAttachmentDirection::Back,
		EATAttachmentDirection::Right,
		EATAttachmentDirection::Left,
		EATAttachmentDirection::Top,
		EATAttachmentDirection::Bottom
	};

	const TMap<EATAttachmentDirection, EATAttachmentDirection> Opposites = {
		{ EATAttachmentDirection::Front, EATAttachmentDirection::Back },
		{ EATAttachmentDirection::Back, EATAttachmentDirection::Front },
		{ EATAttachmentDirection::Right, EATAttachmentDirection::Left },
		{ EATAttachmentDirection::Left, EATAttachmentDirection::Right },
		{ EATAttachmentDirection::Top, EATAttachmentDirection::Back },
		{ EATAttachmentDirection::Bottom, EATAttachmentDirection::Top }
	};
	
	const TMap<EATAttachmentDirection, FIntVector> IntOffsets =	{
		{ EATAttachmentDirection::None, FIntVector(0, 0, 0) }, // Used on recursion start, should be zero
		{ EATAttachmentDirection::Front, FIntVector(1, 0, 0) },
		{ EATAttachmentDirection::Back, FIntVector(-1, 0, 0) },
		{ EATAttachmentDirection::Right, FIntVector(0, 1, 0) },
		{ EATAttachmentDirection::Left, FIntVector(0, -1, 0) },
		{ EATAttachmentDirection::Top, FIntVector(0, 0, 1) },
		{ EATAttachmentDirection::Bottom, FIntVector(0, 0, -1) }
	};

	const TMap<EATAttachmentDirection, float> BaseAttachmentStrengthMuls = {
		{ EATAttachmentDirection::None, 1.0f }, // Used on recursion start, should be 1.0f
		{ EATAttachmentDirection::Front, 0.85f },
		{ EATAttachmentDirection::Back, 0.85f },
		{ EATAttachmentDirection::Right, 0.85f },
		{ EATAttachmentDirection::Left, 0.85f },
		{ EATAttachmentDirection::Top, 0.6 },
		{ EATAttachmentDirection::Bottom, 1.0f }
	};

	static FVector CreateVectorFromAttachmentDirections(const TSet<EATAttachmentDirection>& InDirectionsSet)
	{
		FVector OutVector = FVector::ZeroVector;

		for (EATAttachmentDirection SampleDirection : InDirectionsSet)
		{
			switch (SampleDirection)
			{
				case EATAttachmentDirection::Front: { OutVector += FVector::ForwardVector; break; }
				case EATAttachmentDirection::Back: { OutVector += FVector::ForwardVector; break; }
				case EATAttachmentDirection::Right: { OutVector += FVector::RightVector; break; }
				case EATAttachmentDirection::Left: { OutVector += FVector::LeftVector; break; }
				case EATAttachmentDirection::Top: { OutVector += FVector::UpVector; break; }
				case EATAttachmentDirection::Bottom: { OutVector += FVector::DownVector; break; }
			}
		}
		return OutVector;
	}

	static FString CreateStringFromAttachmentDirections(const TSet<EATAttachmentDirection>& InDirectionsSet)
	{
		FString OutString = FString();

		for (EATAttachmentDirection SampleDirection : InDirectionsSet)
		{
			switch (SampleDirection)
			{
				case EATAttachmentDirection::Front: { OutString += TEXT("Front | "); break; }
				case EATAttachmentDirection::Back: { OutString += TEXT("Back | "); break; }
				case EATAttachmentDirection::Right: { OutString += TEXT("Right | "); break; }
				case EATAttachmentDirection::Left: { OutString += TEXT("Left | "); break; }
				case EATAttachmentDirection::Top: { OutString += TEXT("Top | "); break; }
				case EATAttachmentDirection::Bottom: { OutString += TEXT("Bottom | "); break; }
			}
		}
		OutString.RemoveFromEnd(" | ");
		return OutString;
	}

	const TArray<FIntVector> SideOffsets = {
		FIntVector(1, 0, 0),
		FIntVector(-1, 0, 0),
		FIntVector(0, 1, 0),
		FIntVector(0, -1, 0),
		FIntVector(0, 0, 1),
		FIntVector(0, 0, -1)
	};
}

USTRUCT(BlueprintType)
struct FVoxelInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<const class UATVoxelTypeData> TypeData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AvalancheValue = 1.0f;

	bool IsTypeDataValid() const { return TypeData != nullptr; }

	static const FVoxelInstanceData Invalid;
};

USTRUCT(BlueprintType)
struct FVoxelBreakData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bForced = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNotify = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> Source = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AController> Instigator = nullptr;

	FVoxelBreakData(bool bInForced = false, bool bInNotify = true, AActor* InSource = nullptr, AController* InInstigator = nullptr)
		: bForced(bInForced), bNotify(bInNotify), Source(InSource), Instigator(InInstigator) {}
};

USTRUCT()
struct FChunkWithSquaredDistance
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<class AATVoxelChunk> Chunk = nullptr;

	UPROPERTY()
	int32 SquaredDistance = 0;

	operator TObjectPtr<class AATVoxelChunk>() const
	{
		return Chunk;
	}

	bool operator==(const FChunkWithSquaredDistance& InOther) const
	{
		return Chunk == InOther.Chunk;
	}

	bool operator!=(const FChunkWithSquaredDistance& InOther) const
	{
		return Chunk != InOther.Chunk;
	}

	bool operator==(const TObjectPtr<class AATVoxelChunk>& InOtherChunk) const
	{
		return Chunk == InOtherChunk;
	}

	bool operator!=(const TObjectPtr<class AATVoxelChunk>& InOtherChunk) const
	{
		return Chunk != InOtherChunk;
	}
};

USTRUCT()
struct FSortedChunksBySquaredDistance
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FChunkWithSquaredDistance> DataArray;

	void UpdateDistancesAndSort(const class AATVoxelTree* InTree, const bool bInReverse);
};

USTRUCT(BlueprintType)
struct FVoxelChunkDebugData_Entry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Value;

	FString ToString() const
	{
		return FString::Printf(TEXT("%s: %s"), *Label, *Value);
	}

	FVoxelChunkDebugData_Entry() : Label(FString()), Value(FString()) {}
	FVoxelChunkDebugData_Entry(const FString& InLabel, const FString& InValue) : Label(InLabel), Value(InValue) {}

	FVoxelChunkDebugData_Entry(const FString& InLabel, float InFloatValue, const int32 InFloatValueMinDigits = 2)
	{
		Label = InLabel;
		Value = FString::SanitizeFloat(InFloatValue, InFloatValueMinDigits);
	}

	FVoxelChunkDebugData_Entry(const FString& InLabel, int32 InIntValue)
	{
		Label = InLabel;
		Value = FString::FromInt(InIntValue);
	}
};

USTRUCT(BlueprintType)
struct FVoxelChunkDebugData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform ChunkHighlightTransform = FTransform();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Label = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FColor LabelColor = FColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVoxelChunkDebugData_Entry> CommonEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVoxelChunkDebugData_Entry> AttachmentsEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVoxelChunkDebugData_Entry> StabilityEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InstanceLabel = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVoxelChunkDebugData_Entry> InstanceEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform InstanceHighlightTransform = FTransform();

	void Reset()
	{
		ChunkHighlightTransform = FTransform();
		Label = FString();
		LabelColor = FColor::White;

		CommonEntries.Empty();
		AttachmentsEntries.Empty();
		StabilityEntries.Empty();

		InstanceLabel = FString();
		InstanceEntries.Empty();
		InstanceHighlightTransform = FTransform();
	}
};

/*USTRUCT(BlueprintType)
struct FVoxelCompoundData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Origin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Size;

	bool IsValid() const { return Size > 0; }

	void GetAllPoints(TArray<FIntVector>& OutPoints) const;
	void GetPointsAtSide(EATAttachmentDirection InSide, TArray<FIntVector>& OutPoints, const bool bInOutside = false) const;

	FVoxelCompoundData(const FIntVector& InOrigin = FIntVector::ZeroValue, int32 InSize = 0)
		: Origin(InOrigin), Size(InSize) {
	}

	static const FVoxelCompoundData Invalid;
};

USTRUCT(BlueprintType)
struct FVoxelChunkPendingUpdates
{
	GENERATED_BODY()

private:

	TArraySetPair<FIntVector> PendingPoints;
	TArraySetPair<FIntVector> ThisTickSelectedPoints;
	TArraySetPair<FIntVector> ThisTickAlreadyUpdatedPoints;
	TArraySetPair<FIntVector> NextTickNewPendingIndices;

	UPROPERTY()
	bool bIsInsideUpdateSequence;

public:

	const TArray<FIntVector>& GetPendingPointsConstArray() const { return PendingPoints.GetConstArray(); }
	const TArray<FIntVector>& GetThisTickSelectedPointsConstArray() const { return ThisTickSelectedPoints.GetConstArray(); }

	UPROPERTY()
	TObjectPtr<class UATVoxelISMC> TargetISMC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DebugVoxelCustomData_ThisTickSelected = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDebugMarkThisTickSelectedIndices = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDebugDeMarkThisTickSelectedIndices = true;

	void QueuePointIfRelevant(const FIntVector& InPoint);
	void MarkPointAsUpdatedThisTick(const FIntVector& InPoint);
	bool PrepareThisTickSelectedPoints(int32 InDesiredUpdatesNum);
	bool IsInstanceWaitingToUpdateThisTick(const FIntVector& InPoint) const;
	void ResolveThisTickSelectedPoints();
	int32 ResolveThisTickAlreadyUpdatedPoints();
};*/

#if DEBUG_VOXELS
	#pragma optimize("", on)
#endif // DEBUG_VOXELS
