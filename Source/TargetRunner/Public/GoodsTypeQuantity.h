// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "GoodsType.h"
#include "GoodsTypeQuantity.generated.h"

USTRUCT(BlueprintType)
struct FGoodsTypeQuantity : public FTableRowBase
{
	GENERATED_BODY()

public:

	// Quantity of goods
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float Quantity;

	// The internal name of the goods
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FGoodsType GoodsData;

public:

	FGoodsTypeQuantity()
	{
		Quantity = 0.0f;
	}

	FGoodsTypeQuantity(const float NewQuantity, const FGoodsType NewGoodsData)
	{
		Quantity = NewQuantity;
		GoodsData = NewGoodsData;
	}

	FORCEINLINE bool operator==(const FGoodsTypeQuantity& Other) const
	{
		if (Quantity != Other.Quantity) return false;
		if (!(GoodsData == Other.GoodsData)) return false;
		return true;
	}

	FORCEINLINE bool operator==(const FGoodsTypeQuantity& Other)
	{
		if (Quantity != Other.Quantity) return false;
		if (!(GoodsData == Other.GoodsData)) return false;
		return true;
	}

	FORCEINLINE bool operator==(FGoodsTypeQuantity& Other)
	{
		if (Quantity != Other.Quantity) return false;
		if (!(GoodsData == Other.GoodsData)) return false;
		return true;
	}

	FORCEINLINE bool operator==(const FName& OtherName) const
	{
		if (GoodsData.Name != OtherName) return false;
		return true;
	}

	FORCEINLINE bool operator==(const FName& OtherName)
	{
		if (GoodsData.Name != OtherName) return false;
		return true;
	}

	FORCEINLINE bool operator==(FName& OtherName)
	{
		if (GoodsData.Name != OtherName) return false;
		return true;
	}

};