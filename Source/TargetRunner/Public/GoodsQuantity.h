// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "Engine/DataTable.h"
#include "GoodsQuantity.generated.h"

USTRUCT(BlueprintType)
struct FGoodsQuantity //: public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float Quantity;

public:
	FGoodsQuantity()
	{
		Name = FName();
		Quantity = 0.0f;
	}

	FGoodsQuantity(const FName& NewName, const float NewQuantity)
	{
		Name = NewName;
		Quantity = NewQuantity;
	}

	FORCEINLINE bool operator==(const FGoodsQuantity& Other) const
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
};
