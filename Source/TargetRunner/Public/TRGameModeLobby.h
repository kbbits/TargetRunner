// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TRPersistentDataComponent.h"
#include "TRPlayerControllerBase.h"
#include "GoodsPurchaseItem.h"
#include "ToolPurchaseItem.h"
#include "PlayerLevelUpData.h"
#include "TRGameModeLobby.generated.h"

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API ATRGameModeLobby : public AGameMode
{
	GENERATED_BODY()

public:
	ATRGameModeLobby();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UDataTable* GoodsMarketTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UDataTable* ToolsMarketTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UDataTable* LevelUpTable;

public:
	
	// [Any]
	// Gets the market data available to the provided player.
	// Return the number of goods market items found.
	UFUNCTION(BlueprintCallable)
		int32 GetMarketDataForPlayer(const ATRPlayerControllerBase* MarketPlayerController, TArray<FGoodsPurchaseItem>& MarketGoods);

	// [Any]
	// Gets the available tools in the tools market available to the provided player.
	// Return the number of goods market items found.
	UFUNCTION(BlueprintCallable)
		int32 GetToolMarketDataForPlayer(const ATRPlayerControllerBase* MarketPlayerController, TArray<FToolPurchaseItem>& ToolMarketGoods);

	// Gets the cost for a player to upgrade to the given experience level.
	// Returns false if there is no such experience level. Returns true and a populated GoodsRequired if there is matching data for the experience level.
	UFUNCTION(BlueprintCallable)
		bool GetLevelUpData(const int32 Level, FPlayerLevelUpData& LevelUpData);

	// Gets the cost for the given player to upgrade to the next experience level.
	// Returns false if there is no higher experience level for the player. Returns true and a populated GoodsRequired if there does exist a higher xp level for player.
	UFUNCTION(BlueprintCallable)
		bool GetLevelUpDataForPlayer(const ATRPlayerControllerBase* PlayerController, FPlayerLevelUpData& LevelUpData);
};
