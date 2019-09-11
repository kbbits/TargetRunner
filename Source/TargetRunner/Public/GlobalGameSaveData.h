#pragma once

#include "Engine/DataTable.h"
#include "GameFramework/GameMode.h"
#include "GameModeRecordData.h"
#include "GlobalGameSaveData.generated.h"

// This class holds info on the current state of the installed game.
// ex: last save slot selected, etc.
USTRUCT(BlueprintType)
struct FGlobalGameSaveData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float LastSelectedSaveSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TSubclassOf<AGameMode> LastGameMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float GameDifficulty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FGameModeRecordData> GameModeRecords;

};