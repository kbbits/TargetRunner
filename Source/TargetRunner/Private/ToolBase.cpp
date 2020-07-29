// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolBase.h"
#include "..\Public\ToolBase.h"
#include "ResourceRateFilterSet.h"

const FName UToolBase::DAMAGE_RATES_NAME = FName(TEXT("DamageRates"));
const FName UToolBase::EXTRACTION_RATES_NAME = FName(TEXT("ExtractionRates"));
const FName UToolBase::BUY_VALUE_NAME = FName(TEXT("BuyValue"));

// Sets default values
UToolBase::UToolBase()
	: Super()
{
	EnergyPerShot.Name = FName(TEXT("EnergyPerShot"));
	BaseDamage.Name = FName(TEXT("BaseDamage"));
}


void UToolBase::ToToolData_Implementation(FToolData& ToolData)
{
	ToolData.ToolClass = this->GetClass();
	ToolData.AttributeData.ItemDisplayName = DisplayName;
	ToolData.AttributeData.ItemGuid = ItemGuid;
	ToolData.AttributeData.FloatAttributes.Add(BUY_VALUE_NAME, BuyValue);
	ToolData.AttributeData.Attributes.Add(EnergyPerShot.Name, EnergyPerShot);
	ToolData.AttributeData.Attributes.Add(BaseDamage.Name, BaseDamage);
	ToolData.AttributeData.ResourceRateAttributes.Add(DAMAGE_RATES_NAME, FResourceRateFilterSet(BaseDamageRates));
	ToolData.AttributeData.ResourceRateAttributes.Add(EXTRACTION_RATES_NAME, FResourceRateFilterSet(BaseResourceExtractionRates));
}


void UToolBase::UpdateFromToolData_Implementation(const FToolData& ToolData)
{
	if (ToolData.ToolClass.Get()->IsChildOf(GetClass()))
	{
		if (ToolData.AttributeData.ItemGuid.IsValid()) {
			if (ItemGuid.IsValid() && ItemGuid != ToolData.AttributeData.ItemGuid) {
				UE_LOG(LogTRGame, Warning, TEXT("UpdateFromTooldData - %s item guids do not match"), *GetNameSafe(this));
			}
			ItemGuid = ToolData.AttributeData.ItemGuid;
		}
		DisplayName = ToolData.AttributeData.ItemDisplayName;
		if (ToolData.AttributeData.FloatAttributes.Contains(BUY_VALUE_NAME)) {
			BuyValue = ToolData.AttributeData.FloatAttributes[BUY_VALUE_NAME];
		}
		if (ToolData.AttributeData.Attributes.Contains(EnergyPerShot.Name)) { 
			EnergyPerShot = ToolData.AttributeData.Attributes[EnergyPerShot.Name]; 
		}
		if (ToolData.AttributeData.Attributes.Contains(BaseDamage.Name)) { 
			BaseDamage = ToolData.AttributeData.Attributes[BaseDamage.Name]; 
		}
		if (ToolData.AttributeData.ResourceRateAttributes.Contains(DAMAGE_RATES_NAME)) { 
			BaseDamageRates = ToolData.AttributeData.ResourceRateAttributes[DAMAGE_RATES_NAME].Rates; 
		}
		if (ToolData.AttributeData.ResourceRateAttributes.Contains(EXTRACTION_RATES_NAME)) { 
			BaseResourceExtractionRates = ToolData.AttributeData.ResourceRateAttributes[EXTRACTION_RATES_NAME].Rates; 
		}
	}
}


UToolBase* UToolBase::CreateToolFromToolData(const FToolData& InToolData, UObject* Outer)
{
	UToolBase* NewTool = NewObject<UToolBase>(Outer, InToolData.ToolClass);
	NewTool->UpdateFromToolData(InToolData);
	return NewTool;
}
