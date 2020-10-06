// Fill out your copyright notice in the Description page of Project Settings.


#include "TRGameModeLobby.h"
#include "TRPlayerState.h"

ATRGameModeLobby::ATRGameModeLobby()
	: Super()
{
	
}

int32 ATRGameModeLobby::GetMarketDataForPlayer(const ATRPlayerControllerBase* MarketPlayerController, TArray<FGoodsPurchaseItem>& MarketGoods)
{
	if (!IsValid(GoodsMarketTable)) { return false; }
	ATRPlayerState* PlayerState = MarketPlayerController->GetPlayerState<ATRPlayerState>();
	const FGoodsPurchaseItem* PurchaseItem = nullptr;

	if (PlayerState == nullptr) { return 0; }
	// Type-Safety
	check(GoodsMarketTable->GetRowStruct()->IsChildOf(FGoodsPurchaseItem::StaticStruct()));
	for (const TPair<FName, uint8*>& RowItr : GoodsMarketTable->GetRowMap())
	{
		PurchaseItem = reinterpret_cast<const FGoodsPurchaseItem*>(RowItr.Value);
		if (PurchaseItem->TierAvailable <= PlayerState->MaxTierCompleted)
		{
			MarketGoods.Add(*PurchaseItem);
		}
	}
	return MarketGoods.Num();
}