// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LevelTemplate.h"
#include "LevelTemplatesSave.generated.h"

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API ULevelTemplatesSave : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FLevelTemplate> LevelTemplates;
	
};
