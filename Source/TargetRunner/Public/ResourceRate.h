// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TargetRunner.h"
#include "ResourceType.h"
#include "ResourceRate.generated.h"

USTRUCT(BlueprintType)
struct FResourceRate
{
	GENERATED_BODY()

public:

	// Required, but ResourceType can be partially filled. Category, Type or SubType = "None" are treated as wildcard filters.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FResourceType ResourceType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Category;

	// Required - By default this will match the GoodsType name related to this resource. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Type;

	// Optional
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName SubType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float Rate;

public:
	FResourceRate()
	{
		ResourceType = FResourceType();
		Rate = 0.0f;
	}

	FResourceRate(const FName& FilteredCategory, const FName& FilteredType, const float NewRate)
	{
		ResourceType = FResourceType(FilteredCategory, FilteredType);
		Rate = NewRate;
	}

	FResourceRate(const FName& FilteredCategory, const FName& FilteredType, const FName& FilteredSubType, const float NewRate)
	{
		ResourceType = FResourceType(FilteredCategory, FilteredType, FilteredSubType);
		Rate = NewRate;
	}

	FResourceRate(const FResourceType& FilteredType, const float NewRate)
	{
		ResourceType = FilteredType;
		Rate = NewRate;
	}

	bool operator==(const FResourceRate& Other) const
	{
		if (ResourceType != Other.ResourceType) return false;
		if (Rate != Other.Rate) return false;
		return true;
	}

	bool operator==(const FResourceRate& Other)
	{
		if (ResourceType != Other.ResourceType) return false;
		if (Rate != Other.Rate) return false;
		return true;
	}

	bool operator==(FResourceRate& Other)
	{
		if (ResourceType != Other.ResourceType) return false;
		if (Rate != Other.Rate) return false;
		return true;
	}
};
