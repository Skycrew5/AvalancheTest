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

namespace EATAttachmentDirection_Utils
{
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

	const TMap<EATAttachmentDirection, float> AttachmentMuls = {
		{ EATAttachmentDirection::None, 1.0f }, // Used on recursion start, should be 1.0f
		{ EATAttachmentDirection::Front, 0.8f },
		{ EATAttachmentDirection::Back, 0.8f },
		{ EATAttachmentDirection::Right, 0.8f },
		{ EATAttachmentDirection::Left, 0.8f },
		{ EATAttachmentDirection::Top, 0.4 },
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
}

USTRUCT(BlueprintType)
struct FVoxelInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<const class UATVoxelTypeData> TypeData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 10.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Stability = 0.0f;

	bool IsTypeDataValid() const { return TypeData != nullptr; }

	static const FVoxelInstanceData Invalid;
};

template<typename ElementType>
struct TArraySetPair
{
private:
	TArray<ElementType> Array;
	TSet<ElementType> Set;
public:

	FORCEINLINE const TArray<ElementType>& GetConstArray() const { return Array; }
	FORCEINLINE const TSet<ElementType>& GetConstSet() const { return Set; }

	FORCEINLINE bool Add(const ElementType& InElement)
	{
		if (Contains(InElement))
		{
			return false;
		}
		else
		{
			Set.Add(InElement);
			Array.Add(InElement);
			ensure(Array.Num() == Set.Num());
			return true;
		}
	}

	FORCEINLINE bool Remove(const ElementType& InElement)
	{
		if (Contains(InElement))
		{
			Set.Remove(InElement);
			Array.Remove(InElement);
			ensure(Array.Num() == Set.Num());
			return true;
		}
		else
		{
			return false;
		}
	}

	FORCEINLINE bool Replace(const ElementType& InPrevElement, const ElementType& InNewElement)
	{
		if (Contains(InPrevElement))
		{
			Remove(InPrevElement);
			Add(InNewElement);
			return true;
		}
		else
		{
			return false;
		}
	}

	FORCEINLINE void AddFromOther(const TArraySetPair<ElementType>& InOther)
	{
		for (const ElementType& SampleOtherItem : InOther.Array)
		{
			Add(SampleOtherItem);
		}
	}

	FORCEINLINE void RemoveFromOther(const TArraySetPair<ElementType>& InOther)
	{
		for (const ElementType& SampleOtherItem : InOther.Array)
		{
			Remove(SampleOtherItem);
		}
	}

	FORCEINLINE void AddHeadTo(int32 InDesiredElementsNum, TArraySetPair<ElementType>& InOutOther, const bool bInPopElements = false)
	{
		int32 FirstIndex = 0;
		int32 LastIndex = FMath::Min(InDesiredElementsNum, Array.Num()) - 1;

		for (int32 SampleIndex = FirstIndex; SampleIndex <= LastIndex; ++SampleIndex)
		{
			InOutOther.Add(Array[SampleIndex]);
		}
		if (bInPopElements)
		{
			RemoveFromOther(InOutOther);
		}
	}

	FORCEINLINE void AddTailTo(int32 InDesiredElementsNum, TArraySetPair<ElementType>& InOutOther, const bool bInPopElements = false)
	{
		int32 FirstIndex = Array.Num() - 1;
		int32 LastIndex = Array.Num() - FMath::Min(InDesiredElementsNum, Array.Num());

		for (int32 SampleIndex = FirstIndex; SampleIndex >= LastIndex; --SampleIndex)
		{
			InOutOther.Add(Array[SampleIndex]);
		}
		if (bInPopElements)
		{
			RemoveFromOther(InOutOther);
		}
	}

	FORCEINLINE int32 Num() const { return Array.Num(); }
	FORCEINLINE bool IsEmpty() const { return Array.IsEmpty(); }
	FORCEINLINE bool Contains(const ElementType& InElement) const { return Set.Contains(InElement); }

	FORCEINLINE void Empty(int32 InSlack = 0)
	{
		Array.Empty(InSlack);
		Set.Empty(InSlack);
	}

	FORCEINLINE ElementType Pop(EAllowShrinking InAllowShrinking = EAllowShrinking::Default)
	{
		ElementType PoppedItem = Array.Pop(InAllowShrinking);
		Set.Remove(PoppedItem);
		return PoppedItem;
	}
};

template<typename KeyType, typename ValueType>
struct TMirroredMapPair
{
private:
	TMap<KeyType, ValueType> KeyValueMap;
	TMap<ValueType, KeyType> ValueKeyMap;
public:

