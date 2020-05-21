// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TR_GameMode.generated.h"

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API ATR_GameMode : public AGameMode
{
	GENERATED_BODY()

public:

	// Size of each grid cell in world units.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float GridCellSize;

		
};
