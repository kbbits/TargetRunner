// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TR_GameMode.h"
#include "GameModeRooms.generated.h"

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API AGameModeRooms : public ATR_GameMode
{
	GENERATED_BODY()

public:

	AGameModeRooms();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
