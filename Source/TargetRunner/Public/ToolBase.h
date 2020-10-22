#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ToolData.h"
#include "AttributeData.h"
#include "NamedPrimitiveTypes.h"
#include "ToolData.h"
#include "TRProjectileBase.h"
#include "GoodsQuantity.h"
#include "ResourceRateFilter.h"
#include "ToolBase.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TARGETRUNNER_API UToolBase : public UObject
{
	GENERATED_BODY()

public:

	UToolBase();

public:

	// Unique instance ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FGuid ItemGuid;

	// Thumbnail for GUI use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TAssetPtr<UTexture2D> Thumbnail;

	// The actor class associated with this tool
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<class AToolActorBase> ToolActorClass;

	// Displayed to user
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FText DisplayName;

	// Type of projectile this tool fires. Can be None.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		TSubclassOf<ATRProjectileBase> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		FAttributeData EnergyPerShot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		FGoodsQuantity AmmoPerShot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		FAttributeData BaseDamage;

	// If this is false the weapon/tool cannot be fired/used by the player.
	// Usually set to false for "passive only" tools that only have effects on equipping.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, SaveGame, Category = "Player Weapons")
		bool bAllowsActivation;

	// Minimum time in seconds between activations (i.e. between shots). Shots per second = (1 / FireDelay) + 1
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		FAttributeData ActivationDelay;

	// The speed of the projectile
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		FAttributeData ProjectileSpeed;

	// The effective damage rates of this tool against given resource types. Expressed as a percent of base damage (0.0 - 1.0).
	// Values > 1.0 are allowed. Calculated damage against a resource type not matching any filters will = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons", meta = (TitleProperty = "ResourceTypeFilter"))
		TArray<FResourceRateFilter> BaseDamageRates;

	// The effective extraction rates of this tool against given resource types. Expressed as a percent (0.0 - 1.0).
	// Values > 1.0 are allowed. Calculated extracted amounts against a resource type not matching any filters will = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons", meta = (TitleProperty = "ResourceTypeFilter"))
		TArray<FResourceRateFilter> BaseResourceExtractionRates;

protected:

	static const FName DAMAGE_RATES_NAME;				// = FName(TEXT("DamageRates"));
	static const FName EXTRACTION_RATES_NAME;			// = FName(TEXT("ExtractionRates"));
	static const FName EQUIP_MODS_NAME;					// = FName(TEXT("EquipModifiers"));
	static const FName ACTIVATE_MODS_NAME;				// = FName(TEXT("ActivateModifiers"));
	
	// Value of the item in the shop
	UPROPERTY(EditDefaultsOnly, Meta = (DeprecatedProperty, DeprecationMessage = "Based on data table of ToolPurchaseItems now."))
		FTRNamedFloat BuyValue;

	// Cost of the item in the market. The GoodsQuantitySet is loaded from tool market data and should be set after tool is constructed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FNamedGoodsQuantitySet CostValue;

	// Cost to upgrade a damage rate by 1 percent = (BuyValue / 100) * UpgradeDamageCostFactor
	UPROPERTY(EditDefaultsOnly)
		FTRNamedFloat UpgradeDamageCostMultiplier;

	// Cost to upgrade an extraction rate by 1 percent = (BuyValue / 100) * UpgradeExtractionCostFactor
	UPROPERTY(EditDefaultsOnly)
		FTRNamedFloat UpgradeExtractionCostMultiplier;

	// The number of times projectile will bounce when hitting world surfaces. Default = 0
	UPROPERTY(EditDefaultsOnly, Category = "Player Weapons")
		FTRNamedInt MaximumBounces;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Weapons")
		FNamedModifierSet EquipModifiers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Weapons")
		FNamedModifierSet ActivateModifiers;

public:

	// Value of the item in the market
	UFUNCTION(BlueprintPure)
		FORCEINLINE float GetBuyValue() { return BuyValue.Quantity; }

	// Cost of the tool in the tool market
	UFUNCTION(BlueprintPure)
		FORCEINLINE TArray<FGoodsQuantity> GetCostValue() { return CostValue.GoodsQuantitySet.Goods; }

	// Cost to upgrade a damage rate by 1 percent = (BuyValue / 100) * UpgradeDamageCostFactor
	UFUNCTION(BlueprintPure)
		FORCEINLINE float GetUpgradeDamageCostMultiplier() { return UpgradeDamageCostMultiplier.Quantity; }

	// Cost to upgrade an extraction rate by 1 percent = (BuyValue / 100) * UpgradeExtractionCostFactor
	UFUNCTION(BlueprintPure)
		FORCEINLINE float GetUpgradeExtractionCostMultiplier() { return UpgradeExtractionCostMultiplier.Quantity; }

	// The number of times projectile will bounce when hitting world surfaces. Default = 0
	UFUNCTION(BlueprintPure)
		FORCEINLINE int32 GetMaximumBounces() { return MaximumBounces.Quantity; }

	// The speed of the projectile
	UFUNCTION(BlueprintPure)
		FORCEINLINE float GetProjectileSpeed() { return ProjectileSpeed.CurrentValue; }

	UFUNCTION(BlueprintPure)
		FORCEINLINE TArray<FAttributeModifier> GetEquipModifiers() { return EquipModifiers.Modifiers; }

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