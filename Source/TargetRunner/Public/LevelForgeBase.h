#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "TargetRunner.h"
#include "ResourceQuantity.h"
#include "GoodsQuantityRange.h"
#include "LevelTemplate.h"
#include "LevelForgeBase.generated.h"

/**
 *
 */
UCLASS(BlueprintType, Blueprintable, Config=Game)
class TARGETRUNNER_API ULevelForgeBase : public UObject
{
    GENERATED_BODY()

public:
    // Constructor for defaults
    ULevelForgeBase(const FObjectInitializer& OI);
        
public:

    // The maximum difficulty tier
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config)
        int32 MAX_TIER;

    // Maximum absolute value of X grid extents both + and -
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config)
        int32 MAX_EXTENT_X;

    // Maximum absolute value of Y grid extents both + and -
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config)
        int32 MAX_EXTENT_Y;

    // The quantity of resources for a level. Will be scaled by resource tier relative to level tier. 
    // If resource tier == level tier, this will be the quantity range used.
    // Default is (5000.0f, 6000.0f)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config)
        FFloatRange BaseResourceQuantityRange;

    // Resources of one tier higher than current tier are also generated. When they are, the quantities are multiplied by this factor for these higher level resoruces.
    // Default = 0.2;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config)
        float HigherTierResourceQuantityMultiplier;

    // When resources quantities are scaled by relative tier.
    // Formula when resource tier != level tier: Quantity for resource tier X = Base quantity * ( 1 / ( (abs(Level Tier - X)+1) ^ ResourceQuantityTierScalingExp )
    // Default = 0.5 = inverse square falloff
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config)
        float ResourceQuantityTierScalingExp;

    // The cost to unlock levels - scaled by tier. See UnlockCostScalingExp
    // Default = 1000
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config)
        float BaseUnlockCost;

    // Level template unlock costs scale by tier.
    // Formula for unlock cost = ((level tier ^ UnlockCostScalingExp) * (BaseUnlockCost / 10)) * 10
    // Default = 1.75
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config)
        float UnlockCostScalingExp;

    // Base amount of time per level tier, in seconds.
    // Default = 300
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config, meta = (ClampMin="60.0", ClampMax="1800.0"))
        float BaseAvailableTime;

    // Scales the time available in level by tier.
    // Formula: (level tier ^ AvailableTimeScaleExp) * BaseAvailableTime. Final value is in increments of 10 and clamped to a range of BaseAvailableTime to 14400.
    // Default = 0.75;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config)
        float AvailableTimeScaleExp;

    // One row per tier. Each row (a FTieredGoodsQuantityRange) indicates a multiplier of goods quantities required for unlocking a level template of the given tier.
    // Must be set before LevelForge can be used;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UDataTable* UnlockGoodsCostTable;

    // A DataTable of FText rows
    // Must be set before LevelForge can be used;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UDataTable* ProperNamesTable;

    // A DataTable of FText rows.
    // Must be set before LevelForge can be used;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UDataTable* DescriptiveNamesTable;

    // A DataTable of FResourceTypeData rows
    // Must be set before LevelForge can be used;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UDataTable* ResourceDataTable;

    // A DataTable of FRoomSpecialActorsByTier rows. One row per tier.
    // Must be set before LevelForge can be used;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UDataTable* RoomSpecialActorsDataTable;

    // Until we get a table of thumbnails.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TAssetPtr<UTexture2D> DefaultThumbnail;

protected:

    // The current level template we are working on.
    //FLevelTemplate* LevelTemplate;

    // The random stream we are using to generate the LevelTemplate and content.
    FRandomStream LevelStream;

public:

    // Generate a new LevelTemplate with the given seed.
    // Subclasses override this to provide different implementations. (if ever needed)
    UFUNCTION(BlueprintCallable)
        virtual void GenerateNewLevelTemplate(const int32 NewSeed, const float DifficultyTier, FLevelTemplate& NewLevelTemplate, bool& Successful);

    UFUNCTION(BlueprintCallable)
        void UnlockGoodsCostFactorForTier(const int32 Tier, TArray<FGoodsQuantityRange>& CostFactors);
    
protected:

    // Generates a new DisplayName for a LevelTemplate.
    // Returns true if successful, false otherwise (ex: names tables are null or empty).
    bool GenerateDisplayName(FText& DisplayName);

    bool GenerateThumbnail(TAssetPtr<UTexture2D> Thumbnail);

    bool GenerateGridExtents(const float DifficultyTier, FLevelTemplate&);

    bool GenerateResourcesAvailable(const float DifficultyTier, TArray<FResourceQuantity>& ResourcesAvailable);

    bool GenerateSpecialsAvailable(const float DifficultyTier, TArray<TSubclassOf<AActor>>& SpecialsAvailable);
        
    // Debug
#if WITH_EDITORONLY_DATA
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
        bool bEnableClassDebugLog = true;
#endif
#if WITH_EDITOR
    FORCEINLINE void DebugLog(const FString& LogString) { if (bEnableClassDebugLog) { UE_LOG(LogTRGame, Log, TEXT("%s"), *LogString); } };
#else
    FORCEINLINE void DebugLog(const FString& LogString) { };
#endif

    FString LevelTemplateToString(const FLevelTemplate& LevelTemplate);

private:

    // Verifies that the ProperNamesTable and DescriptiveNamesTable are non-null and that they contain correct row types.
    bool VerifyNameTablesValid();
};