// Scientific Ways

#pragma once

#include "AvalancheTest.h"

#include "ATTypes_World.generated.h"

UENUM(BlueprintType, meta = (DisplayName = "[AT] Attachment Direction"))
enum class EATAttachmentDirection : uint8
{
	Front,
	Back,
	Right,
	Left,
	Top,
	Bottom
};

namespace EATAttachmentDirection_Utils
{
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
	float Health = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 SMI_Index = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Stability = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSet<EATAttachmentDirection> AttachmentDirections = TSet<EATAttachmentDirection>();

	bool IsTypeDataValid() const { return TypeData != nullptr; }
	bool IsAttachmentDataValid() const { return !AttachmentDirections.IsEmpty(); }
	bool IsAttachedTo(EATAttachmentDirection ToDirection) const { return AttachmentDirections.Contains(ToDirection); }
	bool HasMesh() const { return SMI_Index != INDEX_NONE; }

	FVoxelInstanceData(TObjectPtr<const class UATVoxelTypeData> InTypeData = nullptr, float InHealth = 1.0f)
		: TypeData(InTypeData), Health(InHealth) {}

	static const FVoxelInstanceData Invalid;
};

#if DEBUG_VOXELS
	#pragma optimize("", off)
#endif // DEBUG_VOXELS

template<typename Type>
struct TArraySetPair
{
private:
	TArray<Type> Array;
	TSet<Type> Set;
public:

	const TArray<Type>& GetConstArray() const { return Array; }
	const TSet<Type>& GetConstSet() const { return Set; }

	bool Add(Type InItem)
	{
		if (Contains(InItem))
		{
			return false;
		}
		else
		{
			Set.Add(InItem);
			Array.Add(InItem);
			ensure(Array.Num() == Set.Num());
			return true;
		}
	}

	bool Remove(Type InItem)
	{
		if (Contains(InItem))
		{
			Set.Remove(InItem);
			Array.Remove(InItem);
			ensure(Array.Num() == Set.Num());
			return true;
		}
		else
		{
			return false;
		}
	}

	bool Replace(Type InPrevItem, Type InNewItem)
	{
		if (Contains(InPrevItem))
		{
			Remove(InPrevItem);
			Add(InNewItem);
			return true;
		}
		else
		{
			return false;
		}
	}

	void AddFromOther(const TArraySetPair<Type>& InOther)
	{
		for (Type SampleOtherItem : InOther.Array)
		{
			Add(SampleOtherItem);
		}
	}

	void RemoveFromOther(const TArraySetPair<Type>& InOther)
	{
		for (Type SampleOtherItem : InOther.Array)
		{
			Remove(SampleOtherItem);
		}
	}

	void AddHeadTo(Type InDesiredElementsNum, TArraySetPair<Type>& InOutOther)
	{
		Type FirstIndex = 0;
		Type LastIndex = FMath::Min(InDesiredElementsNum, Array.Num()) - 1;

		for (Type SampleIndex = FirstIndex; SampleIndex <= LastIndex; ++SampleIndex)
		{
			InOutOther.Add(Array[SampleIndex]);
		}
	}

	void AddTailTo(Type InDesiredElementsNum, TArraySetPair<Type>& InOutOther)
	{
		Type FirstIndex = Array.Num() - 1;
		Type LastIndex = Array.Num() - FMath::Min(InDesiredElementsNum, Array.Num());

		for (Type SampleIndex = FirstIndex; SampleIndex >= LastIndex; --SampleIndex)
		{
			InOutOther.Add(Array[SampleIndex]);
		}
	}

	Type Num() const { return Array.Num(); }
	bool Contains(Type InItem) const { return Set.Contains(InItem); }
	bool IsEmpty() const { return Array.IsEmpty(); }

	void Empty(int32 InSlack = 0)
	{
		Array.Empty(InSlack);
		Set.Empty(InSlack);
	}
};

#if DEBUG_VOXELS
	#pragma optimize("", on)
#endif // DEBUG_VOXELS

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
	FTransform ChunkHighlightTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FColor LabelColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVoxelChunkDebugData_Entry> CommonEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVoxelChunkDebugData_Entry> AttachmentsEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVoxelChunkDebugData_Entry> StabilityEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InstanceLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVoxelChunkDebugData_Entry> InstanceEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform InstanceHighlightTransform;

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
