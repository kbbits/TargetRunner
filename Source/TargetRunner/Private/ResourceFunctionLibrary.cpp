// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceFunctionLibrary.h"
#include "..\Public\ResourceFunctionLibrary.h"

ETRResourceMatch UResourceFunctionLibrary::ResourceFilterMatch(const FResourceType& ResourceType, const FResourceRateFilter& ResourceFilter)
{
	if (!ResourceFilter.ResourceTypeFilter.Category.IsNone() && ResourceType.Category == ResourceFilter.ResourceTypeFilter.Category)
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
		TmpDegree = ResourceFilterMatch(RateFilter.ResourceTypeFilter, RateFilter);
		if (TmpDegree == ETRResourceMatch::Exact)
		{
			// Found exact match, so stop looking.
			FoundRate = RateFilter;
			FoundMatchDegree = ETRResourceMatch::Exact;
			return true;
		}
		else
		{
			matchRank = static_cast<uint8>(TmpDegree);
			if (matchRank > minDegree && matchRank > bestDegree)
			{
				BestMatch = RateFilter;
				BestMatchDegree = TmpDegree;
				bestDegree = matchRank;
			}
		}
	}
	if (BestMatchDegree != ETRResourceMatch::None && bestDegree >= minDegree)
	{
		FoundRate = BestMatch;
		FoundMatchDegree = BestMatchDegree;
		return true;
	}
	return false;
}

TArray<FResourceQuantity> UResourceFunctionLibrary::AddResourceQuantities(const TArray<FResourceQuantity>& QuantitiesOne, const TArray<FResourceQuantity>& QuantitiesTwo)
{
	TArray<FResourceQuantity> TotalResources;
	bool bFound = false;
	if (QuantitiesOne.Num() + QuantitiesTwo.Num() == 0)
	{
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
		if (!bFound)
		{
			TotalResources.Add(TmpQuantity);
		}
	}
	// Go through second array of quantities
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
		if (!bFound)
		{
			TotalResources.Add(TmpQuantity);
		}
	}

	return TotalResources;
}

FName UResourceFunctionLibrary::GoodsNameForResource(const FResourceType& ResourceType)
{
	return ResourceType.Type;
}

void UResourceFunctionLibrary::ResourceTypeForData(const FResourceTypeData& ResourceData, FResourceType& ResourceType)
{
	if (!ResourceData.Code.IsNone())
	{
		TArray<FString> CodeStrings;
		ResourceData.Code.ToString().ParseIntoArray(CodeStrings, TEXT("."), true);
		ResourceType = FResourceType(
			ResourceData.Code,												/* Code */
			CodeStrings.IsValidIndex(0) ? FName(*CodeStrings[0]) : FName(), /* Category */
			CodeStrings.IsValidIndex(1) ? FName(*CodeStrings[1]) : FName(), /* Type */
			CodeStrings.IsValidIndex(2) ? FName(*CodeStrings[2]) : FName()  /* SubType */
		);
	}
	else
	{
		ResourceType = FResourceType();
	}
}

int32 UResourceFunctionLibrary::ResourceDataInTier(const UDataTable* ResourceDataTable, const float MinTier, const float MaxTier, TArray<FResourceTypeData>& ResourceData)
{
	TArray<FName> RowNames = ResourceDataTable->GetRowNames();
	FResourceTypeData* PickedRow = nullptr;
	int32 FoundCnt = 0;
	for (FName RowName : RowNames)
	{
		PickedRow = ResourceDataTable->FindRow<FResourceTypeData>(RowName, "", false);
		if (PickedRow == nullptr) {
			UE_LOG(LogTRGame, Error, TEXT("%s - ResourcesDataInTier - ResourceDataTable does not contain ResourceTypeData rows."));
			break;
		}
		if (MinTier <= PickedRow->Tier && PickedRow->Tier <= MaxTier)
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
			if (!ResourceDataByTier.Contains((int32)Resource.Tier))
			{
				ResourceDataByTier.Add((int32)Resource.Tier, FResourceTypeDataCollection());
			}
			ResourceDataByTier.Find((int32)Resource.Tier)->Data.Add(Resource);
			FoundCount++;
		}
	}
	return FoundCount;
}
