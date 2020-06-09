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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Category;

	// Required - By default this will match the GoodsType.Name related to this resource. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Type;

	// Optional
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName SubType;

	// Optional. Usually the GoodsType related to a ResourceType is GoodsType.Name == ResourceType.Type
	// Set this GoodsNameOverride to associate this ResourceType with a different Goods Type.
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	//	FName GoodsNameOverride;

public:
	FResourceType()
	{
		Category = FName();
		Type = FName();
		SubType = FName();
		//GoodsNameOverride = FName();
	}

	FResourceType(const FName& ResourceCategory, const FName& ResourceType)
	{
		Category = ResourceCategory;
		Type = ResourceType;
		SubType = FName();
		//GoodsNameOverride = FName();
	}

	FResourceType(const FName& ResourceCategory, const FName& ResourceType, const FName& ResourceSubType)
	{
		Category = ResourceCategory;
		Type = ResourceType;
		SubType = ResourceSubType;
		//GoodsNameOverride = FName();
	}

	bool operator==(const FResourceType& Other) const
	{
		if (Category != Other.Category) return false;
		if (Type != Other.Type) return false;
		if (SubType != Other.SubType) return false;
		//if (GoodsNameOverride != Other.GoodsNameOverride) return false;
		return true;
	}

	bool operator==(const FResourceType& Other)
	{
		if (Category != Other.Category) return false;
		if (Type != Other.Type) return false;
		if (SubType != Other.SubType) return false;
		//if (GoodsNameOverride != Other.GoodsNameOverride) return false;
		return true;
	}

	bool operator==(FResourceType& Other)
	{
		if (Category != Other.Category) return false;
		if (Type != Other.Type) return false;
		if (SubType != Other.SubType) return false;
		//if (GoodsNameOverride != Other.GoodsNameOverride) return false;
		return true;
	}

	bool operator!=(const FResourceType& Other) const
	{
		return !(*this == Other);
	}

	bool operator!=(const FResourceType& Other)
	{
		return !(*this == Other);
	}

	bool operator!=(FResourceType& Other)
	{
		return !(*this == Other);
	}
};
