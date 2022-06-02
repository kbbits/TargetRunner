// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceFunctionLibrary.h"
#include "..\Public\ResourceFunctionLibrary.h"


bool UResourceFunctionLibrary::IsResourceTypeValid(const FResourceType& ResourceType)
{
	return ResourceType.IsValid();
}


bool UResourceFunctionLibrary::ContainsAnyQuantity(const TArray<FResourceQuantity>& ResourceQuantities, const float TestQuantity)
{
	if (TestQuantity <= 0.0f)
	{
		for (FResourceQuantity TmpQuantity : ResourceQuantities) {
			if (TmpQuantity.Quantity > 0.0f) { return true; }
		}
	}
	else
	{
		for (FResourceQuantity TmpQuantity : ResourceQuantities) {
			if (TmpQuantity.Quantity >= TestQuantity) { return true; }
		}
	}
	return false;
}


float UResourceFunctionLibrary::GetResourceQuantity(const TArray<FResourceQuantity>& ResourceQuantities, const FResourceType OfThisType)
{
	float QuantityTotal = 0.0f;
	for (FResourceQuantity CurQuantity : ResourceQuantities) {
		if (CurQuantity.ResourceType == OfThisType) { QuantityTotal += CurQuantity.Quantity; }
	}
	return QuantityTotal;
}


ETRResourceMatch UResourceFunctionLibrary::ResourceFilterMatch(const FResourceType& ResourceType, const FResourceRateFilter& ResourceFilter)
{
	/*if (!ResourceFilter.ResourceTypeFilter.Category.IsNone() && ResourceType.Category == ResourceFilter.ResourceTypeFilter.Category)
	{
		if (!ResourceFilter.ResourceTypeFilter.Type.IsNone() && ResourceType.Type == ResourceFilter.ResourceTypeFilter.Type)
		{
			if (!ResourceFilter.ResourceTypeFilter.SubType.IsNone() && ResourceType.SubType == ResourceFilter.ResourceTypeFilter.SubType)
			{
				return ETRResourceMatch::Exact;
			}
			else
			{
				return ETRResourceMatch::Type;
			}			
		}
		else
		{
			return ETRResourceMatch::Category;
		}		
	}
	*/
	if (!ResourceFilter.ResourceTypeFilter.Category.IsNone())
	{
		if (ResourceType.Category == ResourceFilter.ResourceTypeFilter.Category)
		{
			if (!ResourceFilter.ResourceTypeFilter.Type.IsNone())
			{
				if (ResourceType.Type == ResourceFilter.ResourceTypeFilter.Type)
				{
					if (!ResourceFilter.ResourceTypeFilter.SubType.IsNone())
					{
						if (ResourceType.SubType == ResourceFilter.ResourceTypeFilter.SubType)
						{
							return ETRResourceMatch::Exact;
						}
					}
					else
					{
						return ETRResourceMatch::Type;
					}
				}
			}
			else
			{
				return ETRResourceMatch::Category;
			}
		}
	}
	return ETRResourceMatch::None;
}


bool UResourceFunctionLibrary::FindResourceRateFilter(const TArray<FResourceRateFilter>& ResourceFilters, const FResourceType& TargetType, FResourceRateFilter& FoundRate, ETRResourceMatch& FoundMatchDegree, const ETRResourceMatch& MinimumMatchDegree)
{
	ETRResourceMatch TmpDegree = ETRResourceMatch::None;
	ETRResourceMatch BestMatchDegree = ETRResourceMatch::None;
	FResourceRateFilter BestMatch = FResourceRateFilter();
	uint8 minDegree = static_cast<uint8>(MinimumMatchDegree);
	uint8 bestDegree = 0;
	uint8 matchRank = 0;

	for (FResourceRateFilter RateFilter : ResourceFilters)
	{
		TmpDegree = ResourceFilterMatch(TargetType, RateFilter);
		if (TmpDegree == ETRResourceMatch::Exact)
		{
			//UE_LOG(LogTRGame, Log, TEXT("FindResourceRateFilter - Found exact match %s -- %s"), *TargetType.Code.ToString(), *RateFilter.ResourceTypeFilter.Code.ToString());
			// Found exact match, so stop looking.
			FoundRate = RateFilter;
			FoundMatchDegree = ETRResourceMatch::Exact;
			return true;
		}
		else
		{
			matchRank = static_cast<uint8>(TmpDegree);
			if (matchRank >= minDegree && matchRank > bestDegree)
			{
				BestMatch = RateFilter;
				BestMatchDegree = TmpDegree;
				bestDegree = matchRank;
			}
		}
	}
	if (BestMatchDegree != ETRResourceMatch::None && bestDegree >= minDegree)
	{
		//UE_LOG(LogTRGame, Log, TEXT("FindResourceRateFilter - Found match %s -- %s"), *TargetType.Code.ToString(), *BestMatch.ResourceTypeFilter.Code.ToString());
		FoundRate = BestMatch;
		FoundMatchDegree = BestMatchDegree;
		return true;
	}
	return false;
}


