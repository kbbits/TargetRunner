// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "GoodsQuantityRange.generated.h"

USTRUCT(BlueprintType)
struct FGoodsQuantityRange : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName GoodsName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float QuantityMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float QuantityMax;
		
};