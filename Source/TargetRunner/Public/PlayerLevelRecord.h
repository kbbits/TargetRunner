#pragma once

#include "Engine/DataTable.h"
#include "GoodsQuantity.h"
#include "PlayerLevelRecord.generated.h"

/*
* Instances of these indicate a player's access and data related to the given level ID.
* These PlayerLevelRecords are related to LevelTemplates by the level ID.
*/ 
USTRUCT(BlueprintType)
struct FPlayerLevelRecord : public FTableRowBase
{
	GENERATED_BODY()

public:
	// The player ID guid for this player level record.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FGuid PlayerGuid;
	
	// The ID related to this level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName LevelId;

	// Has the player unlocked access to this level?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		bool Unlocked;

	// The timestamp of when the player unlocked this level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FDateTime UnlockedAt;

	// The number of times the player has completed a level run of this level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 NumberOfCompletions;

	// Shortest time of completed level run.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FTimespan ShortestLevelTime;
	
	// Total goods player has earned on this level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FGoodsQuantity> TotalGoodsEarned;

public:

	FORCEINLINE bool operator==(const FPlayerLevelRecord& Other) const
	{
		if (PlayerGuid != Other.PlayerGuid) return false;
		if (LevelId != Other.LevelId) return false;
		return true;
	}

	FORCEINLINE bool operator==(const FPlayerLevelRecord& Other)
	{
		if (PlayerGuid != Other.PlayerGuid) return false;
		if (LevelId != Other.LevelId) return false;
		return true;
	}

	FORCEINLINE bool operator==(FPlayerLevelRecord& Other)
	{
		if (PlayerGuid != Other.PlayerGuid) return false;
		if (LevelId != Other.LevelId) return false;
		return true;
	}
};