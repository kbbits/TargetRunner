// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "Engine/DataTable.h"
#include "ResourceType.generated.h"

/*
* Describes a specific type of resource.
*/
USTRUCT(BlueprintType)
struct FResourceType //: public FTableRowBase
{
	GENERATED_BODY()

public:

	// Required
	// This should be set as the row name when resource data is loaded from a data table.
	// This is the concatenated Category.Type.SubType of this resource type.
	// It should be in format : Category.Type.SubType
	//    ex: Metal.Copper  or  Metal.Copper.Oxidized
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Code;

	// Required - the high-level category of this resource type. ex: Metal
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Category;

	// Required - By default this must match the GoodsType.Name related to this resource.  ex: Copper
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
		//if (Code != Other.Code) return false;
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
	FORCEINLINE bool IsValid() const { return !(Code == NAME_None || Category == NAME_None || Type == NAME_None); }

	/** Is this resource type a Category resource type. i.e. it has no specific type. ex: Code=Metal Category=Metal Type=None  */
	FORCEINLINE bool IsCategory() const { return (Code != NAME_None && Category != NAME_None && Type == NAME_None); }
};