	FORCEINLINE const TMap<KeyType, ValueType>& GetConstKeyValueMap() const { return KeyValueMap; }
	FORCEINLINE const TMap<ValueType, KeyType>& GetConstValueKeyMap() const { return ValueKeyMap; }

	/*FORCEINLINE*/ bool AddPair(const KeyType& InKey, const ValueType& InValue)
	{
		ensure(ContainsKey(InKey) == ContainsValue(InValue));

		if (ContainsKey(InKey))
		{
			return false;
		}
		KeyValueMap.Add(InKey, InValue);
		ValueKeyMap.Add(InValue, InKey);
		ensure(KeyValueMap.Num() == ValueKeyMap.Num());
		return true;
	}

	FORCEINLINE bool RemoveByKey(const KeyType& InKey)
	{
		if (ContainsKey(InKey))
		{
			ValueKeyMap.Remove(KeyValueMap[InKey]);
			KeyValueMap.Remove(InKey);
			ensure(KeyValueMap.Num() == ValueKeyMap.Num());
			return true;
		}
		return false;
	}

	/*FORCEINLINE*/ bool RemoveByValue(const ValueType& InValue)
	{
		if (ContainsValue(InValue))
		{
			KeyValueMap.Remove(ValueKeyMap[InValue]);
			ValueKeyMap.Remove(InValue);
			ensure(KeyValueMap.Num() == ValueKeyMap.Num());
			return true;
		}
		return false;
	}

	FORCEINLINE bool ReplaceKey(const KeyType& InPrevKey, const KeyType& InNewKey)
	{
		if (ContainsKey(InPrevKey))
		{
			RemoveByKey(InNewKey);

			ValueType ValueAtKey = KeyValueMap[InPrevKey];
			ValueKeyMap[ValueAtKey] = InNewKey;

			KeyValueMap.Remove(InPrevKey);
			KeyValueMap.Add(InNewKey, ValueAtKey);
			ensure(KeyValueMap.Num() == ValueKeyMap.Num());
			return true;
		}
		return false;
	}

	/*FORCEINLINE*/ bool ReplaceValue(const ValueType& InPrevValue, const ValueType& InNewValue)
	{
		if (ContainsValue(InPrevValue))
		{
			if (ContainsValue(InNewValue))
			{
				Swap(KeyValueMap[ValueKeyMap[InPrevValue]], KeyValueMap[ValueKeyMap[InNewValue]]);
				Swap(ValueKeyMap[InPrevValue], ValueKeyMap[InNewValue]);
			}
			else
			{
				KeyType KeyAtPrevValue = ValueKeyMap[InPrevValue];
				
				RemoveByKey(KeyAtPrevValue);
				AddPair(KeyAtPrevValue, InNewValue);
				ensure(KeyValueMap.Num() == ValueKeyMap.Num());
			}
			return true;
		}
		return false;
	}

	FORCEINLINE ValueType* FindByKey(const KeyType& InKey) const { return const_cast<ValueType*>(KeyValueMap.Find(InKey)); }
	FORCEINLINE KeyType* FindByValue(const ValueType& InValue) const { return const_cast<KeyType*>(ValueKeyMap.Find(InValue)); }

	FORCEINLINE ValueType& FindRefByKey(const KeyType& InKey, const ValueType& InDefaultValue) const
	{
		if (const ValueType* FoundValuePtr = KeyValueMap.Find(InKey))
		{
			return const_cast<ValueType&>(*FoundValuePtr);
		}
		return const_cast<ValueType&>(InDefaultValue);
	}

	FORCEINLINE KeyType& FindRefByValue(const ValueType& InValue, const KeyType& InDefaultKey) const
	{
		if (const KeyType* FoundKeyPtr = ValueKeyMap.Find(InValue))
		{
			return const_cast<KeyType&>(*FoundKeyPtr);
		}
		return const_cast<KeyType&>(InDefaultKey);
	}

	FORCEINLINE int32 Num() const { return KeyValueMap.Num(); }
	FORCEINLINE bool IsEmpty() const { return KeyValueMap.IsEmpty(); }

	FORCEINLINE bool ContainsKey(const KeyType& InKey) const { return KeyValueMap.Contains(InKey); }
	FORCEINLINE bool ContainsValue(const ValueType& InValue) const { return ValueKeyMap.Contains(InValue); }

	FORCEINLINE void Empty(int32 InSlack = 0)
	{
		KeyValueMap.Empty(InSlack);
		ValueKeyMap.Empty(InSlack);
	}
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
