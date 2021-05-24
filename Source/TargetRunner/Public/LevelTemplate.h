#pragma once

#include "Engine/DataTable.h"
#include "ResourceQuantity.h"
#include "GoodsQuantity.h"
#include "PlayerLevelRecord.h"
#include "LevelTemplate.generated.h"

// Describes a playable level. Instances of these will be generated.
// The instances will be made available to players. This data is then used to generate the playable level map.
USTRUCT(BlueprintType)
struct FLevelTemplate : public FTableRowBase
{
	GENERATED_BODY()

public:
	FLevelTemplate()
	{
		LevelSeed = 0;
		GridForgeType = FName(TEXT("Default"));
		Theme = FName(TEXT("Default"));
	}

	// Unique instance ID - a compound of tier:seed:gridforgetype:theme
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName LevelId;

	// Seed used to generate the level.
	// Level seed should be unique in each tier.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 LevelSeed;

	// Relates to difficulty
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float Tier;

	// The human-readable name for this level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FText DisplayName;

	// Thumbnail for GUI use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TAssetPtr<UTexture2D> Thumbnail;

	// Minimum X coordinate of grid extent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 MinX;
	// Minimum Y coordinate of grid extent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 MinY;
	// Maximum X coordinate of grid extent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 MaxX;
	// Maximum Y coordinate of grid extent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 MaxY;
		
	// The grid generation method to use.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName GridForgeType;

	// Required to unlock the level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float UnlockCost;

	// Required to unlock the level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, meta = (TitleProperty = "Name"))
		TArray<FGoodsQuantity> UnlockGoods;

	// Time available in the level run.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float AvailableTime;

	// The start time of day for the level (0-23, 0=midnight)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float StartHourOfDay;

	// The overall theme used for this level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Theme;

	// Various modifier tags relating to level generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FName> ThemeTags;

	// The total resources available on this level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, meta = (TitleProperty = "ResourceType"))
		TArray<FResourceQuantity> ResourcesAvailable;

	// The total quantities of other goods available on this level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, meta = (TitleProperty = "Name"))
		TArray<FGoodsQuantity> OtherGoodsAvailable;

	// Total list of special actors that will be distributed around the rooms.
	// One instance per class.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<TSubclassOf<AActor>> SpecialsAvailable;

public:
	FORCEINLINE bool IsValid() { return (!LevelId.IsNone() && LevelSeed != 0); }

	FORCEINLINE bool operator==(const FLevelTemplate& Other) const
	{
		if (LevelId != Other.LevelId) return false;
		return true;
	}

	FORCEINLINE bool operator==(const FLevelTemplate& Other)
	{
		if (LevelId != Other.LevelId) return false;
		return true;
	}

	FORCEINLINE bool operator==(FLevelTemplate& Other)
	{
		if (LevelId != Other.LevelId) return false;
		return true;
	}
};