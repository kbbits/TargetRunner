// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "GoodsDropChance.h"
#include "GoodsDropTable.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct TARGETRUNNER_API FGoodsDropTable : public FTableRowBase
{
	GENERATED_BODY()
	
public:

	FGoodsDropTable()
		: Super()
	{
		MinWeightedPicks = 1;
		MaxWeightedPicks = 1;
	}

	// Name of this drop table
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Name;

	/*
	Determines how the list of GoodsDropChances will be evaluated.
		False - Each item in the list will have a % chance to be evaluated during drop based on the Chance variable of the entry 0 to 1.
		True - A number between MinWeightedPicks and MaxWeightedPicks of items in the list will be evaluated during drop. Each pick is determined by picking from the list where each entry is given a weight equal to it's Chance variable, > 0.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		bool bAsWeightedList;

	// If bAsWeightedList == true, this is the minimum number of picks from the weighted list to make.
	// Default = 1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 MinWeightedPicks;

	// If bAsWeightedList == true, this is the maximum number of picks from the weighted list to make.
	// Default = 1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 MaxWeightedPicks;

	// List of goods drop chances. Each entry has a chance of being dropped when this table is evaluated. See: AsWeightedList.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, meta = (TitleProperty = "GoodsQuantities"))
		TArray<FGoodsDropChance> GoodsOddsList;

};
