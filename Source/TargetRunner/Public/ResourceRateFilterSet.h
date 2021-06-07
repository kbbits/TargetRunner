#pragma once

#include "ResourceRateFilter.h"
#include "ResourceRateFilterSet.generated.h"

USTRUCT(BlueprintType)
struct FResourceRateFilterSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, meta = (TitleProperty = "ResourceTypeFilter"))
		TArray<FResourceRateFilter> Rates;

public:
	FResourceRateFilterSet()
	{
	}

	FResourceRateFilterSet(const TArray<FResourceRateFilter>& RateFilters)
	{
		Rates.Append(RateFilters);
	}
};


USTRUCT(BlueprintType)
struct FNamedResourceRateFilterSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FResourceRateFilterSet RateFilterSet;

public:
	FNamedResourceRateFilterSet()
	{
	}

	FNamedResourceRateFilterSet(const FName NewName)
	{
		Name = NewName;
	}

	FNamedResourceRateFilterSet(const FName NewName, const TArray<FResourceRateFilter>& RateFilters)
	{
		Name = NewName;
		RateFilterSet.Rates.Append(RateFilters);
	}

};