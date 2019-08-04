// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RandomStream.h"
#include "DynSceneryBase.h"
#include "DynSceneLayer.h"
#include "DynSceneTemplate.h"
#include "TRBPFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API UTRBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	public:
	
		//UFUNCTION(BlueprintCallable, Category = "DynScenery")
		//	static TArray<FDynSceneLayerItemNames> EvaluateDynSceneTemplateToItemNames(FDynSceneLayer Layer, float PicksMultiplier, float EnemyPicksMultiplier);

		UFUNCTION(BlueprintCallable, Category="DynScenery")
			static TArray<FName> EvaluateDynSceneLayerToItemNames(FRandomStream RandStream, FDynSceneLayer Layer, float PicksMultiplier, float EnemyPicksMultiplier);
		
		UFUNCTION(BlueprintCallable, Category = "DynScenery")
			static FDynSceneLayerItem PickWeightedLayerItem(FRandomStream RandStream, TArray<FDynSceneLayerItem> WeightedLayerItems);
};
