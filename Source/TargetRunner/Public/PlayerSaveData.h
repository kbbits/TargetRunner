#pragma once

#include "Engine/DataTable.h"
#include "TrEnums.h"
#include "AttributeDataSet.h"
#include "ToolData.h"
#include "PlayerSaveData.generated.h"

USTRUCT(BlueprintType)
struct FPlayerSaveData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FGuid PlayerGuid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName ProfileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float MaxTierCompleted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float TotalRunsPlayed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float TotalPlaytimeInRuns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float CurrentScore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float HighScore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 ExperienceLevel;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	//	float MaxHealth;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	//	float MaxEnergy;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	//	float EnergyRegenRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FAttributeDataSet AttributeData;
		
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	//	TArray<FName> ToolsUnlocked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FToolData> ToolInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FGuid> LastEquippedItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TMap<FName, float> HitCounts;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	//	TMap<ETRAmmoType, float> AmmoInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TMap<FName, float> GoodsInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TMap<FName, float> SnapshotInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TMap<FName, float> TotalGoodsCollected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TMap<FName, float> TotalGoodsSpent;
	
};