#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ToolData.h"
#include "AttributeData.h"
#include "NamedPrimitiveTypes.h"
#include "TRProjectileBase.h"
#include "GoodsQuantity.h"
#include "ResourceRateFilter.h"
#include "ToolBase.generated.h"


UENUM(BlueprintType)
enum class EToolEmitType : uint8
{
	// When activated this tool emits no projectile, beam, etc.
	None = 0	UMETA(DisplayName = "None"),
	// When activated this tool fires a projectile
	Projectile = 1	UMETA(DisplayName = "Projectile"),
	// When activated this tool emits a beam
	Beam = 2	UMETA(DisplayName = "Beam")

	// TODO: implement
	// When activated this tool emits a field of effect in an area
	//Area = 4	UMETA(DisplayName = "Area")
};

UENUM(BlueprintType)
enum class EToolActivationType : uint8
{
	// Weapon cannot be activated.
	None = 0	UMETA(DisplayName = "None"),
	// Single "botton press" fires a single activation. i.e. a single shot.
	Single = 1	UMETA(DisplayName = "Single"),
	// Tool repeatedly activates while "button press is held". i.e. fires while trigger is held.
	Auto = 2	UMETA(DisplayName = "Auto")
};

UCLASS(BlueprintType, Blueprintable)
class TARGETRUNNER_API UToolBase : public UObject
{
	GENERATED_BODY()

public:

	UToolBase();

	FORCEINLINE bool operator==(const FGuid& OtherItemGuid) const
	{
		return (ItemGuid == OtherItemGuid);
	}

	FORCEINLINE bool operator==(const FGuid& OtherItemGuid)
	{
		return (ItemGuid == OtherItemGuid);
	}

	FORCEINLINE bool operator==(FGuid& OtherItemGuid)
	{
		return (ItemGuid == OtherItemGuid);
	}
	
	// Does this tool have this ItemGuid?
	UFUNCTION(BlueprintPure, meta = (DisplayName = "== (guid)", CompactNodeTitle = "==", Keywords = "= equal"), Category = "Player Weapons|Guid")
	FORCEINLINE bool EqualsGuid(const FGuid& OtherGuid)
	{
		return ItemGuid == OtherGuid;
	}

public:

	// Unique instance ID of the tool
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FGuid ItemGuid;

	// Thumbnail for GUI use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TAssetPtr<UTexture2D> Thumbnail;

	// The actor class associated with this tool
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<class AToolActorBase> ToolActorClass;

	// The internal name (i.e. code) for this tool.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName Name;

	// Displayed to user. Could be edited by user as well.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FText DisplayName;

	// Displayed to user. Not editable.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FText Description;

	// If this is false the weapon/tool cannot be fired/used by the player.
	// Set to false for "passive only" tools that only have effects on equipping.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, SaveGame, Category = "Player Weapons")
		bool bAllowsActivation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		EToolEmitType EmitType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		EToolActivationType ActivationType;

	// Minimum time in seconds between when an activation ends and the next activations can begin. 
	// (i.e. between shots fired for single-fire tools and between bursts/beams for auto-fire tools). 
	// For single-fire tools, shots per second = (1 / FireDelay) + 1
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		FAttributeData ActivationDelay;

	// The delay between tool activations for a tool with ActivationType = EToolActivationType::Auto.
	// That is, the amount of time in seconds between each auto-fire activation.
	// Shots per second = (1 / AutoFireDelay) + 1
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FAttributeData AutoFireDelay;

	// Limit of activations in an auto-fire burst.
	// i.e. number of shots each time an auto-fire tool is activated.
	// Set to 0 (default) for no limit.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FAttributeData AutoFireBurstLimit;

	// The class of projectile this tool fires.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		TSubclassOf<ATRProjectileBase> ProjectileClass;

	// The speed of the projectile
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		FAttributeData ProjectileSpeed;

	// Energy used each time tool is activated
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		FAttributeData EnergyPerShot;

	// Ammo consumed for each tool activation. i.e. each time it fires
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		FNamedGoodsQuantitySet AmmoPerShot;

	// The base damage tool deals to target. Usually only relevant for Weapons.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		FAttributeData BaseDamage;
	
	// The effective damage rates of this tool against given resource types. Expressed as a percent of base damage (0.0 - 1.0).
	// Values > 1.0 are allowed. Calculated damage against a resource type not matching any filters will = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons", meta = (TitleProperty = "ResourceTypeFilter"))
		TArray<FResourceRateFilter> BaseDamageRates;

	// The effective extraction rates of this tool against given resource types. Expressed as a percent (0.0 - 1.0).
	// Values > 1.0 are allowed. Calculated extracted amounts against a resource type not matching any filters will = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons", meta = (TitleProperty = "ResourceTypeFilter"))
		TArray<FResourceRateFilter> BaseResourceExtractionRates;

protected:

	static const FName EMIT_TYPE_NAME;					// = FName(TEXT("EmitType"));
	static const FName ACTIVATION_TYPE_NAME;			// = FName(TEXT("ActivationType"));
	static const FName DAMAGE_RATES_NAME;				// = FName(TEXT("DamageRates"));
	static const FName EXTRACTION_RATES_NAME;			// = FName(TEXT("ExtractionRates"));
	static const FName EQUIP_MODS_NAME;					// = FName(TEXT("EquipModifiers"));
	static const FName ACTIVATE_MODS_NAME;				// = FName(TEXT("ActivateModifiers"));
	
