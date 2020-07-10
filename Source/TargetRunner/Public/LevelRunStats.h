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
		FGuid PlayerGuid;

	// The seed (ID) related to this level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName LevelId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		bool ShownToPlayer;

	// Relates to difficulty
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float Tier;

	// Points earned during level run.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float Points;

	// Time played in the level run.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float LevelTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FGoodsQuantity> GoodsEarned;
};