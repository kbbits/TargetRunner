#pragma once

#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "RoomSpecialActorsByTier.generated.h"

USTRUCT(BlueprintType)
struct FSpecialActorChance
{
	GENERATED_BODY()
public:
	FSpecialActorChance()
	{
		Weight = 1.0f;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float Weight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<TSubclassOf<AActor>> SpecialActorClasses;
};

USTRUCT(BlueprintType)
struct FSpecialActorsChanceSet 
{
	GENERATED_BODY()

public:
	FSpecialActorsChanceSet()
	{
		ChanceToSpawn = 1.0f;
		NumPicks = 1;
	}

	// The percent chance that this set will spawn something.
	// 0.0 - 1.0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float ChanceToSpawn;

	// The number of items that will be selected from the weighted list of special actors.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 NumPicks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FSpecialActorChance> WeightedSpecialActorChances;
};

// Each of these records represents a single tier of potential special actors to add to the room grid.
USTRUCT(BlueprintType)
struct FRoomSpecialActorsByTier : public FTableRowBase
{
	GENERATED_BODY()

public:
	FRoomSpecialActorsByTier()
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 Tier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FSpecialActorsChanceSet> SpecialActorChances;
};