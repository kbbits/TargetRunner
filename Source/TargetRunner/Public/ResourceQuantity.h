// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TargetRunner.h"
#include "ResourceType.h"
#include "ResourceQuantity.generated.h"

USTRUCT(BlueprintType)
struct FResourceQuantity
{
	GENERATED_BODY()

public:

	// Required
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FResourceType ResourceType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float Quantity;

public:
	FResourceQuantity()
	{
		ResourceType = FResourceType();
		Quantity = 0.0f;
	}

	FResourceQuantity(const FName& NewResourceCategory, const FName& NewResourceType, const float NewQuantity)
	{
		ResourceType = FResourceType(NewResourceCategory, NewResourceType);
		Quantity = NewQuantity;
	}

	FResourceQuantity(const FName& NewResourceCategory, const FName& NewResourceType, const FName& NewResourceSubType, const float NewQuantity)
	{
		ResourceType = FResourceType(NewResourceCategory, NewResourceType, NewResourceSubType);
		Quantity = NewQuantity;
	}

	FResourceQuantity(const FResourceType& NewType, const float NewQuantity)
	{
		ResourceType = NewType;
		Quantity = NewQuantity;
	}

	bool operator==(const FResourceQuantity& Other) const
	{
		if (ResourceType != Other.ResourceType) return false;
		if (Quantity != Other.Quantity) return false;
		return true;
	}

	bool operator==(const FResourceQuantity& Other)
	{
		if (ResourceType != Other.ResourceType) return false;
		if (Quantity != Other.Quantity) return false;
		return true;
	}
	
	bool operator==(FResourceQuantity& Other)
	{
		if (ResourceType != Other.ResourceType) return false;
		if (Quantity != Other.Quantity) return false;
		return true;
	}
};
