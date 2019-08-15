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

};