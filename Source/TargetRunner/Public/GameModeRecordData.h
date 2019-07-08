#pragma once

#include "Engine/DataTable.h"
#include "GameFramework/GameMode.h"
#include "GameModeRecordData.generated.h"

// This struct holds info on a set records (High scores, etc.) for a GameMode.
USTRUCT(BlueprintType)
struct FGameModeRecordData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TSubclassOf<AGameMode> GameModeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float GameDifficulty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FText PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float HighScore;
			
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float TargetsHit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float TargetsDestroyed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float PlayTime;

};