bool UResourceFunctionLibrary::FindBestResourceRateFilter(const TArray<FResourceRateFilter>& ResourceFilters, const FResourceType& TargetType, FResourceRateFilter& FoundRate, ETRResourceMatch& FoundMatchDegree, const ETRResourceMatch& MinimumMatchDegree)
{
	ETRResourceMatch BestMatchDegree = ETRResourceMatch::None;
	ETRResourceMatch TmpDegree = ETRResourceMatch::None;
	FResourceRateFilter BestRate = FResourceRateFilter();
	uint8 MinDegree = static_cast<uint8>(MinimumMatchDegree);
	
	for (FResourceRateFilter RateFilter : ResourceFilters)
	{
		TmpDegree = ResourceFilterMatch(TargetType, RateFilter);
		if (TmpDegree == ETRResourceMatch::Category && !RateFilter.ResourceTypeFilter.Type.IsNone()) { continue; }
		if (TmpDegree == ETRResourceMatch::Type && !RateFilter.ResourceTypeFilter.SubType.IsNone()) { continue; }
		if (static_cast<uint8>(TmpDegree) >= MinDegree && RateFilter.Rate > BestRate.Rate)
		{
			//UE_LOG(LogTRGame, Log, TEXT("FindBestResourceRateFilter - Found match %s -- %s"), *TargetType.Code.ToString(), *RateFilter.ResourceTypeFilter.Code.ToString());
			BestRate = RateFilter;
			BestMatchDegree = TmpDegree;
		}
	}
	if (BestMatchDegree != ETRResourceMatch::None)
	{
		//UE_LOG(LogTRGame, Log, TEXT("FindBestResourceRateFilter - best matching rate %s %.2f"), *BestRate.ResourceTypeFilter.Code.ToString(), BestRate.Rate);
		FoundRate = BestRate;
		FoundMatchDegree = BestMatchDegree;
		return true;
	}
	return false;
}


TArray<FResourceQuantity> UResourceFunctionLibrary::AddResourceQuantities(const TArray<FResourceQuantity>& QuantitiesOne, const TArray<FResourceQuantity>& QuantitiesTwo)
{
	TArray<FResourceQuantity> TotalResources;
	bool bFound = false;
	if (QuantitiesOne.Num() + QuantitiesTwo.Num() == 0) {
		return TotalResources;
	}
	TotalResources.Reserve(QuantitiesOne.Num() + QuantitiesTwo.Num());
	// Go through first array of quantities.  Do this only to ensure no dupe entries.
	for (FResourceQuantity TmpQuantity : QuantitiesOne)
	{
		bFound = false;
		for (int32 i = 0; !bFound && i < TotalResources.Num(); i++)
		{
			// If it exists in our totals, add to the existing quantity
			if (TmpQuantity.ResourceType == TotalResources[i].ResourceType)
			{
				TotalResources[i].Quantity = TotalResources[i].Quantity + TmpQuantity.Quantity;
				bFound = true;
			}
		}
		// Didn't find it in totals, then add an entry
		if (!bFound) {
			TotalResources.Add(TmpQuantity);
		}
	}
	// Go through second array of quantities
	for (FResourceQuantity TmpQuantity : QuantitiesTwo)
	{
		bFound = false;
		for (int32 i = 0; !bFound && i < TotalResources.Num(); i++)
		{
			// If it exists in our totals, add to the existing quantity
			if (TmpQuantity.ResourceType == TotalResources[i].ResourceType)
			{
				TotalResources[i].Quantity = TotalResources[i].Quantity + TmpQuantity.Quantity;
				bFound = true;
			}
		}
		// Didn't find it in totals, then add an entry
		if (!bFound) {
			TotalResources.Add(TmpQuantity);
		}
	}
	return TotalResources;
}


