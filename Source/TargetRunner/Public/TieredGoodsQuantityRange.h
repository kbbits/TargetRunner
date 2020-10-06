// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "GoodsQuantityRange.h"
#include "TieredGoodsQuantityRange.generated.h"

USTRUCT(BlueprintType)
struct FTieredGoodsQuantityRange : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 Tier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, meta = (TitleProperty = "GoodsName"))
		TArray<FGoodsQuantityRange> QuantityRanges;	
};