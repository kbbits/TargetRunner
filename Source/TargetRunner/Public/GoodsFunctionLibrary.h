// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NamedPrimitiveTypes.h"
#include "GoodsQuantity.h"
#include "GoodsQuantityRange.h"
#include "GoodsTypeQuantity.h"
#include "GoodsDropChance.h"
#include "TargetRunner.h"
#include "GoodsFunctionLibrary.generated.h"

/**
 * A collection of helper functions to manipulate GoodsQuantity and related structs.
 */
UCLASS()
class TARGETRUNNER_API UGoodsFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		
public:

	// Multiply the quantity of each good by the given multiplier.
	UFUNCTION(BlueprintPure, Category = "Goods")
		static TArray<FGoodsQuantity> MultiplyGoodsQuantities(const TArray<FGoodsQuantity> GoodsQuantities, const float Multiplier, const bool bTruncateQuantities = true);


	// Adds the quantities of the two GoodsQuantities arrays.
	// bNegateGoodsQuantitesTwo - if true, this will subtract GoodsQuantitiesTwo from GoodQuantitiesOne.
	UFUNCTION(BlueprintPure, Category = "Goods")
		static TArray<FGoodsQuantity> AddGoodsQuantities(const TArray<FGoodsQuantity>& GoodsQuantitiesOne, const TArray<FGoodsQuantity>& GoodsQuantitiesTwo, const bool bNegateGoodsQuantitiesTwo = false);


	// Transform a goods quantity range to a goods quantity. 
	// If optional QuantityScale is provided, the quantity will be mapped from min to max according to the scale (0.0 to 1.0) instead of determining randomly.
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


	// Create an array of GoodsQuantities from a FTRNamedFloats.
	UFUNCTION(BlueprintPure, Category = "Utilities| Goods")
		static FORCEINLINE TArray<FGoodsQuantity> NamedFloatArrayToGoodsQuantityArray(TArray<FTRNamedFloat> NamedFloatArray)
	{
		TArray<FGoodsQuantity> GoodsQuantityArray;
		for (FTRNamedFloat& NamedItem : NamedFloatArray)
		{
			GoodsQuantityArray.Add(FGoodsQuantity(NamedItem.Name, NamedItem.Quantity));
		}
		return GoodsQuantityArray;
	}


	// Create a map of Name->FGoodsQuantity from a FTRNamedFloats, where each key is the NamedFloat's name.
	UFUNCTION(BlueprintPure, Category = "Utilities| Goods")
	static FORCEINLINE TMap<FName, FGoodsQuantity> NamedFloatArrayToGoodsQuantityMap(TArray<FTRNamedFloat> NamedFloatArray)
	{
		TMap<FName, FGoodsQuantity> GoodsQuantityMap;
		for (FTRNamedFloat& NamedItem : NamedFloatArray)
		{
			GoodsQuantityMap.Add(NamedItem.Name, FGoodsQuantity(NamedItem.Name, NamedItem.Quantity));
		}
		return GoodsQuantityMap;
	}


	// Create a map of Name -> Quantity from an array of FGoodsQuantities.
	UFUNCTION(BlueprintPure, Category = "Utilities| Goods")
	static FORCEINLINE TMap<FName, float> GoodsQuantityArrayToNameFloatMap(TArray<FGoodsQuantity> GoodsQuantityArray)
	{
		return NamedQuantitiesToFloatMap(GoodsQuantityArray);
	}


	// Returns the quantity of goods of the given name contained in the supplied goods quantity array
	UFUNCTION(BlueprintPure, Category = "Utilities| Goods")
	static FORCEINLINE float CountInGoodsQuantityArray(const FName GoodsName, const TArray<FGoodsQuantity>& GoodsQuantities, bool& bFound)
	{
		const FGoodsQuantity* FoundGoods = GoodsQuantities.FindByKey<FName>(GoodsName);
		if (FoundGoods)
		{
			bFound = true;
			return FoundGoods->Quantity;
		}
		bFound = false;
		return 0.0f;
	}


	// Returns the quantity of goods of the given name contained in the supplied goods quantity set
	UFUNCTION(BlueprintPure, Category = "Utilities| Goods")
	static FORCEINLINE float CountInGoodsQuantitySet(const FName GoodsName, const FGoodsQuantitySet& GoodsQuantitySet, bool& bFound)
	{
		return CountInGoodsQuantityArray(GoodsName, GoodsQuantitySet.Goods, bFound);
	}


	// Finds the GoodsTypeQuantity with the given goods name within the array.
	// Also returns the quantity of found item as convenience.
	UFUNCTION(BlueprintPure, Category = "Utilities| Goods")
	static FORCEINLINE float GoodsTypeDataInGoodsTypeQuantityArray(const FName GoodsName, const TArray<FGoodsTypeQuantity>& GoodsTypeQuantities, FGoodsTypeQuantity& FoundGoodsTypeQuantity, bool& bFound)
	{
		const FGoodsTypeQuantity* TmpGTQ = GoodsTypeQuantities.FindByKey<FName>(GoodsName);
		if (TmpGTQ)
		{
			FoundGoodsTypeQuantity = *TmpGTQ;
			bFound = true;
			return TmpGTQ->Quantity;
		}
		else
		{
			bFound = false;
			return 0.f;
		}
	}


	// For an array where each item has a Chance (float) property that is > 0.0, this will pick one item from the array.
	// Item picked is a weighted random selection in the weighted list. 
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
				TotalWeightedChance += FMath::Abs(WeightedItem.Chance);
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
