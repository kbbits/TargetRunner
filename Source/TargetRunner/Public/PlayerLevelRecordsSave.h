// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PlayerLevelRecord.h"
#include "PlayerLevelRecordsSave.generated.h"

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API UPlayerLevelRecordsSave : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FPlayerLevelRecord> PlayerRecords;
	
};
