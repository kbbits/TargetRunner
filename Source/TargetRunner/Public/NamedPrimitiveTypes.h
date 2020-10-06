#pragma once

#include "NamedPrimitiveTypes.generated.h"

// 
// A collection of named primitive types. Useful for serialization.

USTRUCT(BlueprintType)
struct FTRNamedFloat
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float Quantity;

public:
	FTRNamedFloat()
	{
		Name = FName();
		Quantity = 0.0f;
	}

	FTRNamedFloat(const FName& NewName, const float NewQuantity)
	{
		Name = NewName;
		Quantity = NewQuantity;
	}

	FORCEINLINE bool operator==(const FTRNamedFloat& Other) const
	{
		if (Name != Other.Name) return false;
		if (Quantity != Other.Quantity) return false;
		return true;
	}

	FORCEINLINE bool operator==(const FTRNamedFloat& Other)
	{
		if (Name != Other.Name) return false;
		if (Quantity != Other.Quantity) return false;
		return true;
	}

	FORCEINLINE bool operator==(FTRNamedFloat& Other)
	{
		if (Name != Other.Name) return false;
		if (Quantity != Other.Quantity) return false;
		return true;
	}

	FORCEINLINE bool operator==(const FName& OtherName) const
	{
		if (Name != OtherName) return false;
		return true;
	}

	FORCEINLINE bool operator==(const FName& OtherName)
	{
		if (Name != OtherName) return false;
		return true;
	}

	FORCEINLINE bool operator==(FName& OtherName)
	{
		if (Name != OtherName) return false;
		return true;
	}

};


USTRUCT(BlueprintType)
struct FTRNamedInt
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 Quantity;

public:
	FTRNamedInt()
	{
		Name = FName();
		Quantity = 0;
	}

	FTRNamedInt(const FName& NewName, const int32 NewQuantity)
	{
		Name = NewName;
		Quantity = NewQuantity;
	}

	FORCEINLINE bool operator==(const FTRNamedInt& Other) const
	{
		if (Name != Other.Name) return false;
		if (Quantity != Other.Quantity) return false;
		return true;
	}

	FORCEINLINE bool operator==(const FTRNamedInt& Other)
	{
		if (Name != Other.Name) return false;
		if (Quantity != Other.Quantity) return false;
		return true;
	}

	FORCEINLINE bool operator==(FTRNamedInt& Other)
	{
		if (Name != Other.Name) return false;
		if (Quantity != Other.Quantity) return false;
		return true;
	}

	FORCEINLINE bool operator==(const FName& OtherName) const
	{
		if (Name != OtherName) return false;
		return true;
	}

	FORCEINLINE bool operator==(const FName& OtherName)
	{
		if (Name != OtherName) return false;
		return true;
	}

	FORCEINLINE bool operator==(FName& OtherName)
	{
		if (Name != OtherName) return false;
		return true;
	}

};