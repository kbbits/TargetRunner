// Fill out your copyright notice in the Description page of Project Settings.


#include "TRGameModeLobby.h"
#include "TRPlayerState.h"
#include "PlayerLevelUpData.h"

ATRGameModeLobby::ATRGameModeLobby()
	: Super()
{
	
}

int32 ATRGameModeLobby::GetMarketDataForPlayer(const ATRPlayerControllerBase* MarketPlayerController, TArray<FGoodsPurchaseItem>& MarketGoods)
{
	if (!IsValid(GoodsMarketTable)) { 
		UE_LOG(LogTRGame, Warning, TEXT("TRGameModeLobby::GetMarketDataForPlayer - GoodsMarketTable is not valid"));
		return 0; 
	}
	ATRPlayerState* PlayerState = MarketPlayerController->GetPlayerState<ATRPlayerState>();
	const FGoodsPurchaseItem* PurchaseItem = nullptr;

	if (PlayerState == nullptr) { 
		UE_LOG(LogTRGame, Warning, TEXT("TRGameModeLobby::GetMarketDataForPlayer - PlayerState is not valid"));
		return 0; 
	}
	// Type-Safety
	check(GoodsMarketTable->GetRowStruct()->IsChildOf(FGoodsPurchaseItem::StaticStruct()));
	for (const TPair<FName, uint8*>& RowItr : GoodsMarketTable->GetRowMap())
	{
		PurchaseItem = reinterpret_cast<const FGoodsPurchaseItem*>(RowItr.Value);
		if (PurchaseItem->LevelAvailable <= PlayerState->ExperienceLevel)
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
		if (PurchaseItem->LevelAvailable <= PlayerState->ExperienceLevel)
		{
			ToolMarketGoods.Add(*PurchaseItem);
		}
	}
	return ToolMarketGoods.Num();
}


bool ATRGameModeLobby::GetLevelUpData(const int32 Level, FPlayerLevelUpData& LevelUpData)
{
	if (!IsValid(LevelUpTable)) { return false; }
	// Check data table row type
	check(LevelUpTable->GetRowStruct()->IsChildOf(FPlayerLevelUpData::StaticStruct()));

	FString NextLevel = FString::FromInt(Level);
	FPlayerLevelUpData* FoundLevelUpData = LevelUpTable->FindRow<FPlayerLevelUpData>(FName(NextLevel), "", false);
	if (FoundLevelUpData == nullptr) { return false; }
	LevelUpData = *FoundLevelUpData;
	return true;
}


bool ATRGameModeLobby::GetLevelUpDataForPlayer(const ATRPlayerControllerBase* PlayerController, FPlayerLevelUpData& LevelUpData)
{
	if (!IsValid(LevelUpTable)) { return false; }
	ATRPlayerState* PlayerState = PlayerController->GetPlayerState<ATRPlayerState>();
	if (PlayerState == nullptr) { return false; }
	// Check data table row type
	return GetLevelUpData(PlayerState->ExperienceLevel + 1, LevelUpData);
}




void ATRGameModeLobby::OnGameModeSeamlessTravelComplete_Implementation()
{

}


void ATRGameModeLobby::OnAllPlayersTravelComplete_Implementation()
{

}


void ATRGameModeLobby::PostSeamlessTravel()
{
	APlayerController* PlayerController;
	TotalPlayerControllersForTravel = 0;
	for (auto It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		PlayerController = Cast<APlayerController>(It->Get());
		if (IsValid(PlayerController))
		{
			TotalPlayerControllersForTravel++;
			//UE_LOG(LogTRGame, Log, TEXT("PostSeamlessTravel PlayerControllers: %s  %s"), *PlayerController->GetName(), *PlayerController->GetClass()->GetName());
		}
	}
	Super::PostSeamlessTravel();
	// Call our hook
	OnGameModeSeamlessTravelComplete();
}


void ATRGameModeLobby::GenericPlayerInitialization(AController* C)
{
	Super::GenericPlayerInitialization(C);
	ATRPlayerControllerBase* TRPlayerController = Cast<ATRPlayerControllerBase>(C);
	if (TRPlayerController)
	{
		ATRPlayerState* TRPlayerState = TRPlayerController->GetPlayerState<ATRPlayerState>();
		if (TRPlayerState)
		{
			UE_LOG(LogTRGame, Log, TEXT("GenericPlayerInit player id: %d"), TRPlayerState->GetPlayerId());
			if (TRPlayerState->PlayerGuid.IsValid())
			{
				//TRPlayerController->PersistentDataComponent->ServerLoadPlayerData();
			}
			else
			{
				UE_LOG(LogTRGame, Error, TEXT("TRGameModeLobby::GenericPlayerInitialization - PlayerState.PlayerGuid is invalid."));
			}
		}
	}

	// Call the BP hook.
	OnGenericPlayerInitialization(C);
}


//void ATRGameModeLobby::HandleLeavingMap()
//{
//
//}


void ATRGameModeLobby::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	//if (NewPlayer->PlayerState) {
	//	UE_LOG(LogTRGame, Log, TEXT("HandleStartingNewPlayer player id %d"), NewPlayer->PlayerState->GetPlayerId());
	//}
	//else {
	//	UE_LOG(LogTRGame, Log, TEXT("HandleStartingNewPlayer no player state"));
	//}
	APlayerController* PlayerController;
	int32 NumLoadedPlayers = 0;
	for (auto It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		PlayerController = Cast<APlayerController>(It->Get());
		// Starting players will all have the same controller class.
		if (IsValid(PlayerController) && PlayerController->GetClass() == NewPlayer->GetClass())
		{
			if (PlayerController->PlayerState && PlayerController->HasClientLoadedCurrentWorld())
			{
				NumLoadedPlayers++;
			}
			//UE_LOG(LogTRGame, Log, TEXT("HandleStartingNewPlayer PlayerControllers: %s  %s"), *PlayerController->GetName(), *PlayerController->GetClass()->GetName());
		}
	}
	UE_LOG(LogTRGame, Log, TEXT("TRGameModeLobby::HandleStartingNewPlayer - NumPlayers %d  NumTravellingPlayers %d  TotalPlayers %d  NumLoadedPlayers %d"), NumPlayers, NumTravellingPlayers, TotalPlayerControllersForTravel, NumLoadedPlayers);
	if (NumLoadedPlayers == TotalPlayerControllersForTravel)
	{
		OnAllPlayersTravelComplete();
	}
}


bool ATRGameModeLobby::ReadyToStartMatch_Implementation()
{
	return Super::ReadyToStartMatch_Implementation();
}