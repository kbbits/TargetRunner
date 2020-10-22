// Fill out your copyright notice in the Description page of Project Settings.


#include "TRGameModeLobby.h"
#include "TRPlayerState.h"

ATRGameModeLobby::ATRGameModeLobby()
	: Super()
{
	
}

int32 ATRGameModeLobby::GetMarketDataForPlayer(const ATRPlayerControllerBase* MarketPlayerController, TArray<FGoodsPurchaseItem>& MarketGoods)
{
	if (!IsValid(GoodsMarketTable)) { return 0; }
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


int32 ATRGameModeLobby::GetToolMarketDataForPlayer(const ATRPlayerControllerBase* MarketPlayerController, TArray<FToolPurchaseItem>& ToolMarketGoods)
{
	if (!IsValid(ToolsMarketTable)) { return 0; }
	ATRPlayerState* PlayerState = MarketPlayerController->GetPlayerState<ATRPlayerState>();
	const FToolPurchaseItem* PurchaseItem = nullptr;

	if (PlayerState == nullptr) { return 0; }
	// Type-Safety
	check(ToolsMarketTable->GetRowStruct()->IsChildOf(FToolPurchaseItem::StaticStruct()));
	for (const TPair<FName, uint8*>& RowItr : ToolsMarketTable->GetRowMap())
	{
		PurchaseItem = reinterpret_cast<const FToolPurchaseItem*>(RowItr.Value);
		if (PurchaseItem->TierAvailable <= PlayerState->MaxTierCompleted)
		{
			ToolMarketGoods.Add(*PurchaseItem);
		}
	}
	return ToolMarketGoods.Num();
}