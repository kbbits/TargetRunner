// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ResourceTypeData.h"
#include "ResourceType.h"
#include "ResourceRateFilter.h"
#include "ResourceQuantity.h"
#include "TargetRunner.h"
#include "ResourceFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API UResourceFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		
public:

	// Determines how well the ResourceRateFilter matches the ResourceType
	UFUNCTION(BlueprintPure, Category = "Resource Functions")
		static ETRResourceMatch ResourceFilterMatch(const FResourceType& ResourceType, const FResourceRateFilter& ResourceFilter);

	// Finds the most specific resource rate filter in the array of FResourceRateFilters matching the given resource type.
	// Returns true if a match was found, false otherwise.
	// NOTE: Any fields in the ResourceRates ResourceType that are "None" (NAME_NONE) are wildcards.
	// Ex: If ResourceFilters contains these: [{Category: "Metal", Type: "Copper", SubType: "None", Rate: 2.0}, {Category: "Metal", Type: "None", SubType: "None", Rate: 1.0}, {Category: "Metal", Type: "Iron", SubType: "None", Rate: 4.0}]
	//    Then calling this with a ResourceType {Category: "Metal", Type: "Copper", SubType: "None"} will return the first rate with Rate=2.0, FoundMatchDegree = ETRResourceMatch::Type
	//    Calling this with a ResourceType {Category: "Metal", Type: "Aluminum", SubType: "None"} will return the second rate with Rate=1.0, FoundMatchDegree = ETRResourceMatch::Category
	//    Calling this with a ResourceType {Category: "Energy", Type: "Photon", SubType: "None"} will return a "None" rate with Rate=0.0, FoundMatchDegree = ETRResourceMatch::None
	UFUNCTION(BlueprintPure, Category = "Resource Functions")
		static bool FindResourceRateFilter(const TArray<FResourceRateFilter>& ResourceFilters, const FResourceType& TargetType, FResourceRateFilter& FoundRate, ETRResourceMatch& FoundMatchDegree, const ETRResourceMatch& MinimumMatchDegree = ETRResourceMatch::Category);

	// Adds two arrays of FResourceQuantites into a single summed array with one entry for each resource type
	UFUNCTION(BlueprintPure, Category = "Resource Functions")
		static TArray<FResourceQuantity> AddResourceQuantities(const TArray<FResourceQuantity>& QuantitiesOne, const TArray<FResourceQuantity>& QuantitiesTwo);

	// Helper to get the Goods.Name for the specified ResourceType
	// If we change to support ResourceTypeData mapping to Goods.Name by something other than Type, we'll need to change this.
	UFUNCTION(BlueprintPure, Category = "Resource Functions")
		static FName GoodsNameForResource(const FResourceType& ResourceType);

	// Helper to convert a ResourceTypeData into a ResourceType
	UFUNCTION(BlueprintPure, Category = "Resource Funtions")
		static void ResourceTypeForData(const FResourceTypeData& ResourceData, FResourceType& ResourceType);

};
