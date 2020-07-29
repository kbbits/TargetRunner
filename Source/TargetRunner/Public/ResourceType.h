// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "Engine/DataTable.h"
#include "ResourceType.generated.h"

USTRUCT(BlueprintType)
struct FResourceType //: public FTableRowBase
{
	GENERATED_BODY()

public:

	// Required
	// This should be set as the row name when resource data is loaded from the data table.
	// It should be in format : Category.Type.SubType
	//    ex: Metal.Copper  or  Metal.Copper.Oxidized
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Code;

	// Required
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Category;

	// Required - By default this must match the GoodsType.Name related to this resource. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Type;

	// Optional
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName SubType;

public:
	FResourceType()
	{
		Code = FName();
		Category = FName();
		Type = FName();
		SubType = FName();
	}

	FResourceType(const FName& _Code, const FName& _Category, const FName& _Type, const FName& _SubType)
	{
		Code = _Code;
		Category = _Category;
		Type = _Type;
		SubType = _SubType;
	}
		
	//FResourceType(const FString& ResourceCode)
	//{
	//	TArray<FString> CodeStrings;
	//	ResourceCode.ParseIntoArray(CodeStrings, TEXT("."), false);
	//	Code = FName(*ResourceCode);
	//	Category = CodeStrings.IsValidIndex(0) ? FName(*CodeStrings[0]) : FName();
	//	Type = CodeStrings.IsValidIndex(1) ? FName(*CodeStrings[1]) : FName();
	//	SubType = CodeStrings.IsValidIndex(2) ? FName(*CodeStrings[2]) : FName();
	//}

	//FResourceType(const FName& ResourceCode)
	//{
	//	FResourceType(ResourceCode.ToString());
	//}
	
		
	FORCEINLINE bool operator==(const FResourceType& Other) const
	{
		if (Code != Other.Code) return false;
		if (Category != Other.Category) return false;
		if (Type != Other.Type) return false;
		if (SubType != Other.SubType) return false;
		//if (GoodsNameOverride != Other.GoodsNameOverride) return false;
		return true;
	}

	FORCEINLINE bool operator==(const FResourceType& Other)
	{
		if (Category != Other.Category) return false;
		if (Type != Other.Type) return false;
		if (SubType != Other.SubType) return false;
		//if (GoodsNameOverride != Other.GoodsNameOverride) return false;
		return true;
	}

	FORCEINLINE bool operator==(FResourceType& Other)
	{
		if (Category != Other.Category) return false;
		if (Type != Other.Type) return false;
		if (SubType != Other.SubType) return false;
		//if (GoodsNameOverride != Other.GoodsNameOverride) return false;
		return true;
	}

	FORCEINLINE bool operator!=(const FResourceType& Other) const
	{
		return !(*this == Other);
	}

	FORCEINLINE bool operator!=(const FResourceType& Other)
	{
		return !(*this == Other);
	}

	FORCEINLINE bool operator!=(FResourceType& Other)
	{
		return !(*this == Other);
	}

public:
	FORCEINLINE bool IsValid() { return !(Code == NAME_None || Category == NAME_None || Type == NAME_None); }
};
