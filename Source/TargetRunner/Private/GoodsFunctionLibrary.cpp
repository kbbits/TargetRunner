// Fill out your copyright notice in the Description page of Project Settings.


#include "GoodsFunctionLibrary.h"

TArray<FGoodsQuantity> UGoodsFunctionLibrary::MultiplyGoodsQuantities(const TArray<FGoodsQuantity> GoodsQuantities, const float Multiplier)
{
	TArray<FGoodsQuantity> NewQuantities;
	for (FGoodsQuantity CurQuantity : GoodsQuantities) 
	{
		NewQuantities.Add(FGoodsQuantity(CurQuantity.Name, CurQuantity.Quantity * Multiplier));
	}
	return NewQuantities;
}

TArray<FGoodsQuantity> UGoodsFunctionLibrary::AddGoodsQuantities(const TArray<FGoodsQuantity>& GoodsQuantitiesOne, const TArray<FGoodsQuantity>& GoodsQuantitiesTwo, const bool bNegateGoodsQuantitiesTwo)
{
	// May actually be better to just loop through both instead of converting to map first, then back again.
	TMap<FName, float> SummedQuantities = GoodsQuantityArrayToNameFloatMap(GoodsQuantitiesOne);
	for (FGoodsQuantity GoodsTwo : GoodsQuantitiesTwo)
	{
		float Delta = bNegateGoodsQuantitiesTwo ? -1.0f * GoodsTwo.Quantity : GoodsTwo.Quantity;
		if (SummedQuantities.Contains(GoodsTwo.Name))
		{
			SummedQuantities[GoodsTwo.Name] = SummedQuantities[GoodsTwo.Name] + Delta;
		}
		else
		{
			SummedQuantities.Add(GoodsTwo.Name, Delta);
		}
	}
	return NameQuantityMapToGoodsQuantityArray(SummedQuantities);
}

FGoodsQuantity UGoodsFunctionLibrary::GoodsQuantityFromRange(FRandomStream& RandStream, const FGoodsQuantityRange& QuantityRange, const float QuantityScale /* 0.0 - 1.0 */)
{
	FGoodsQuantity Goods(QuantityRange.GoodsName, 0.0f);
	//UE_LOG(LogTRGame, Log, TEXT("EvaluateGoodsQuantityRange for %s. Min: %f, Max: %f, QuantityScale: %f"), *QuantityRange.Name.ToString(), QuantityRange.QuantityMin, QuantityRange.QuantityMax, QuantityScale);
	//UE_LOG(LogTRGame, Log, TEXT("    RandStream current seed: %d"), RandStream.GetCurrentSeed());
	if (QuantityRange.QuantityMax > 0)
	{
		if (QuantityRange.QuantityMin == QuantityRange.QuantityMax)
		{
			Goods.Quantity = QuantityRange.QuantityMin;
		}
		else
		{
			// If QuantityScale is >= 0.0, then we determine the quantity by selecting a value that is QuantityScale % (lerp) between min and max values.
			if (QuantityScale >= 0.0f)
			{
				//UE_LOG(LogTRGame, Log, TEXT("  Quantity scale raw: %f"), QuantityScale);
				float ClampedScale = FMath::Min<float>(QuantityScale, 1.0f);
				Goods.Quantity = FMath::TruncToFloat(QuantityRange.QuantityMin + (ClampedScale * (QuantityRange.QuantityMax - QuantityRange.QuantityMin)));
				//UE_LOG(LogTRGame, Log, TEXT("  QuantityFromRange clamped scale: %f picked quantity: %f"), ClampedScale, Goods.Quantity);
			}
			else
			{
				Goods.Quantity = FMath::TruncToFloat(RandStream.FRandRange(0.0f, QuantityRange.QuantityMax - QuantityRange.QuantityMin) + QuantityRange.QuantityMin);
				//UE_LOG(LogTRGame, Log, TEXT("  QuantityFromRange random: picked quantity: %f"), Goods.Quantity);
			}
		}
	}
	return Goods;
}


TArray<FGoodsQuantity> UGoodsFunctionLibrary::GoodsQuantitiesFromRanges(FRandomStream& RandStream, const TArray<FGoodsQuantityRange>& QuantityRanges, const float QuantityScale /* 0.0 - 1.0 */)
{
	TArray<FGoodsQuantity> Goods;
	for (const FGoodsQuantityRange& GoodsRange : QuantityRanges)
	{
		Goods.Add(GoodsQuantityFromRange(RandStream, GoodsRange, QuantityScale));
	}
	return Goods;
}

