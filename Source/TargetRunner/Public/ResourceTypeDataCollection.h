// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ResourceTypeData.h"
#include "ResourceTypeDataCollection.generated.h"

// An array of FResourceTypeData.  Mostly for use in TMaps.
USTRUCT(BlueprintType)
struct FResourceTypeDataCollection
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FResourceTypeData> Data;
};