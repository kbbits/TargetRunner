#pragma once

#include "ResourceRateFilter.h"
#include "ResourceRateFilterSet.generated.h"

USTRUCT(BlueprintType)
struct FResourceRateFilterSet
{
	GENERATED_BODY()

public:

	FResourceRateFilterSet() {
	}

	FResourceRateFilterSet(const TArray<FResourceRateFilter>& RateFilters)
	{
		Rates.Append(RateFilters);
	}

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FResourceRateFilter> Rates;


};