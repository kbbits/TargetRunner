#pragma once

#include "Engine/DataTable.h"
#include "GoodsQuantity.h"
#include "ToolBase.h"
#include "PlayerLevelUpData.generated.h"

// This struct represents one row of data describing requirements for a player level up.
USTRUCT(BlueprintType)
struct FPlayerLevelUpData : public FTableRowBase
{
	GENERATED_BODY()

public:

	// The level represented by this record. This is the player level that is being leveled to, not the current player level.
	// ex: if Level = 2, then this describes the requirements to reach level 2 and the associated player upgrades (max health, etc.)
	// NOTE: When building DataTables, the row name must be set to the value of this field because level records are looked up by row name.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 Level;

	// Goods required to reach this level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FGoodsQuantitySet GoodsRequired;

	// Equipped weapons slots to add
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 AddEquippedWeapons;

	// Equipped equipment slots to add
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 AddEquippedEquipment;

	// Amount to increase max health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float AddMaxHealth;

	// Amount to increase max animus
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float AddMaxAnimus;

	// Amount to increase max energy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float AddMaxEnergy;

	// Goods to award player when they reach this level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FGoodsQuantitySet GoodsAwarded;

	// Tools to award player when they reach this level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<TSubclassOf<UToolBase>> ToolsAwarded;
	
};