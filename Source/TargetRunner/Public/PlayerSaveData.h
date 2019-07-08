#pragma once

#include "Engine/DataTable.h"
#include "PlayerSaveData.generated.h"

USTRUCT(BlueprintType)
struct FPlayerSaveData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float TotalRunsPlayed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float TotalPlaytimeInRuns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float TargetsHit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float CurrentScore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float TotalTargetsHit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float HighScore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float MaxEnergy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float TargetsDestroyed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float TotalTargetsDestroyed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float EnergyRegenRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FName> ToolsUnlocked;

};