// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "GoodsQuantity.h"
#include "GoodsQuantityRange.h"
#include "GoodsDropChance.generated.h"

USTRUCT(BlueprintType)
struct FGoodsDropChance : public FTableRowBase
{
	GENERATED_BODY()

public:
	// See: GoodsDropTable.AsWeightedList for use.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float Chance;
	
	// When this instance is evaluaed during drop, all goods in this list will be evaluated and included in the drop.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FGoodsQuantityRange> GoodsQuantities;

	// Array of names of GoodsDropTables to also be evaluated and included during drop.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FName> OtherGoodsDrops;
};