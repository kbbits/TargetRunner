// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TargetRunner.h"
#include "ResourceType.h"
#include "ResourceRateFilter.generated.h"

USTRUCT(BlueprintType)
struct FResourceRateFilter
{
	GENERATED_BODY()

public:

	// Required, but ResourceType can be partially filled. Category, Type or SubType = "None" are treated as wildcard filters.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FResourceType ResourceTypeFilter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float Rate;

public:
	FResourceRateFilter()
	{
		ResourceTypeFilter = FResourceType();
		Rate = 0.0f;
	}

	FResourceRateFilter(const FName& FilteredCategory, const FName& FilteredType, const float NewRate)
	{
		ResourceTypeFilter = FResourceType(FilteredCategory, FilteredType);
		Rate = NewRate;
	}

	FResourceRateFilter(const FName& FilteredCategory, const FName& FilteredType, const FName& FilteredSubType, const float NewRate)
	{
		ResourceTypeFilter = FResourceType(FilteredCategory, FilteredType, FilteredSubType);
		Rate = NewRate;
	}

	FResourceRateFilter(const FResourceType& FilteredType, const float NewRate)
	{
		ResourceTypeFilter = FilteredType;
		Rate = NewRate;
	}

	bool operator==(const FResourceRateFilter& Other) const
	{
		if (ResourceTypeFilter != Other.ResourceTypeFilter) return false;
		if (Rate != Other.Rate) return false;
		return true;
	}

	bool operator==(const FResourceRateFilter& Other)
	{
		if (ResourceTypeFilter != Other.ResourceTypeFilter) return false;
		if (Rate != Other.Rate) return false;
		return true;
	}

	bool operator==(FResourceRateFilter& Other)
	{
		if (ResourceTypeFilter != Other.ResourceTypeFilter) return false;
		if (Rate != Other.Rate) return false;
		return true;
	}
};
