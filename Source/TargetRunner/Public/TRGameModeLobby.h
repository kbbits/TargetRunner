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

	/** GameMode's seamless travel has completed. All player controllers may not have completed travel yet. */
	UFUNCTION(BlueprintNativeEvent)
		void OnGameModeSeamlessTravelComplete();

	/** All players have completed travel. */
	UFUNCTION(BlueprintNativeEvent)
		void OnAllPlayersTravelComplete();


	/** Called when GameMode has completed seamless travel.
	*  We override to add a hook for GameMode level actions.
	*/
	virtual void PostSeamlessTravel() override;

	/**
	 * Handles all player initialization that is shared between the travel methods
	 * (i.e. called from both PostLogin() and HandleSeamlessTravelPlayer())
	 * This (does not loads player controller and state data from save) and then
	 * calls the BP version OnGenericPlayerInitialization.
	 */
	virtual void GenericPlayerInitialization(AController* C) override;

	/** Handle when player controllers change during seamless travel. */
	//virtual void SwapPlayerControllers(APlayerController* OldPC, APlayerController* NewPC) override;

protected:

	/** When each player is starting on the map level. */
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;


	/** Checks that there are no travelling players.  */
	virtual bool ReadyToStartMatch_Implementation() override;

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = Game, meta = (DisplayName = "OnGenericPlayerInitialization", ScriptName = "OnGenericPlayerInitialization"))
		void OnGenericPlayerInitialization(AController* C);
};
