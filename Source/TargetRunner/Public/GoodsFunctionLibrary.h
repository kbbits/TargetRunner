// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GoodsQuantity.h"
#include "GoodsQuantityRange.h"
#include "GoodsDropChance.h"
#include "TargetRunner.h"
#include "GoodsFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API UGoodsFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		
public:

	// Multiply the quantity of each good by the given multiplier.
	UFUNCTION(BlueprintPure, Category = "Goods")
		static TArray<FGoodsQuantity> MultiplyGoodsQuantities(const TArray<FGoodsQuantity> GoodsQuantities, const float Multiplier);

	// Adds the quantities of the two GoodsQuantities arrays.
	// bNegateGoodsQuantitesTwo - if true, this will subtract GoodsQuantitiesTwo from GoodQuantitiesOne.
	UFUNCTION(BlueprintPure, Category = "Goods")
		static TArray<FGoodsQuantity> AddGoodsQuantities(const TArray<FGoodsQuantity>& GoodsQuantitiesOne, const TArray<FGoodsQuantity>& GoodsQuantitiesTwo, const bool bNegateGoodsQuantitiesTwo = false);

	// Transform a goods quantity range to a goods quantity. 
	// If optional QuantityScale is provided, the quantity will be mapped from min to max according to the scale (0-1) instead of determining randomly.
	UFUNCTION(BlueprintCallable, Category = "Goods")
		static FGoodsQuantity GoodsQuantityFromRange(UPARAM(ref) FRandomStream& RandStream, const FGoodsQuantityRange& QuantityRange, const float QuantityScale = -1.0f /* 0.0 - 1.0 */);

	// Transform an array of goods quantity ranges to an array of goods quantities. 
	// If optional QuantityScale is provided, each quantity will be mapped from min to max according to the scale instead of determining randomly.
	UFUNCTION(BlueprintCallable, Category = "Goods")
		static TArray<FGoodsQuantity> GoodsQuantitiesFromRanges(UPARAM(ref) FRandomStream& RandStream, const TArray<FGoodsQuantityRange>& QuantityRanges, const float QuantityScale = -1.0f /* 0.0 - 1.0 */);

	// Create an array of FGoodsQuantities from a map of Name (key) and Quantity (float value).
	UFUNCTION(BlueprintPure, Category = "Utilities| Goods")
	static FORCEINLINE TArray<FGoodsQuantity> NameQuantityMapToGoodsQuantityArray(TMap<FName, float> NamedItemsMap)
	{
		TArray<FGoodsQuantity> GoodsQuantities;
		for (TPair<FName, float> NamedItemElem : NamedItemsMap)
		{
			GoodsQuantities.Add(FGoodsQuantity(NamedItemElem.Key, NamedItemElem.Value));
		}
		return GoodsQuantities;
	}

	// Create a map of Name -> FGoodsQuantity from an array of FGoodsQuantities.
	UFUNCTION(BlueprintPure, Category = "Utilities| Goods")
	static FORCEINLINE TMap<FName, FGoodsQuantity> GoodsQuantityArrayToMap(TArray<FGoodsQuantity> GoodsQuantityArray)
	{
		return NamedItemsToMap(GoodsQuantityArray);
	}

	// Create a map of Name -> Quantity from an array of FGoodsQuantities.
	UFUNCTION(BlueprintPure, Category = "Utilities| Goods")
	static FORCEINLINE TMap<FName, float> GoodsQuantityArrayToNameFloatMap(TArray<FGoodsQuantity> GoodsQuantityArray)
	{
		return NamedQuantitiesToFloatMap(GoodsQuantityArray);
	}

	// Create a map from an array. Each element in the array must have a Name (FName) property. (ex: FGoodsQuantity)
	// New map has Name as key and element as value.  Collisions of Name in array are overwritten, last one wins.
	template<class T>
	static FORCEINLINE TMap<FName, T> NamedItemsToMap(TArray<T> NamedItems)
	{
		TMap<FName, T> NamedItemMap;
		for (T& NamedItem : NamedItems)
		{
			NamedItemMap.Add(NamedItem.Name, NamedItem);
		}
		return NamedItemMap;
	}

	// Create a map from an array. Each element in array must have a Name (FName) and a Quantity (int) property. (ex: FGoodsQuantity)
	// New map will have Name as key and Quantity as value. Collisions of Name in array are overwritten, last one wins.
	// ex: Useful for transforming an array of GoodsQuantities to a map with each key as the GoodsQuantity.Name and the Quantity as the value.
	template<class T>
	static FORCEINLINE TMap<FName, int32> NamedQuantitiesToCountMap(TArray<T> NamedItems)
	{
		TMap<FName, int32> NamedCountMap;
		for (T& NamedItem : NamedItems)
		{
			NamedCountMap.Add(NamedItem.Name, static_cast<int32>(NamedItem.Quantity));
		}
		return NamedCountMap;
	}

	// Create a map from an array. Each element in array must have a Name (FName) and a Quantity (float) property. (ex: FGoodsQuantity)
	// New map will have Name as key and Quantity as value. Collisions of Name in array are overwritten, last one wins.
	// ex: Useful for transforming an array of GoodsQuantities to a map with each key as the GoodsQuantity.Name and the Quantity as the value.
	template<class T>
	static FORCEINLINE TMap<FName, float> NamedQuantitiesToFloatMap(TArray<T> NamedItems)
	{
		TMap<FName, float> NamedFloatMap;
		for (T& NamedItem : NamedItems)
		{
			NamedFloatMap.Add(NamedItem.Name, static_cast<float>(NamedItem.Quantity));
		}
		return NamedFloatMap;
	}

	// For an array where each item has a Chance (float) property that is > 0.0, this will pick one item from the array.
	// Item picked is a random selection in the weighted list. 
	// Can return nullptr if list is empty or results in negative total weight.
	// ex: useful for ad-hock picking a GoodsDropChance from an array of GoodsDropChances.
	template<class T>
	static FORCEINLINE T* PickOneFromWeightedList(TArray<T>& WeightedItems, const float TotalWeight = -1.0f)
	{
		float TotalWeightedChance = TotalWeight;
		float PickedWeight = 0.0f;
		float CumulativeWeight = 0.0;
		// Sum our total weights if it was not passed in.
		if (TotalWeightedChance < 0.0f)
		{
			TotalWeightedChance = 0.0f;
			for (T& WeightedItem : WeightedItems)
			{
				TotalWeightedChance += WeightedItem.Chance;
			}
		}
		if (TotalWeightedChance <= 0.0f)
		{
			return nullptr;
		}

		PickedWeight = FMath::FRandRange(0.0, TotalWeightedChance);
		//UE_LOG(LogTRGame, Log, TEXT("PickOne total weight: %f  picked weight %f"), TotalWeightedChance, PickedWeight);

		// Iterate through our list of items until we find the first one where the overall PickedWeight is less than our cumulative total weight of items iterated so far.
		for (T& WeightedItem : WeightedItems)
		{
			CumulativeWeight += WeightedItem.Chance;
			if (PickedWeight <= CumulativeWeight)
			{
				//UE_LOG(LogTRGame, Log, TEXT("Picked: %s"), *WeightedItem.UniqueNameBase.ToString())
				return &WeightedItem;
			}
		}
		return nullptr;
	}
};
