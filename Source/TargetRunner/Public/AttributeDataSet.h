#pragma once

#include "AttributeData.h"
#include "AttributeDataSet.generated.h"

USTRUCT(BlueprintType)
struct FAttributeDataSet 
{
	GENERATED_BODY()

public:

	// Attribute Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ActorAttributes")
		TMap<FName, FAttributeData> Attributes;

	// Boolean Attributes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ActorAttributes")
		TMap<FName, bool> BoolAttributes;

	// Float Attributes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ActorAttributes")
		TMap<FName, float> FloatAttributes;

	// Vector Attributes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ActorAttributes")
		TMap<FName, FVector> VectorAttributes;

};