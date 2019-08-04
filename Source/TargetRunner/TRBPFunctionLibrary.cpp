// Fill out your copyright notice in the Description page of Project Settings.


#include "TRBPFunctionLibrary.h"

//TArray<FDynSceneLayerItemNames> UTRBPFunctionLibrary::EvaluateDynSceneTemplateToItemNames(FDynSceneTemplate Template, float PicksMultiplier = 1.0, float EnemyPicksMultiplier = 1.0)
//{
//}

TArray<FName> UTRBPFunctionLibrary::EvaluateDynSceneLayerToItemNames(UPARAM(ref) FRandomStream& RandStream, FDynSceneLayer Layer, float PicksMultiplier=1.0, float EnemyPicksMultiplier=1.0)
{
	TArray<FName> SceneryTemplateNames;
	FDynSceneLayerItem curItem;
	int32 itemCount;
	int32 picks;
	if (Layer.EnemyLayer)
		picks = FMath::RoundToInt(RandStream.FRandRange(Layer.MinPicks * EnemyPicksMultiplier, Layer.MaxPicks * EnemyPicksMultiplier));
	else
		picks = FMath::RoundToInt(RandStream.FRandRange(Layer.MinPicks * PicksMultiplier, Layer.MaxPicks * PicksMultiplier));
	
	/*if (Layer.EnemyLayer)
		picks = FMath::RoundToInt(FMath::FRandRange(Layer.MinPicks * EnemyPicksMultiplier, Layer.MaxPicks * EnemyPicksMultiplier));
	else
		picks = FMath::RoundToInt(FMath::FRandRange(Layer.MinPicks * PicksMultiplier, Layer.MaxPicks * PicksMultiplier));
	*/
	
	if (picks > 0) {
		for (int32 p = 1; p <= picks; p++) {
			curItem = PickWeightedLayerItem(RandStream, Layer.WeightedSceneryItems);
			if (curItem.ItemTemplateName != NAME_None)
			{
				itemCount = RandStream.FRandRange(curItem.QuantityMin, curItem.QuantityMax);
				for (int32 c = 1; c <= itemCount; c++) {
					SceneryTemplateNames.Add(curItem.ItemTemplateName);
				}
			}
		}
	}
	return SceneryTemplateNames;
}

FDynSceneLayerItem UTRBPFunctionLibrary::PickWeightedLayerItem(UPARAM(ref) FRandomStream& RandStream, TArray<FDynSceneLayerItem> WeightedLayerItems)
{
	FDynSceneLayerItem Item;
	float TotalWeight = 0;
	float AccumWeight = 0;
	float pick = 0;

	for (auto Itr(WeightedLayerItems.CreateIterator()); Itr; Itr++)
	{
		TotalWeight += Itr->ListWeight;
	}

	pick = RandStream.FRandRange(0, TotalWeight);
	for (auto Itr(WeightedLayerItems.CreateIterator()); Itr; Itr++)
	{
		if (pick <= (AccumWeight + Itr->ListWeight)) 
		{
			Item = WeightedLayerItems[Itr.GetIndex()];
			break;
		}
		else {
			AccumWeight += Itr->ListWeight;
		}
	}

	return Item;
}
