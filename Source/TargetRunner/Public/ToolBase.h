#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ToolData.h"
#include "AttributeData.h"
//#include "ToolActorBase.h"
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

	// Value of the item in the shop
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float BuyValue;

	// Cost to upgrade a damage rate by 1 percent = (BuyValue / 100) * UpgradeDamageCostFactor
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float UpgradeDamageCostMultiplier;

	// Cost to upgrade an extraction rate by 1 percent = (BuyValue / 100) * UpgradeExtractionCostFactor
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float UpgradeExtractionCostMultiplier;

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

	// The effective damage rates of this tool against given resource types. Expressed as a percent of base damage (0.0 - 1.0).
	// Values > 1.0 are allowed. Calculated damage against a resource type not matching any filters will = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons")
		TArray<FResourceRateFilter> BaseDamageRates;

	// The effective extraction rates of this tool against given resource types. Expressed as a percent (0.0 - 1.0).
	// Values > 1.0 are allowed. Calculated extracted amounts against a resource type not matching any filters will = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons")
		TArray<FResourceRateFilter> BaseResourceExtractionRates;

protected:

	static const FName DAMAGE_RATES_NAME;				// = FName(TEXT("DamageRates"));
	static const FName EXTRACTION_RATES_NAME;			// = FName(TEXT("ExtractionRates"));
	static const FName BUY_VALUE_NAME;					// = FName(TEXT("BuyValue"));
	static const FName UPGRADE_DAMAGE_MULTIPLIER_NAME;		// = FName(TEXT("UpgradeDamageCostMultiplier"));
	static const FName UPGRADE_EXTRACTION_MULTIPLIER_NAME;	// = FName(TEXT("UpgradeExtractionCostMultiplier"));

public:

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