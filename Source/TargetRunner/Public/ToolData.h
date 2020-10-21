#pragma once

//#include "ToolBase.h"
#include "AttributeDataSet.h"
#include "ModifiableAttributes.h"
#include "ToolData.generated.h"

USTRUCT(BlueprintType)
struct FToolData
{
	GENERATED_BODY()

public:

	// Tool class this is based on
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TSubclassOf<class UToolBase> ToolClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FAttributeDataSet AttributeData;

	// Modifiers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TMap<FName, FNamedModifierSet> Modifiers;

public:

	FToolData()
	{
		AttributeData.ResourceRateAttributes.Add(FName(TEXT("DamageRates")), FResourceRateFilterSet());
		AttributeData.ResourceRateAttributes.Add(FName(TEXT("ExtractionRates")), FResourceRateFilterSet());
		Modifiers.Add(FName(TEXT("EquipModifiers")), FNamedModifierSet(FName(TEXT("EquipModifiers"))));
		Modifiers.Add(FName(TEXT("ActivateModifiers")), FNamedModifierSet(FName(TEXT("ActivateModifiers"))));
	}
};