bool UResourceFunctionLibrary::SubtractResourceQuantities(const TArray<FResourceQuantity>& QuantitiesOne, const TArray<FResourceQuantity>& QuantitiesTwo, TArray<FResourceQuantity>& TotalResources, const bool bAllowUnderflow, const bool bIncludeZeroTotals)
{
	bool bFound = false;
	bool bHasNegative = false;
	float NewQuantity;
	TArray<FResourceQuantity> TmpTotalResources = QuantitiesOne;
	if (QuantitiesTwo.Num() == 0 || (bAllowUnderflow && QuantitiesOne.Num() == 0)) {
		return true;
	}
	// Subtract each entry in QuantitiesTwo 
	for (FResourceQuantity TmpQuantity : QuantitiesTwo)
	{
		bFound = false;
		for (int32 i = 0; !bFound && i < TmpTotalResources.Num(); i++)
		{
			// If it exists in our totals, do the subtraction
			if (TmpQuantity.ResourceType == TmpTotalResources[i].ResourceType)
			{
				NewQuantity = TmpTotalResources[i].Quantity - TmpQuantity.Quantity;
				if (NewQuantity < 0.0f)
				{
					TmpTotalResources[i].Quantity = 0.0f;
					bHasNegative = true;
				}
				else {
					TmpTotalResources[i].Quantity = NewQuantity;
				}
				bFound = true;
			}
		}
		if (!bFound)
		{
			bHasNegative = true;
			if (bIncludeZeroTotals)
			{
				// Add an entry with a 0 quantity
				TmpTotalResources.Add(FResourceQuantity(TmpQuantity.ResourceType, 0.0f));
			}
		}
	}
	TotalResources.Empty(TmpTotalResources.Num());
	if (bIncludeZeroTotals)
	{ 
		TotalResources.Append(TmpTotalResources);
	}
	else
	{
		// Only include entries for non-zero quantities.
		for (FResourceQuantity TotQuantity : TmpTotalResources)
		{
			if (TotQuantity.Quantity > 0.0f) {
				TotalResources.Add(TotQuantity);
			}
		}
	}

	return !bHasNegative || bAllowUnderflow;
}


void UResourceFunctionLibrary::MultiplyResourceQuantity(const TArray<FResourceQuantity>& Quantities, const float Multiplier, const bool bTruncate, TArray<FResourceQuantity>& MultipliedQuantities)
{
	float NewQuantity = 0;
	MultipliedQuantities.Empty(Quantities.Num());
	for (FResourceQuantity TmpQuantity : Quantities)
	{
		NewQuantity = TmpQuantity.Quantity * Multiplier;
		if (bTruncate) { NewQuantity = FMath::TruncToFloat(NewQuantity); }
		MultipliedQuantities.Add(FResourceQuantity(TmpQuantity.ResourceType, NewQuantity));
	}
}


void UResourceFunctionLibrary::StripZeroQuantities(TArray<FResourceQuantity>& Quantities)
{
	TArray<int32> StripIndexes;
	for (int32 i = 0; i < Quantities.Num(); i++)
	{
		if (Quantities[i].Quantity == 0.0f) {
			StripIndexes.Add(i);
		}
	}
	for (int32 i = StripIndexes.Num() - 1; i >= 0; i--)	{
		Quantities.RemoveAt(StripIndexes[i], 1, i == 0);
	}
}


FName UResourceFunctionLibrary::GoodsNameForResource(const FResourceType& ResourceType)
{
	return ResourceType.Code;
	//if (!ResourceType.SubType.IsNone()) { return ResourceType.SubType; }
	//return ResourceType.Type.IsNone() ? ResourceType.Category : ResourceType.Type;
}


void UResourceFunctionLibrary::ResourceTypeForCode(const FName& ResourceCode, FResourceType& ResourceType)
{
	if (!ResourceCode.IsNone())
	{
		TArray<FString> CodeStrings;
		ResourceCode.ToString().ParseIntoArray(CodeStrings, TEXT("."), true);
		ResourceType = FResourceType(
			ResourceCode,													/* Code */
			CodeStrings.IsValidIndex(0) ? FName(*CodeStrings[0]) : FName(), /* Category */
			CodeStrings.IsValidIndex(1) ? FName(*CodeStrings[1]) : FName(), /* Type */
			CodeStrings.IsValidIndex(2) ? FName(*CodeStrings[2]) : FName()  /* SubType */
		);
	}
	else {
		ResourceType = FResourceType();
	}
}


void UResourceFunctionLibrary::ResourceTypeForData(const FResourceTypeData& ResourceData, FResourceType& ResourceType)
{
	ResourceTypeForCode(ResourceData.Code, ResourceType);
}


int32 UResourceFunctionLibrary::ResourceDataInTier(const UDataTable* ResourceDataTable, const float MinTier, const float MaxTier, TArray<FResourceTypeData>& ResourceData)
{
	TArray<FName> RowNames = ResourceDataTable->GetRowNames();
	FResourceTypeData* PickedRow = nullptr;
	int32 FoundCnt = 0;
	for (FName RowName : RowNames)
	{
		PickedRow = ResourceDataTable->FindRow<FResourceTypeData>(RowName, "", false);
		if (PickedRow == nullptr) 
		{
			UE_LOG(LogTRGame, Error, TEXT("%s - ResourcesDataInTier - ResourceDataTable does not contain ResourceTypeData rows."));
			break;
		}
		// Exclude "category only" resources.
		if (PickedRow->Code.ToString().Contains(TEXT(".")) && MinTier <= PickedRow->Tier && PickedRow->Tier <= MaxTier)
		{
			ResourceData.Add(PickedRow);
			FoundCnt++;
		}
	}
	return FoundCnt;
}


