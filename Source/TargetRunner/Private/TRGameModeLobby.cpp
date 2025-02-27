// Fill out your copyright notice in the Description page of Project Settings.


#include "TRGameModeLobby.h"
#include "TRGameInstance.h"
#include "TRPlayerControllerBase.h"
#include "TRPlayerState.h"
#include "PlayerLevelUpData.h"

ATRGameModeLobby::ATRGameModeLobby()
	: Super()
{
	TotalPlayerControllersForTravel = 0;
}


void ATRGameModeLobby::SetSelectedLevelTemplate(const FName LevelId)
{
	UTRGameInstance* GameInst = GetWorld()->GetGameInstance<UTRGameInstance>();
	if (GameInst) {
		if (!GameInst->LevelTemplatesMap.Contains(LevelId)) {
			UE_LOG(LogTRGame, Error, TEXT("GameModeLobby::SetSelectedLevelTemplate - Level template with id %s does not exist in templates map"), *LevelId.ToString());
			return;
		}
	}
	else {
		UE_LOG(LogTRGame, Error, TEXT("GameModeLobby::SetSelectedLevelTemplate - Cannot get game instance."));
		return;
	}
	FLevelTemplate& NewTemplate = GameInst->LevelTemplatesMap.FindRef(LevelId)->LevelTemplate;
	if (NewTemplate.LevelSeed == 0) { UE_LOG(LogTRGame, Warning, TEXT("GameModeLobby - SetSelectedLevelTemplate - Template has 0 seed.")); }
	// Set it on server
	GameInst->SetSelectedLevelTemplate(NewTemplate);
	ATRPlayerControllerBase* PController = nullptr;
	// Relay the selection to all clients (non-local controllers)
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		PController = Cast<ATRPlayerControllerBase>(*Iterator);
		if (IsValid(PController) && !PController->IsLocalController()) {
			PController->PersistentDataComponent->ClientSetSelectedLevelTemplate(NewTemplate);
		}
	}
}


void ATRGameModeLobby::SaveAllPlayerData()
{
	ATRPlayerControllerBase* PController = nullptr;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		PController = Cast<ATRPlayerControllerBase>(*Iterator);
		if (IsValid(PController)) {
			PController->PersistentDataComponent->ServerSavePlayerData();
		}
	}
}


void ATRGameModeLobby::ReloadAllPlayerData()
{
	ATRPlayerControllerBase* PController = nullptr;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		PController = Cast<ATRPlayerControllerBase>(*Iterator);
		if (IsValid(PController)) {
			PController->PersistentDataComponent->ServerLoadPlayerData();
		}
	}
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
		if (PurchaseItem->LevelAvailable <= PlayerState->ExperienceLevel) {
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
		if (PurchaseItem->LevelAvailable <= PlayerState->ExperienceLevel) {
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
	return GetLevelUpData(PlayerState->ExperienceLevel + 1, LevelUpData);
}




void ATRGameModeLobby::OnGameModeSeamlessTravelComplete_Implementation()
{

}


void ATRGameModeLobby::OnAllPlayersTravelComplete_Implementation()
{

}


void ATRGameModeLobby::LevelTemplatesChanged_Implementation()
{
	ATRPlayerControllerBase* PlayerController = nullptr;
	for (auto It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		PlayerController = Cast<ATRPlayerControllerBase>(It->Get());
		if (IsValid(PlayerController)) {
			PlayerController->ClientLevelTemplatesChanged();
		}
	}
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
			UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("PostSeamlessTravel PlayerControllers: %s  %s"), *PlayerController->GetName(), *PlayerController->GetClass()->GetName());
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
			UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("GenericPlayerInit player id: %d"), TRPlayerState->GetPlayerId());
			if (TRPlayerState->PlayerGuid.IsValid())
			{
				UE_LOG(LogTRGame, Log, TEXT("GenericPlayerInit player guid: %sd"), *TRPlayerState->PlayerGuid.ToString(EGuidFormats::Digits));
				//TRPlayerController->PersistentDataComponent->ServerLoadPlayerData();
			}
			else {
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

	if (bEnableClassDebug)
	{
		if (NewPlayer->PlayerState) 
		{
			ATRPlayerState* TRPlayerState = Cast<ATRPlayerState>(NewPlayer->PlayerState);
			if (TRPlayerState) {
				UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("HandleStartingNewPlayer player id %d guid %s"), NewPlayer->PlayerState->GetPlayerId(), *TRPlayerState->PlayerGuid.ToString(EGuidFormats::Digits));
			}
			else {
				UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("HandleStartingNewPlayer player id %d. Not a TRPlayerState."), NewPlayer->PlayerState->GetPlayerId());
			}
		}
		else {
			UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("HandleStartingNewPlayer no player state"));
		}
	}
	APlayerController* PlayerController;
	int32 NumLoadedPlayers = 0;
	for (auto It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		PlayerController = Cast<APlayerController>(It->Get());
		// Starting players will all have the same controller class.
		if (IsValid(PlayerController) && PlayerController->GetClass() == NewPlayer->GetClass())
		{
			if (PlayerController->PlayerState && PlayerController->HasClientLoadedCurrentWorld()) {
				NumLoadedPlayers++;
			}
			UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("HandleStartingNewPlayer PlayerControllers: %s  %s"), *PlayerController->GetName(), *PlayerController->GetClass()->GetName());
		}
	}
	UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("TRGameModeLobby::HandleStartingNewPlayer - NumPlayers %d  NumTravellingPlayers %d  TotalPlayers %d  NumLoadedPlayers %d"), NumPlayers, NumTravellingPlayers, TotalPlayerControllersForTravel, NumLoadedPlayers);
	if (NumLoadedPlayers == TotalPlayerControllersForTravel)
	{
		OnAllPlayersTravelComplete();
	}
}


bool ATRGameModeLobby::ReadyToStartMatch_Implementation()
{
	//return Super::ReadyToStartMatch_Implementation();
	if (GetMatchState() == MatchState::WaitingToStart)
	{
		if (bAllClientsReady && (TotalPlayerControllersForTravel == 0 || NumPlayers == TotalPlayerControllersForTravel)) {
			return true;
		}
	}
	return false;
}