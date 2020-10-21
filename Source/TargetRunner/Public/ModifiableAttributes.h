#pragma once

#include "Engine/DataTable.h"
#include "ModifiableAttributes.generated.h"


UENUM(BlueprintType)
enum class EModifierType : uint8
{
	Scalar		UMETA(DisplayName = "Scalar"),
	Multiplier 	UMETA(DisplayName = "Multiplier")
};

UENUM(BlueprintType)
enum class EAttributeModifierType : uint8
{
	ValueMin	UMETA(DisplayName = "ValueMin"),
	ValueMax	UMETA(DisplayName = "ValueMax"),
	Rate		UMETA(DisplayName = "Rate")
};

UENUM(BlueprintType)
enum class EModifiableAttributeDisplayType : uint8
{
	PlainValue	UMETA(DisplayName = "PlainValue"),
	Percent 	UMETA(DisplayName = "Percent"),
	Boolean		UMETA(DisplayName = "Boolean")
};

USTRUCT(BlueprintType)
struct FModifier 
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		EModifierType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float Value;

	FModifier() 
	{
		Type = EModifierType::Multiplier;
		Value = 0.0f;
	}

	bool operator==(const FModifier& Other) const
	{
		if (Type != Other.Type) return false;
		if (Value != Other.Value) return false;
		return true;
	}
};

/*
	Represents a modifier that could be applied to an attriubute. (applied to a ActorAttributeComponent)
*/
USTRUCT(BlueprintType)
struct FAttributeModifier
{
	GENERATED_BODY()

public:
	
	// Identifying code for the type of attribute being modified. ex: Energy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName AttributeCode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		EAttributeModifierType EffectsValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FModifier Modifier;

	FAttributeModifier()
	{
		AttributeCode = FName();
		EffectsValue = EAttributeModifierType::ValueMax;
	}

	bool operator==(const FAttributeModifier& Other) const
	{
		if (AttributeCode != Other.AttributeCode) return false;
		if (EffectsValue != Other.EffectsValue) return false;
		if (!(Modifier == Other.Modifier)) return false;
		return true;
	}
};

/*
	Named sets of modifiers that are stored and added to in-game objects that will apply modifiers to others.
	Lists of these are intented to be kept in a DataTable and will describe the modifier affects of items etc.
*/
USTRUCT(BlueprintType)
struct FNamedModifierSet : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FAttributeModifier> Modifiers;

public:

	FNamedModifierSet()
	{
	}

	FNamedModifierSet(const FName NewName)
	{
		Name = NewName;
	}

	FNamedModifierSet(const FName NewName , const TArray<FAttributeModifier>& NewMods)
	{
		Name = NewName;
		Modifiers = NewMods;
	}

	bool operator==(const FNamedModifierSet& Other) const
	{
		if (Name != Other.Name) return false;
		if (Modifiers.Num() != Other.Modifiers.Num()) return false;
		for (int32 i = 0; i < Modifiers.Num(); i++)
		{
			if (!(Modifiers[i] == Other.Modifiers[i])) return false;
		}
		return true;
	}
};
