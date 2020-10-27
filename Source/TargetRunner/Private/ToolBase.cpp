// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolBase.h"
#include "..\Public\ToolBase.h"
#include "ResourceRateFilterSet.h"

const FName UToolBase::EMIT_TYPE_NAME = FName(TEXT("EmitType"));
const FName UToolBase::ACTIVATION_TYPE_NAME = FName(TEXT("ActivationType"));
const FName UToolBase::DAMAGE_RATES_NAME = FName(TEXT("DamageRates"));
const FName UToolBase::EXTRACTION_RATES_NAME = FName(TEXT("ExtractionRates"));
const FName UToolBase::EQUIP_MODS_NAME = FName(TEXT("EquipModifiers"));
const FName UToolBase::ACTIVATE_MODS_NAME = FName(TEXT("ActivateModifiers"));

// Sets default values
UToolBase::UToolBase()
	: Super()
{
	AutoFireDelay.Name = FName(TEXT("AutoFireDelay"));
	AutoFireBurstLimit.Name = FName(TEXT("AutoFireBurstLimit"));
	BuyValue.Name = FName(TEXT("BuyValue"));
	CostValue.Name = FName(TEXT("CostValue"));
	UpgradeDamageCostMultiplier.Name = FName(TEXT("UpgradeDamageCostMultiplier"));
	UpgradeDamageCostMultiplier.Quantity = 1.0f;
	UpgradeExtractionCostMultiplier.Name = FName(TEXT("UpgradeExtractionCostMultiplier"));
	UpgradeExtractionCostMultiplier.Quantity = 1.0f;
	MaximumBounces.Name = FName(TEXT("MaximumBounces"));

	EnergyPerShot.Name = FName(TEXT("EnergyPerShot"));
	AmmoPerShot.Name = FName(TEXT("AmmoPerShot"));
	BaseDamage.Name = FName(TEXT("BaseDamage"));
	ActivationDelay.Name = FName(TEXT("ActivationDelay"));
	ProjectileSpeed.Name = FName(TEXT("ProjectileSpeed"));

	EquipModifiers.Name = EQUIP_MODS_NAME;
	ActivateModifiers.Name = ACTIVATE_MODS_NAME;
}


void UToolBase::ToToolData_Implementation(FToolData& ToolData)
{
	ToolData.ToolClass = this->GetClass();
	ToolData.AttributeData.ItemDisplayName = DisplayName;
	ToolData.AttributeData.ItemGuid = ItemGuid;
	ToolData.AttributeData.FloatAttributes.Add(EMIT_TYPE_NAME, static_cast<float>(static_cast<uint8>(EmitType)));
	ToolData.AttributeData.FloatAttributes.Add(ACTIVATION_TYPE_NAME, static_cast<float>(static_cast<uint8>(ActivationType)));
	ToolData.AttributeData.Attributes.Add(AutoFireDelay.Name, AutoFireDelay);
	ToolData.AttributeData.Attributes.Add(AutoFireBurstLimit.Name, AutoFireBurstLimit);
	ToolData.AttributeData.FloatAttributes.Add(BuyValue.Name, BuyValue.Quantity);
	ToolData.AttributeData.GoodsQuantitiesAttributes.Add(CostValue.Name, CostValue.GoodsQuantitySet);
	ToolData.AttributeData.FloatAttributes.Add(UpgradeDamageCostMultiplier.Name, UpgradeDamageCostMultiplier.Quantity);
	ToolData.AttributeData.FloatAttributes.Add(UpgradeExtractionCostMultiplier.Name, UpgradeExtractionCostMultiplier.Quantity);
	ToolData.AttributeData.FloatAttributes.Add(MaximumBounces.Name, static_cast<float>(MaximumBounces.Quantity));
	ToolData.AttributeData.Attributes.Add(EnergyPerShot.Name, EnergyPerShot);
	ToolData.AttributeData.GoodsQuantitiesAttributes.Add(AmmoPerShot.Name, AmmoPerShot.GoodsQuantitySet);
	ToolData.AttributeData.Attributes.Add(BaseDamage.Name, BaseDamage);
	ToolData.AttributeData.Attributes.Add(ActivationDelay.Name, ActivationDelay);
	ToolData.AttributeData.Attributes.Add(ProjectileSpeed.Name, ProjectileSpeed);
	ToolData.AttributeData.ResourceRateAttributes.Add(DAMAGE_RATES_NAME, FResourceRateFilterSet(BaseDamageRates));
	ToolData.AttributeData.ResourceRateAttributes.Add(EXTRACTION_RATES_NAME, FResourceRateFilterSet(BaseResourceExtractionRates));
	ToolData.AttributeData.BoolAttributes.Add(FName(TEXT("AllowActivate")), bAllowsActivation);
	ToolData.Modifiers.Add(EquipModifiers.Name, EquipModifiers);
	ToolData.Modifiers.Add(ActivateModifiers.Name, ActivateModifiers);
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

	auto UpdateNamedGoodsQuantitySetAttr = [ToolData](FNamedGoodsQuantitySet& NamedGoodsQuantitySet)
	{
		if (ToolData.AttributeData.GoodsQuantitiesAttributes.Contains(NamedGoodsQuantitySet.Name))
		{
			NamedGoodsQuantitySet.GoodsQuantitySet = ToolData.AttributeData.GoodsQuantitiesAttributes[NamedGoodsQuantitySet.Name];
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
		if (ToolData.AttributeData.FloatAttributes.Contains(EMIT_TYPE_NAME))
		{
			// Perhaps some excessive casting here. Just being explicit.
			EmitType = static_cast<EToolEmitType>(static_cast<uint8>(ToolData.AttributeData.FloatAttributes[EMIT_TYPE_NAME]));
		}
		if (ToolData.AttributeData.FloatAttributes.Contains(ACTIVATION_TYPE_NAME))
		{
			ActivationType = static_cast<EToolActivationType>(static_cast<uint8>(ToolData.AttributeData.FloatAttributes[ACTIVATION_TYPE_NAME]));
		}
		UpdateAttr(AutoFireDelay);
		UpdateAttr(AutoFireBurstLimit);
		UpdateNamedGoodsQuantitySetAttr(AmmoPerShot);
		UpdateNamedGoodsQuantitySetAttr(CostValue);
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
		if (ToolData.AttributeData.BoolAttributes.Contains(FName(TEXT("AllowActivate")))) {
			bAllowsActivation = ToolData.AttributeData.BoolAttributes[FName(TEXT("AllowActivate"))];
		}
		if (ToolData.Modifiers.Contains(EquipModifiers.Name)) {
			EquipModifiers = ToolData.Modifiers[EquipModifiers.Name];
		}
		if (ToolData.Modifiers.Contains(ActivateModifiers.Name)) {
			ActivateModifiers = ToolData.Modifiers[ActivateModifiers.Name];
		}
	}
}


UToolBase* UToolBase::CreateToolFromToolData(const FToolData& InToolData, UObject* Outer)
{
	UToolBase* NewTool = NewObject<UToolBase>(Outer, InToolData.ToolClass);
	NewTool->UpdateFromToolData(InToolData);
	return NewTool;
}
