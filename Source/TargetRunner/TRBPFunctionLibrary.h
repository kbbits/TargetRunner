// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DynSceneryBase.h"
#include "DynSceneLayer.h"
#include "TRBPFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API UTRBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	public:
	
		UFUNCTION(BlueprintCallable, Category="DynScenery")
			static TArray<FName> EvaluateDynSceneLayerToItemNames(FDynSceneLayer Layer, float NumMultiplier);
		
		UFUNCTION(BlueprintCallable, Category = "DynScenery")
			static FDynSceneLayerItem PickWeightedLayerItem(TArray<FDynSceneLayerItem> WeightedLayerItems);
};
