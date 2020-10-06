// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolBase.h"
#include "..\Public\ToolBase.h"
#include "ResourceRateFilterSet.h"

const FName UToolBase::DAMAGE_RATES_NAME = FName(TEXT("DamageRates"));
const FName UToolBase::EXTRACTION_RATES_NAME = FName(TEXT("ExtractionRates"));

// Sets default values
UToolBase::UToolBase()
	: Super()
{
	BuyValue.Name = FName(TEXT("BuyValue"));
	UpgradeDamageCostMultiplier.Name = FName(TEXT("UpgradeDamageCostMultiplier"));
	UpgradeDamageCostMultiplier.Quantity = 1.0f;
	UpgradeExtractionCostMultiplier.Name = FName(TEXT("UpgradeExtractionCostMultiplier"));
	UpgradeExtractionCostMultiplier.Quantity = 1.0f;
	MaximumBounces.Name = FName(TEXT("MaximumBounces"));

	EnergyPerShot.Name = FName(TEXT("EnergyPerShot"));
	BaseDamage.Name = FName(TEXT("BaseDamage"));
	ActivationDelay.Name = FName(TEXT("ActivationDelay"));
	ProjectileSpeed.Name = FName(TEXT("ProjectileSpeed"));
	
}


void UToolBase::ToToolData_Implementation(FToolData& ToolData)
{
	ToolData.ToolClass = this->GetClass();
	ToolData.AttributeData.ItemDisplayName = DisplayName;
	ToolData.AttributeData.ItemGuid = ItemGuid;
	ToolData.AttributeData.FloatAttributes.Add(BuyValue.Name, BuyValue.Quantity);
	ToolData.AttributeData.FloatAttributes.Add(UpgradeDamageCostMultiplier.Name, UpgradeDamageCostMultiplier.Quantity);
	ToolData.AttributeData.FloatAttributes.Add(UpgradeExtractionCostMultiplier.Name, UpgradeExtractionCostMultiplier.Quantity);
	ToolData.AttributeData.FloatAttributes.Add(MaximumBounces.Name, static_cast<float>(MaximumBounces.Quantity));
	ToolData.AttributeData.Attributes.Add(EnergyPerShot.Name, EnergyPerShot);
	ToolData.AttributeData.Attributes.Add(BaseDamage.Name, BaseDamage);
	ToolData.AttributeData.Attributes.Add(ActivationDelay.Name, ActivationDelay);
	ToolData.AttributeData.Attributes.Add(ProjectileSpeed.Name, ProjectileSpeed);
	ToolData.AttributeData.ResourceRateAttributes.Add(DAMAGE_RATES_NAME, FResourceRateFilterSet(BaseDamageRates));
	ToolData.AttributeData.ResourceRateAttributes.Add(EXTRACTION_RATES_NAME, FResourceRateFilterSet(BaseResourceExtractionRates));
}


void UToolBase::UpdateFromToolData_Implementation(const FToolData& ToolData)
{
	// Some helper lambdas  //////////////////////////
	auto UpdateAttr = [ToolData](FAttributeData& AData)
	{
		if (ToolData.AttributeData.Attributes.Contains(AData.Name))
		{
			AData = ToolData.AttributeData.Attributes[AData.Name];
		}
	};

	auto UpdateFloatAttr = [ToolData](const FName& Name, float& FloatValue)
	{
		if (ToolData.AttributeData.FloatAttributes.Contains(Name))
		{
			FloatValue = ToolData.AttributeData.FloatAttributes[Name];
		}
	};

	auto UpdateNamedFloatAttr = [ToolData](FTRNamedFloat& NamedFloat)
	{
		if (ToolData.AttributeData.FloatAttributes.Contains(NamedFloat.Name))
		{
			NamedFloat.Quantity = ToolData.AttributeData.FloatAttributes[NamedFloat.Name];
		}
	};

	auto UpdateNamedIntAttr = [ToolData](FTRNamedInt& NamedInt)
	{
		if (ToolData.AttributeData.FloatAttributes.Contains(NamedInt.Name))
		{
			NamedInt.Quantity = static_cast<int32>(ToolData.AttributeData.FloatAttributes[NamedInt.Name]);
		}
	};
	/////////////////////////////////////////////////////////////

	if (ToolData.ToolClass.Get()->IsChildOf(GetClass()))
	{
		if (ToolData.AttributeData.ItemGuid.IsValid()) {
			if (ItemGuid.IsValid() && ItemGuid != ToolData.AttributeData.ItemGuid) {
				UE_LOG(LogTRGame, Warning, TEXT("UpdateFromTooldData - %s item guids do not match"), *GetNameSafe(this));
			}
			ItemGuid = ToolData.AttributeData.ItemGuid;
		}
		DisplayName = ToolData.AttributeData.ItemDisplayName;
		/*
		// Float Attributes
		if (ToolData.AttributeData.FloatAttributes.Contains(BUY_VALUE_NAME)) {
			BuyValue = ToolData.AttributeData.FloatAttributes[BUY_VALUE_NAME];
		}
		if (ToolData.AttributeData.FloatAttributes.Contains(UPGRADE_DAMAGE_MULTIPLIER_NAME)) {
			UpgradeDamageCostMultiplier = ToolData.AttributeData.FloatAttributes[UPGRADE_DAMAGE_MULTIPLIER_NAME];
		}
		if (ToolData.AttributeData.FloatAttributes.Contains(UPGRADE_EXTRACTION_MULTIPLIER_NAME)) {
			UpgradeExtractionCostMultiplier = ToolData.AttributeData.FloatAttributes[UPGRADE_EXTRACTION_MULTIPLIER_NAME];
		}
		if (ToolData.AttributeData.FloatAttributes.Contains(MAXIMUM_BOUNCES_NAME)) {
			MaximumBounces = static_cast<int32>(ToolData.AttributeData.FloatAttributes[MAXIMUM_BOUNCES_NAME]);
		}
		// Attributes
		if (ToolData.AttributeData.Attributes.Contains(EnergyPerShot.Name)) { 
			EnergyPerShot = ToolData.AttributeData.Attributes[EnergyPerShot.Name]; 
		}
		if (ToolData.AttributeData.Attributes.Contains(BaseDamage.Name)) { 
			BaseDamage = ToolData.AttributeData.Attributes[BaseDamage.Name]; 
		}
		if (ToolData.AttributeData.Attributes.Contains(ActivationDelay.Name)) {
			ActivationDelay = ToolData.AttributeData.Attributes[ActivationDelay.Name];
		}
		if (ToolData.AttributeData.Attributes.Contains(ProjectileSpeed.Name)) {
			ProjectileSpeed = ToolData.AttributeData.Attributes[ProjectileSpeed.Name];
		}
		*/
		UpdateNamedFloatAttr(BuyValue);
		UpdateNamedFloatAttr(UpgradeDamageCostMultiplier);
		UpdateNamedFloatAttr(UpgradeExtractionCostMultiplier);
		UpdateNamedIntAttr(MaximumBounces);
		UpdateAttr(EnergyPerShot);
		UpdateAttr(BaseDamage);
		UpdateAttr(ActivationDelay);
		UpdateAttr(ProjectileSpeed);
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
