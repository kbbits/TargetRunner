#pragma once

#include "Engine/DataTable.h"
#include "GoodsQuantity.h"
#include "LevelRunStats.generated.h"

USTRUCT(BlueprintType)
struct FLevelRunStats : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		bool ShownToPlayer;

	// Relates to difficulty
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float LevelNum;

	// Points earned during level run.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float Points;

	// Time played in the level run.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float LevelTime;

	// Seed used to generate the level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float LevelSeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FGoodsQuantity> GoodsEarned;
};