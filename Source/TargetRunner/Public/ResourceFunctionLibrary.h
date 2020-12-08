// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ResourceTypeData.h"
#include "ResourceTypeDataCollection.h"
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

	// Does the FResourceType represent a valid resource?.
	//   i.e. a valid Code, Category and Type
	UFUNCTION(BlueprintPure, Category = "Resource Functions")
		static bool IsResourceTypeValid(const FResourceType& ResourceType);

	// Determines how well the ResourceRateFilter matches the ResourceType
	UFUNCTION(BlueprintPure, Category = "Resource Functions")
		static ETRResourceMatch ResourceFilterMatch(const FResourceType& ResourceType, const FResourceRateFilter& ResourceFilter);

	// Finds the most specific resource rate filter in the array of FResourceRateFilters matching the given resource type.
	// Returns true if a match was found, false otherwise.
	// NOTE: Any fields in the ResourceFilters ResourceType that are "None" (NAME_NONE) are wildcards and cause all more granular filters to behave as None.
	// Ex: If ResourceFilters contains these: [{Category: "Metal", Type: "Copper", SubType: "None", Rate: 2.0}, {Category: "Metal", Type: "None", SubType: "None", Rate: 1.0}, {Category: "Metal", Type: "Iron", SubType: "None", Rate: 4.0}]
	//    Then calling this with a ResourceType {Category: "Metal", Type: "Copper", SubType: "None"} will return the first rate with Rate=2.0, FoundMatchDegree = ETRResourceMatch::Type
	//    Calling this with a ResourceType {Category: "Metal", Type: "Aluminum", SubType: "None"} will return the second rate with Rate=1.0, FoundMatchDegree = ETRResourceMatch::Category
	//    Calling this with a ResourceType {Category: "Energy", Type: "Photon", SubType: "None"} will return a "None" rate with Rate=0.0, FoundMatchDegree = ETRResourceMatch::None
	UFUNCTION(BlueprintPure, Category = "Resource Functions")
		static bool FindResourceRateFilter(const TArray<FResourceRateFilter>& ResourceFilters, const FResourceType& TargetType, FResourceRateFilter& FoundRate, ETRResourceMatch& FoundMatchDegree, const ETRResourceMatch& MinimumMatchDegree = ETRResourceMatch::Category);
	
	// Similar to FindResourceRateFilter, but this will return the highest rate among all matching resource types, not the most closely matching one.
	UFUNCTION(BlueprintPure, Category = "Resource Functions")
		static bool FindBestResourceRateFilter(const TArray<FResourceRateFilter>& ResourceFilters, const FResourceType& TargetType, FResourceRateFilter& FoundRate, ETRResourceMatch& FoundMatchDegree, const ETRResourceMatch& MinimumMatchDegree = ETRResourceMatch::Category);

	// Adds two arrays of FResourceQuantites into a single summed array with one entry for each resource type
	UFUNCTION(BlueprintPure, Category = "Resource Functions")
		static TArray<FResourceQuantity> AddResourceQuantities(const TArray<FResourceQuantity>& QuantitiesOne, const TArray<FResourceQuantity>& QuantitiesTwo);

	// Multiplies the Quantity of each FResourceQuantity in the array by the given multiplier.
	// If bTruncate is true, the multplied quantities will be truncated to an integer value.
	UFUNCTION(BlueprintPure, Category = "Resource Functions")
		static void MultiplyResourceQuantity(const TArray<FResourceQuantity>& Quantities, const float Multiplier, const bool bTruncate, TArray<FResourceQuantity>& MultipliedQuantities);

	// Helper to get the Goods.Name for the specified ResourceType
	// This currently maps the Resource.Code to Goods.Name.
	UFUNCTION(BlueprintPure, Category = "Resource Functions")
		static FName GoodsNameForResource(const FResourceType& ResourceType);

	// Helper to convert a ResourceType code into a ResourceType struct
	UFUNCTION(BlueprintPure, Category = "Resource Funtions")
		static void ResourceTypeForCode(const FName& ResourceCode, FResourceType& ResourceType);
	
	// Helper to convert a ResourceTypeData into a ResourceType
	UFUNCTION(BlueprintPure, Category = "Resource Funtions")
		static void ResourceTypeForData(const FResourceTypeData& ResourceData, FResourceType& ResourceType);

	// Searches the data table and returns all ResourceTypeDatas where MinTier <= Resource.Tier <= MaxTier.
	// Returns the number of resources found.
	UFUNCTION(BlueprintPure, Category = "Resource Functions")
		static int32 ResourceDataInTier(const UDataTable* ResourceDataTable, const float MinTier, const float MaxTier, TArray<FResourceTypeData>& ResourceData);

	// Searches the data table and returns all ResourceTypeDatas where MinTier <= Resource.Tier <= MaxTier.
	// Returns ResourceData as a map of Tier -> ResourceTypeDataCollection, which will contain resources of that tier.
	// Returns the number of resources found.
	UFUNCTION(BlueprintPure, Category = "Resource Functions")
		static int32 ResourceDataByTier(const UDataTable* ResourceDataTable, const float MinTier, const float MaxTier, TMap<int32, FResourceTypeDataCollection>& ResourceDataByTier);

	// Calculates the amount of resources that should be extracted with the given extraction rates.
	//	PercentOfTotal - 0.0 to 1.0 is 0% to 100%, but can be over 1.0. The percent of the resources to extract. ex: if 25% of object's health was damaged, could extract 25% (0.25) of the resources.
	// Makes no changes to current resource quantities.
	// Returns true if any resources were calculated to be extracted.
	UFUNCTION(BlueprintCallable, Category = "Resource Functions")
		static bool CalculateExtractedResources(const TArray<FResourceQuantity>& ExtractableResources, const float PercentOfTotal, const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities);

	// Similar to CalculateExtractedResources, but this also subtracts the extracted quantities from ExtractableResources.
	UFUNCTION(BlueprintCallable, Category = "Resource Functions")
		static bool SubtractExtractedResources(UPARAM(ref) TArray<FResourceQuantity>& ExtractableResources, const float PercentOfTotal, const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities);
};
