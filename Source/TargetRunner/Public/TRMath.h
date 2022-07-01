// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TargetRunner.h"
#include "MeshOption.h"
#include "TRMath.generated.h"

/**
 *
 */
UCLASS()
class TARGETRUNNER_API UTRMath : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

	// Computes the summation, ex: Summation(5) = 1+2+3+4+5 = 10,  Summation(6) = 1+2+3+4+5+6 = 16
	// But supports non-whole numbers. ex: Summation(5.5) = 13	
	UFUNCTION(BlueprintPure)
		static float Summation(const float Value);

    UFUNCTION(BlueprintPure)
        static FVector2D DirectionToOffsetVector(const ETRDirection Direction);

    UFUNCTION(BlueprintPure)
        static FIntPoint DirectionToOffset(const ETRDirection Direction);

    UFUNCTION(BlueprintPure)
        static ETRDirection OppositeDirection(const ETRDirection Direction);

	UFUNCTION(BlueprintPure)
		static TArray<FIntPoint> VectorsToIntPoints(const TArray<FVector2D>& Vector2DArray);

	UFUNCTION(BlueprintPure)
		static const FMeshOption& PickMeshOption(UPARAM(ref) FRandomStream& RandStream, const TArray<FMeshOption>& MeshOptions);

	// For an array where each item has a Weight (float) property that is > 0.0, this will pick one item from the array.
	// Item picked is a weighted random selection in the weighted list. 
	// Can return nullptr if list is empty or results in negative total weight.
	template<class T>
	static FORCEINLINE const T* PickOneFromWeightedList(FRandomStream& RandStream, const TArray<T>& WeightedItems, const float TotalWeight = -1.0f)
	{
		float TotalWeightedChance = TotalWeight;
		float PickedWeight = 0.0f;
		float CumulativeWeight = 0.0;
		// Sum our total weights if it was not passed in.
		if (TotalWeightedChance < 0.0f)
		{
			TotalWeightedChance = 0.0f;
			for (const T& WeightedItem : WeightedItems) {
				TotalWeightedChance += FMath::Abs(WeightedItem.Weight);
			}
		}
		if (TotalWeightedChance <= 0.0f) {
			return nullptr;
		}
		//PickedWeight = FMath::FRandRange(0.0, TotalWeightedChance);
		PickedWeight = RandStream.FRandRange(0.0f, TotalWeightedChance);
		//UE_LOG(LogMMGame, Log, TEXT("PickOne total weight: %f  picked weight %f"), TotalWeightedChance, PickedWeight);
		// Iterate through our list of items until we find the first one where the overall PickedWeight is less than our cumulative total weight of items iterated so far.
		for (const T& WeightedItem : WeightedItems)
		{
			CumulativeWeight += WeightedItem.Weight;
			if (PickedWeight <= CumulativeWeight)
			{
				//UE_LOG(LogMMGame, Log, TEXT("Picked: %s"), *WeightedItem.UniqueNameBase.ToString())
				return &WeightedItem;
			}
		}
		return nullptr;
	}

	// For an array where each item has a Chance (float) property that is > 0.0, this will pick one item from the array.
	// Item picked is a weighted random selection in the weighted list. 
	// Can return nullptr if list is empty or results in negative total weight.
	// ex: useful for ad-hock picking a GoodsDropChance from an array of GoodsDropChances.
	template<class T>
	static FORCEINLINE const T* PickOneFromChanceList(FRandomStream& RandStream, const TArray<T>& WeightedItems, const float TotalWeight = -1.0f)
	{
		float TotalWeightedChance = TotalWeight;
		float PickedWeight = 0.0f;
		float CumulativeWeight = 0.0;
		// Sum our total weights if it was not passed in.
		if (TotalWeightedChance < 0.0f)
		{
			TotalWeightedChance = 0.0f;
			for (const T& WeightedItem : WeightedItems) {
				TotalWeightedChance += FMath::Abs(WeightedItem.Chance);
			}
		}
		if (TotalWeightedChance <= 0.0f) {
			return nullptr;
		}
		//PickedWeight = FMath::FRandRange(0.0, TotalWeightedChance);
		PickedWeight = RandStream.FRandRange(0.0f, TotalWeightedChance);
		//UE_LOG(LogMMGame, Log, TEXT("PickOne total weight: %f  picked weight %f"), TotalWeightedChance, PickedWeight);
		// Iterate through our list of items until we find the first one where the overall PickedWeight is less than our cumulative total weight of items iterated so far.
		for (const T& WeightedItem : WeightedItems)
		{
			CumulativeWeight += WeightedItem.Chance;
			if (PickedWeight <= CumulativeWeight)
			{
				//UE_LOG(LogMMGame, Log, TEXT("Picked: %s"), *WeightedItem.UniqueNameBase.ToString())
				return &WeightedItem;
			}
		}
		return nullptr;
	}


};