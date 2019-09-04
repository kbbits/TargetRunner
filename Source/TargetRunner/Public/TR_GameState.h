// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TR_GameState.generated.h"

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API ATR_GameState : public AGameState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float CurrentDifficulty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float CurrentLevelNumber;
	
};
