#pragma once

#include "AttributeData.h"
#include "ResourceRateFilterSet.h"
#include "AttributeDataSet.generated.h"

USTRUCT(BlueprintType)
struct FAttributeDataSet 
{
	GENERATED_BODY()

public:

	// Unique instance ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		FGuid ItemGuid;

	// Attribute Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		TMap<FName, FAttributeData> Attributes;

	// Resource Rate Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		TMap<FName, FResourceRateFilterSet> ResourceRateAttributes;

	// Boolean Attributes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		TMap<FName, bool> BoolAttributes;

	// Float Attributes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		TMap<FName, float> FloatAttributes;

	// Vector Attributes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		TMap<FName, FVector> VectorAttributes;

};