int32 UResourceFunctionLibrary::ResourceDataByTier(const UDataTable* ResourceDataTable, const float MinTier, const float MaxTier, TMap<int32, FResourceTypeDataCollection>& ResourceDataByTier)
{
	TArray<FResourceTypeData> TierResources;
	int32 FoundCount = 0;
	if (ResourceDataInTier(ResourceDataTable, MinTier, MaxTier, TierResources) > 0)
	{
		for (FResourceTypeData Resource : TierResources)
		{
			if (!ResourceDataByTier.Contains((int32)Resource.Tier)) {
				ResourceDataByTier.Add((int32)Resource.Tier, FResourceTypeDataCollection());
			}
			ResourceDataByTier.Find((int32)Resource.Tier)->Data.Add(Resource);
			FoundCount++;
		}
	}
	return FoundCount;
}


bool UResourceFunctionLibrary::CalculateExtractedResources(const TArray<FResourceQuantity>& ExtractableResources, const float PercentOfTotal, const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities)
{
	ExtractedQuantities.Empty();
	// If we have no resources to extract just return
	if (ExtractableResources.Num() == 0) { return false; }
	// If 0% or extraction rates is empty just return
	if (PercentOfTotal <= 0 || ExtractionRates.Num() == 0) { return false; }

	float ExtractedQuantity;
	FResourceRateFilter FoundRate;
	ETRResourceMatch FoundSimilarity;

	ExtractedQuantities.Reserve(ExtractableResources.Num());
	for (FResourceQuantity CurResource : ExtractableResources)
	{
		if (FindBestResourceRateFilter(ExtractionRates, CurResource.ResourceType, FoundRate, FoundSimilarity))
		{
			//UE_LOG(LogTRGame, Log, TEXT("CalculateExtractedResources - target resource type: %s Found resource rate: %s  %.2f"), *CurResource.ResourceType.Code.ToString(), *FoundRate.ResourceTypeFilter.Code.ToString(), FoundRate.Rate)
			ExtractedQuantity = FMath::CeilToFloat((CurResource.Quantity * PercentOfTotal) * FoundRate.Rate);
			if (ExtractedQuantity <= 0.0f) { continue; }
			if (ExtractedQuantity > CurResource.Quantity) { ExtractedQuantity = CurResource.Quantity; }
			ExtractedQuantities.Add(FResourceQuantity(CurResource.ResourceType, ExtractedQuantity));
		}
	}
	return ExtractedQuantities.Num() > 0;
}


bool UResourceFunctionLibrary::SubtractExtractedResources(TArray<FResourceQuantity>& ExtractableResources, const float PercentOfTotal, const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities)
{
	ExtractedQuantities.Empty();
	// If we have no resources to extract just return
	if (ExtractableResources.Num() == 0) { return false; }
	// If 0% or extraction rates is empty just return
	if (PercentOfTotal <= 0 || ExtractionRates.Num() == 0) { return false; }

	float ExtractedQuantity;
	FResourceRateFilter FoundRate;
	ETRResourceMatch FoundSimilarity;
	FResourceQuantity* CurResource;

	ExtractedQuantities.Reserve(ExtractableResources.Num());
	for (int i = 0; i < ExtractableResources.Num(); i++)
	{
		CurResource = &ExtractableResources[i];
		if (FindBestResourceRateFilter(ExtractionRates, CurResource->ResourceType, FoundRate, FoundSimilarity))
		{
			//UE_LOG(LogTRGame, Log, TEXT("CalculateExtractedResources - target resource type: %s Found resource rate: %s  %.2f"), *CurResource.ResourceType.Code.ToString(), *FoundRate.ResourceTypeFilter.Code.ToString(), FoundRate.Rate)
			ExtractedQuantity = FMath::CeilToFloat((CurResource->Quantity * PercentOfTotal) * FoundRate.Rate);
			if (ExtractedQuantity <= 0.0f) { continue; }
			if (ExtractedQuantity > CurResource->Quantity) { ExtractedQuantity = CurResource->Quantity; }
			ExtractedQuantities.Add(FResourceQuantity(CurResource->ResourceType, ExtractedQuantity));
			CurResource->Quantity -= ExtractedQuantity;
		}
	}
	return ExtractedQuantities.Num() > 0;
}