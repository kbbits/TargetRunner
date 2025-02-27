// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "GoodsDropChance.h"
#include "GoodsDropTable.generated.h"

/**
 * Each goods drop table is a collection of goods drop chances.
 * Note: when evaluated, a goods drop chance in a goods drop table may reference other goods drop tables. This allows the designer to 
 * set up some goods drop tables as categories of goods and other goods drop tables that have chances of including results from those
 * categorized goods drop tables.
 */
USTRUCT(BlueprintType)
struct TARGETRUNNER_API FGoodsDropTable : public FTableRowBase
{
	GENERATED_BODY()
	
public:

	FGoodsDropTable()
		: Super()
	{
		bAsWeightedList = false;
		MinWeightedPicks = 1;
		MaxWeightedPicks = 1;
	}

	// Name of this drop table
	// For clutter resource drops the name is in format ClutterResources<level tier>. ex: ClutterResources01
	// For clutter goods drops the name in format Clutter<level tier>. ex: Clutter01
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Name;

	/*
	Determines how the list of GoodsDropChances will be evaluated.
		False - All items in the list will have a % chance to be evaluated during drop based on the Chance property of the entry 0.0-1.0.
		True -  A number between MinWeightedPicks and MaxWeightedPicks of items in the list will be evaluated during drop. Each pick from the list is one GoodsDropChance, where each entry is given a weight equal to it's Chance variable, > 0.
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
