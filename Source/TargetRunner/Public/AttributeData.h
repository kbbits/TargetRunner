#pragma once

#include "AttributeData.generated.h"

USTRUCT(BlueprintType)
struct FAttributeData 
{
	GENERATED_BODY()

public:

	// The name of the attribute. ex: Energy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		FName Name;

	// The minimum value allowed for this attribute
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		float MinValue = 0;

	// The maximum value allowed for this attribute
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		float MaxValue = 0;

	// The current value of this attribute
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemAttributes")
		float CurrentValue = 0;

};