	// Value of the item in the shop
	UPROPERTY(EditDefaultsOnly, Meta = (DeprecatedProperty, DeprecationMessage = "Based on data table of ToolPurchaseItems now."))
		FTRNamedFloat BuyValue;

	// Determines the cost of the item in the market (if available) as well as upgrade costs.
	// For items from the market, the CostValue is loaded from tool market data which overwrites the defaults.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FNamedGoodsQuantitySet CostValue;

	// Cost multiplier when upgrading a damage rate by 1 percent. Default is 1.0
	UPROPERTY(EditDefaultsOnly)
		FTRNamedFloat UpgradeDamageCostMultiplier;

	// Cost multiplier when upgrading an extraction rate by 1 percent. Default is 1.0
	UPROPERTY(EditDefaultsOnly)
		FTRNamedFloat UpgradeExtractionCostMultiplier;

	// The number of times projectile will bounce when hitting world surfaces. Default = 0
	UPROPERTY(EditDefaultsOnly, Category = "Player Weapons")
		FTRNamedInt MaximumBounces;

	// Modifiers applied by this tool when it is equipped.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Weapons")
		FNamedModifierSet EquipModifiers;

	// Modifiers applied by this tool when it is activated.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Weapons")
		FNamedModifierSet ActivateModifiers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Weapons")
		bool bEnableClassDebug = false;

public:

	// Minimum time in seconds between when an activation ends and the next activations can begin. 
	// (i.e. between shots fired for single-fire tools and between bursts/beams for auto-fire tools). 
	// For single-fire tools, shots per second = (1 / FireDelay) + 1
	UFUNCTION(BlueprintPure)
		FORCEINLINE float GetActivationDelay() { return ActivationDelay.CurrentValue; }

	// The delay between tool activations for a tool with ActivationType = EToolActivationType::Auto.
	// That is, the amount of time in seconds, between each auto-fire activation.
	// Shots per second = (1 / FireDelay) + 1
	UFUNCTION(BlueprintPure)
		FORCEINLINE float GetAutoFireDelay() { return AutoFireDelay.CurrentValue; }

	UFUNCTION(BlueprintPure)
		FORCEINLINE float GetAutoFireBurstLimit() { return AutoFireBurstLimit.CurrentValue; }

	// Value of the item in the market
	UFUNCTION(BlueprintPure)
		FORCEINLINE float GetBuyValue() { return BuyValue.Quantity; }

	// Cost of the tool in the tool market
	UFUNCTION(BlueprintPure)
		FORCEINLINE TArray<FGoodsQuantity> GetCostValue() { return CostValue.GoodsQuantitySet.Goods; }

	// Returns the calculated upgrade cost to upgrade this tool's given damage/extraction rate by the given percent
	UFUNCTION(BlueprintPure)
		FGoodsQuantitySet GetUpgradeCost(const float UpgradePercent, const ETRToolUpgrade UpgradeType, const FResourceType& ResourceRateType);

	// Cost to upgrade a damage rate by 1 percent = (BuyValue / 100) * UpgradeDamageCostMultplier
	UFUNCTION(BlueprintPure)
		FORCEINLINE float GetUpgradeDamageCostMultiplier() { return UpgradeDamageCostMultiplier.Quantity; }

	// Cost to upgrade an extraction rate by 1 percent = (BuyValue / 100) * UpgradeExtractionCostMultiplier
	UFUNCTION(BlueprintPure)
		FORCEINLINE float GetUpgradeExtractionCostMultiplier() { return UpgradeExtractionCostMultiplier.Quantity; }

	// The number of times projectile will bounce when hitting world surfaces. Default = 0
	UFUNCTION(BlueprintPure)
		FORCEINLINE int32 GetMaximumBounces() { return MaximumBounces.Quantity; }

	// The speed of the projectile
	UFUNCTION(BlueprintPure)
		FORCEINLINE float GetProjectileSpeed() { return ProjectileSpeed.CurrentValue; }

	UFUNCTION(BlueprintPure)
		FORCEINLINE float GetEnergyPerShot() { return EnergyPerShot.CurrentValue; }

	UFUNCTION(BlueprintPure)
		FORCEINLINE TArray<FGoodsQuantity> GetAmmoPerShot() { return AmmoPerShot.GoodsQuantitySet.Goods; }

	UFUNCTION(BlueprintPure)
		FORCEINLINE float GetBaseDamage() { return BaseDamage.CurrentValue; }

	// Modifiers applied by this tool when it is equipped.
	UFUNCTION(BlueprintPure)
		FORCEINLINE TArray<FAttributeModifier> GetEquipModifiers() { return EquipModifiers.Modifiers; }

	// Modifiers applied by this tool when it is activated.
	UFUNCTION(BlueprintPure)
		FORCEINLINE TArray<FAttributeModifier> GetActivateModifiers() { return ActivateModifiers.Modifiers; }

	// Transform this tool to a ToolData struct.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void ToToolData(FToolData& ToolData);

	// Update this tool with data from ToolData struct.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void UpdateFromToolData(const FToolData& ToolData);

	// Construct a new tool from a ToolData struct
	UFUNCTION(BlueprintCallable)
		static UToolBase* CreateToolFromToolData(const FToolData& InToolData, UObject* Outer);
};