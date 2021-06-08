#pragma once

#include "AttributeData.h"
#include "GoodsQuantity.h"
#include "ResourceRateFilterSet.h"
#include "NamedPrimitiveTypes.h"
#include "AttributeDataSet.generated.h"

USTRUCT(BlueprintType)
struct FAttributeDataSet 
{
	GENERATED_BODY()

public:

	// Unique instance ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		FGuid ItemGuid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		FText ItemDisplayName;

	// Attribute Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		TArray<FAttributeData> Attributes;
		//TMap<FName, FAttributeData> Attributes;

	// Resource Rate Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		TArray<FNamedResourceRateFilterSet> ResourceRateAttributes;
		//TMap<FName, FResourceRateFilterSet> ResourceRateAttributes;

	// Boolean Attributes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		TArray<FTRNamedBool> BoolAttributes;
		//TMap<FName, bool> BoolAttributes;

	// Float Attributes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		TArray<FTRNamedFloat> FloatAttributes;
		//TMap<FName, float> FloatAttributes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		TArray<FTRNamedInt> IntAttributes;

	// Vector Attributes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		TArray<FTRNamedVector> VectorAttributes;
		//TMap<FName, FVector> VectorAttributes;

	// GoodsQuantities
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		TArray<FNamedGoodsQuantitySet> GoodsQuantitiesAttributes;
		//TMap<FName, FGoodsQuantitySet> GoodsQuantitiesAttributes;

};