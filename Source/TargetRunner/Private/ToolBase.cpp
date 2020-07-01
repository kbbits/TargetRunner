// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolBase.h"
#include "..\Public\ToolBase.h"
#include "ResourceRateFilterSet.h"

// Sets default values
UToolBase::UToolBase()
	: Super()
{
	EnergyPerShot.Name = FName(TEXT("EnergyPerShot"));
	BaseDamage.Name = FName(TEXT("BaseDamage"));
}


void UToolBase::ToToolData_Implementation(FToolData& ToolData)
{
	ToolData.ToolClass = StaticClass();
	ToolData.AttributeData.Attributes.Add(EnergyPerShot.Name, EnergyPerShot);
	ToolData.AttributeData.Attributes.Add(BaseDamage.Name, BaseDamage);
	ToolData.AttributeData.ResourceRateAttributes.Add(DAMAGE_RATES_NAME, FResourceRateFilterSet(BaseDamageRates));
	ToolData.AttributeData.ResourceRateAttributes.Add(EXTRACTION_RATES_NAME, FResourceRateFilterSet(BaseResourceExtractionRates));
}

void UToolBase::UpdateFromToolData_Implementation(const FToolData& ToolData)
{
	if (GetClass()->IsChildOf(ToolData.ToolClass))
	{
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