// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ResourceTypeData.h"
#include "ResourceType.h"
#include "TR_GameState.generated.h"

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API ATR_GameState : public AGameState
{
	GENERATED_BODY()

	ATR_GameState();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float CurrentDifficulty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float CurrentLevelNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UDataTable* ResourceTypeDataTable;


public:

	// Helper to get the Goods.Name for the specified ResourceType
	UFUNCTION(BlueprintPure, Category = "Resource Functions")
		FName GetGoodsNameForResource(const FResourceType& ResourceType);

	// Gets the full ResourceTypeData for the given ResourceType.
	// Returns true if data was found, false otherwise.
	UFUNCTION(BlueprintCallable, Category = "Resource Functions")
		bool GetResourceTypeData(const FResourceType& ForResourceType, FResourceTypeData& ResourceData);
	
};
