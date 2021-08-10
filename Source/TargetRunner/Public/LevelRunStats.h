#pragma once

#include "Engine/DataTable.h"
#include "GoodsQuantity.h"
#include "LevelRunStats.generated.h"

/*
* Represents a snapshot of the results of a single run through a level.
*/
USTRUCT(BlueprintType)
struct FLevelRunStats : public FTableRowBase
{
	GENERATED_BODY()

public:

	// The player guid this data is associated with.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FGuid PlayerGuid;

	// The id of the level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName LevelId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		bool ShownToPlayer;

	// Relates to difficulty
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float Tier;

	// Points earned during level run. 
	// No longer used.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float Points;

	// Time played in the level run.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float LevelTime;
	
	// The goods collected by the player during the level run.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FGoodsQuantity> GoodsEarned;
};