#pragma once

#include "Engine/DataTable.h"
#include "GoodsQuantity.h"
#include "PlayerLevelRecord.generated.h"

// Instances of these indicate a player's access and data related to the given level ID.
// These PlayerLevelRecords are related to LevelTemplates by the level ID.
USTRUCT(BlueprintType)
struct FPlayerLevelRecord : public FTableRowBase
{
	GENERATED_BODY()

public:
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 NumberOfCompletions;

	// Shortest time of completed level run.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FTimespan ShortestLevelTime;
	
	// Total goods player has earned on this level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FGoodsQuantity> TotalGoodsEarned;
};