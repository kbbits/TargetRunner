// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TRPersistentDataComponent.h"
#include "TRPlayerControllerBase.h"
#include "GoodsPurchaseItem.h"
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

public:
	// [Any]
	// Gets the market data available to the provided player.
	// Return the number of goods market items found.
	UFUNCTION(BlueprintCallable)
		int32 GetMarketDataForPlayer(const ATRPlayerControllerBase* MarketPlayerController, TArray<FGoodsPurchaseItem>& MarketGoods);
};
