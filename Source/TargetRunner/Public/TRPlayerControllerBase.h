// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Delegates/Delegate.h"
#include "RoomPlatformGridMgr.h"
#include "RoomGridTemplate.h"
#include "ToolActorBase.h"
#include "TRPersistentDataComponent.h"
#include "TRPlayerControllerBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToolInventoryAdded, const FToolData&, ToolDataAdded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquippedToolsChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentToolChanged, const AToolActorBase*, CurrentTool);

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API ATRPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:

	ATRPlayerControllerBase();

public:

	// This component handles save/load and replication of data to client.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		UTRPersistentDataComponent* PersistentDataComponent;

	// All tools owned by player
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		TMap<FGuid, FToolData> ToolInventory;

	// Currently equipped tools
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		TArray<UToolBase*> EquippedTools;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Replicated)
		int32 MaxEquippedWeapons;

	// Tool currently in use
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_CurrentTool, BlueprintReadWrite)
		AToolActorBase* CurrentTool;

	// Delegate event notification when Tool has been added to ToolInventory.
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnToolInventoryAdded OnToolInventoryAdded;

	// Delegate event notification when Tool has been equipped or unequipped.
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnEquippedToolsChanged OnEquippedToolsChanged;

	// Delegate event notification when player's current active Tool has changed.
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnCurrentToolChanged OnCurrentToolChanged;

protected:

	// All tool types available for purchase in the market
	UPROPERTY(EditAnywhere)
		TArray<TSubclassOf<UToolBase>> MarketToolClasses;
	
public:

	// [Client]
	// Replication notification
	UFUNCTION()
		void OnRep_CurrentTool();

	// [Any]
	// The Tools available for purchase in the market.
	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
		void GetMarketTools(TArray<TSubclassOf<UToolBase>>& AvailableMarketToolClasses);

	// [Any]
	// Set the Tools available for purchase in the market.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SetMarketTools(const TArray<TSubclassOf<UToolBase>>& AvailableMarketToolClasses);

	// [Server]
	// Call this to add a tool to player's inventory. This handles rep. to client.
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerAddToolToInventory(TSubclassOf<UToolBase> ToolClass);

	// [Client]
	UFUNCTION(Client, Reliable, WithValidation)
		void ClientAddToolToInventory(TSubclassOf<UToolBase> ToolClass, const FGuid AddedGuid);

	// [Server]
	// Call this to equip a tool from player's inventory. This handles rep. to client.
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerEquipTool(FGuid ToolGuid);

	// [Client]
	UFUNCTION(Client, Reliable, WithValidation)
		void ClientEquipTool(FGuid ToolGuid);

	// [Server]
	// Call this to unequip a tool from player. This handles rep. to client.
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerUnequipTool(FGuid ToolGuid);

	// [Client]
	UFUNCTION(Client, Reliable, WithValidation)
		void ClientUnequipTool(FGuid ToolGuid);

	// [Server]
	// Call this to set the player's current active tool. This handles rep. to client.
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerSetCurrentTool(FGuid ToolGuid);

	// [Server]
	// Does the actual spawning of the current tool actor.
	// Override in BP.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void SpawnAsCurrentTool(UToolBase* NewCurrentTool);

	// [Any]
	// Finds the current grid manager in the level.
	// Returns nullptr if no grid manager could be found.
	UFUNCTION(BlueprintCallable)
		ARoomPlatformGridMgr* FindGridManager();

	// Called from server to update the client's room grid template
	UFUNCTION(BlueprintCallable, Client, Reliable)
		void ClientUpdateRoomGridTemplate(const FRoomGridTemplate& UpdatedTemplate, const TArray<FVector2D>& RoomCoords, const TArray<FRoomTemplate>& RoomTemplates);

	// [Server]
	// Gets player save data from controller and player state.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void GetPlayerSaveData(FPlayerSaveData& SaveData);

	// [Any]
	// updates the controller and player state from serialized data.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void UpdateFromPlayerSaveData(const FPlayerSaveData& SaveData);
};
