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
	// Thumbnail for GUI use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TAssetPtr<UTexture2D> Thumbnail;

	// The actor class associated with this tool
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		TSubclassOf<class AToolActorBase> ToolActorClass;

	// Displayed to user
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
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

	const FName DAMAGE_RATES_NAME = FName(TEXT("DamageRates"));
	const FName EXTRACTION_RATES_NAME = FName(TEXT("ExtractionRates"));

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void ToToolData(FToolData& ToolData);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void UpdateFromToolData(const FToolData& ToolData);
};