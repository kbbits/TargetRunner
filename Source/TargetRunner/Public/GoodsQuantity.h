// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "GoodsQuantity.generated.h"

USTRUCT(BlueprintType)
struct FGoodsQuantity : public FTableRowBase
{
	GENERATED_BODY()

public:
	// Default constructor
	FGoodsQuantity();
	// Initialized constructor
	FGoodsQuantity(FName NewName, float NewQuantity);

	// The internal name of the goods
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Name;

	// Quantity of goods
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float Quantity;
};