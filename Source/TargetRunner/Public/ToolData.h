#pragma once

//#include "ToolBase.h"
#include "AttributeDataSet.h"
#include "ModifiableAttributes.h"
#include "ToolData.generated.h"

class UToolBase;

USTRUCT(BlueprintType)
struct FToolData
{
	GENERATED_BODY()

public:

	// Tool class this is based on
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TSubclassOf<UToolBase> ToolClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FAttributeDataSet AttributeData;

	// Modifiers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FNamedModifierSet> Modifiers;
		//TMap<FName, FNamedModifierSet> Modifiers;

public:

	FToolData();

	bool IsValid() const;
};