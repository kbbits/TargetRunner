#pragma once

#include "Engine/DataTable.h"
#include "TrEnums.h"
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
		float MaxLevelCompleted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float TotalRunsPlayed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float TotalPlaytimeInRuns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float ResourceOneTotal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float ResourceTwoTotal;

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
		float ResourceOneSpent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float ResourceTwoSpent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FName> ToolsUnlocked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TMap<FName, float> HitCounts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TMap<EAmmoType, float> AmmoInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TMap<FName, float> GoodsInventory;
};