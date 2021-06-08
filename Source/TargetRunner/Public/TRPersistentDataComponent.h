// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/Delegate.h"
#include "UnrealNetwork.h"
#include "LevelTemplate.h"
#include "LevelTemplateContextStruct.h"
#include "GoodsPurchaseItem.h"
#include "ToolPurchaseItem.h"
#include "PlayerSaveData.h"
#include "PlayerLevelUpData.h"
#include "TRPersistentDataComponent.generated.h"


// Event dispatcher for when CurrentValue changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewLevelTemplatesPage, const TArray<FLevelTemplateContextStruct>&, NewLevelTemplatesPage);
// Delegate for when player data has been loaded
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDataLoaded);
// Delegate when goods market data has been retrieved
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoodsMarketDataRetrieved, const TArray<FGoodsPurchaseItem>&, MarketGoods);
// Delegate when tools market data has been retrieved
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToolsMarketDataRetrieved, const TArray<FToolPurchaseItem>&, ToolsMarketItems);
// Delegate when player level up data has been retrieved
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNextPlayerLevelUpDataRetrieved, const bool, bNextLevelAvailable, const FPlayerLevelUpData&, LevelUpData);
// Tool Data dispatcher
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewPlayerToolData, const TArray<FToolData>&, NewPlayerToolData);


// This class manages the replication of data and RPC calls to/from the server for persistent data and save/load of player data.
// On server side, this calls GameInstance functions as well.
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TARGETRUNNER_API UTRPersistentDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTRPersistentDataComponent();

public:

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_LevelTemplatesPageLoaded)
		TArray<FLevelTemplateContextStruct> LevelTemplatesPage;

	UPROPERTY(ReplicatedUsing = OnRep_LevelTemplatesPageLoaded)
		int32 LevelTemplatesRepTrigger;
		
	// Delegate event when LevelTemplatesPage array has changed.
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnNewLevelTemplatesPage OnNewLevelTemplatesPage;

	// Delegate event when player data has been loaded.
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnPlayerDataLoaded OnPlayerDataLoaded;

	// Delegate when goods market data has been retreived.
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnGoodsMarketDataRetrieved OnGoodsMarketDataRetrieved;

	// Delegate when tools market data has been retreived.
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnToolsMarketDataRetrieved OnToolsMarketDataRetrieved;

	// Delegate when player level up data has been retreived.
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnNextPlayerLevelUpDataRetrieved OnNextPlayerLevelUpDataRetrieved;

	//UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_PlayerToolDataLoaded)
	//	TArray<FToolData> PlayerToolData;

	//UPROPERTY(ReplicatedUsing = OnRep_PlayerToolDataLoaded)
	//	int32 PlayerToolDataRepTrigger;

	//// Delegate event when PlayerToolData array has changed.
	//UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	//	FOnNewPlayerToolData OnNewPlayerToolData;

protected:

	static const FString PlayerFilenameSuffix;

// ##### Functions

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Level Template Save / Load / Paging

	// [Client]
	// Replication notification
	UFUNCTION()
		void OnRep_LevelTemplatesPageLoaded();
			
	// [Server]
	// Server will generate a new level template. Results will show up in the replicated LevelTemplatesPage property which
	// will fire the OnNewLevelTemplatesPage delegate event.
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerGenerateNewLevelTemplate(const float Tier);

	// [Server]
	// Just calls GameInstance to save data
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerSaveLevelTemplatesData();
	
	// [Server]
	// Loads level template data
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerLoadLevelTemplatesData();

	// [Server]
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerUnlockLevelTemplateForPlayer(const FName LevelId);
	
	// [Server]
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerSetLevelTemplateForPlay(const FLevelTemplate& LevelTemplate);

	// Player Save Data

	FString GetPlayerSaveFilename(); 

	// returns a list of all save game filenames in /Saved/SaveGames folder, including the .sav extension.
	UFUNCTION(BlueprintPure)
		static TArray<FString> GetAllSaveProfileFilenames();

	// returns all save games in /Saved/SaveGames folder as FPlayerSaveData structs
	UFUNCTION(BlueprintCallable)
		static TArray<FPlayerSaveData> GetAllSaveProfileData();

	// [Server]
	// Save the player's data
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerSavePlayerData();

	// [Server]
	// Load the player's data
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerLoadPlayerData(const FGuid PlayerGuid);

	// [Client]
	// Echo loaded player data back to client
	UFUNCTION(Client, Reliable, BlueprintCallable, WithValidation)
		void ClientEchoLoadPlayerData(const FPlayerSaveData PlayerSaveData);

	// Market Data

	// [Server]
	// Retrieve the goods market data for the given player.  Results in call to client's ClientGoodsMarketDataRetrieved.
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerRetrieveGoodsMarketData();

	// [Client]
	// Server calls this on client to pass the goods market data for the player.
	UFUNCTION(Client, Reliable, BlueprintCallable, WithValidation)
		void ClientEchoGoodsMarketData(const TArray<FGoodsPurchaseItem>& GoodsMarketData);

	// [Server]
	// Retrieve the tools market data for the given player.  Results in call to client's ClientToolsMarketDataRetrieved.
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerRetrieveToolsMarketData();

	// [Client]
	// Server calls this on client to pass the goods market data for the player.
	UFUNCTION(Client, Reliable, BlueprintCallable, WithValidation)
		void ClientEchoToolsMarketData(const TArray<FToolPurchaseItem>& ToolsMarketData);

	// [Server]
	// Get the level up data for next player level for TRPlayerControllerBase that this component is attached to.
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerRetrieveNextPlayerLevelUpData();

	// [Client]
	// Server calls this to pass the player's next level up data to the client.
	// This calls the notifiation delegate OnNextPlayerLevelUpDataRetrieved
	UFUNCTION(Client, Reliable, WithValidation)
		void ClientEchoNextPlayerLevelUpData(const bool bHasNextLevel, const FPlayerLevelUpData& LevelUpData);
};
