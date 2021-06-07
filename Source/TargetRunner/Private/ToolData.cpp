

#include "ToolData.h"
#include "ToolBase.h"
#include "ResourceRateFilterSet.h"

FToolData::FToolData()
{
	ToolClass = UToolBase::StaticClass();
	AttributeData.ResourceRateAttributes.Add(FNamedResourceRateFilterSet(FName(TEXT("DamageRates"))));
	AttributeData.ResourceRateAttributes.Add(FNamedResourceRateFilterSet(FName(TEXT("ExtractionRates"))));
	Modifiers.Add(FNamedModifierSet(FName(TEXT("EquipModifiers"))));
	Modifiers.Add(FNamedModifierSet(FName(TEXT("ActivateModifiers"))));
}

bool FToolData::IsValid() const
{
	return (ToolClass->IsValidLowLevelFast() && AttributeData.ItemGuid.IsValid());
}