// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Delegates/Delegate.h"
#include "ActorAttributeComponent.h"
#include "GenericTeamAgentInterface.h"
#include "TargetRunner.h"
#include "RoomPlatformGridMgr.h"
#include "RoomGridTemplate.h"
#include "ToolActorBase.h"
#include "ToolWeaponBase.h"
#include "ToolEquipmentBase.h"
#include "TRPersistentDataComponent.h"
#include "TR_Character.h"
#include "TRPlayerControllerBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToolInventoryAdded, const FToolData&, ToolDataAdded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquippedToolsChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentToolChanged, const AToolActorBase*, CurrentTool);

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API ATRPlayerControllerBase : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:

	ATRPlayerControllerBase();

public:

	// This component handles save/load and replication of data to client.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		UTRPersistentDataComponent* PersistentDataComponent;

	// Base walk/run speed of player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		UActorAttributeComponent* RunSpeedAttribute;

	// Base jump force (height) of player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		UActorAttributeComponent* JumpForceAttribute;

	// Additional range boost for resource collection.
	// Added to collection range of collector being used. Default = 0.0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		UActorAttributeComponent* CollectionRangeAttribute;

	// All tools owned by player
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		TMap<FGuid, FToolData> ToolInventory;

	// Currently equipped tools (Weapons and Equipment)
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		TArray<UToolBase*> EquippedTools;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Replicated)
		int32 MaxEquippedWeapons;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Replicated)
		int32 MaxEquippedEquipment;

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

	// Our faction. Should be one of ETRFaction enum
	UPROPERTY(EditAnywhere)
	FGenericTeamId FactionId;

protected:

	// All tool types available for purchase in the market
	UPROPERTY(EditAnywhere)
		TArray<TSubclassOf<UToolBase>> MarketToolClasses;

protected:

	virtual void InitPlayerState() override;

	virtual void OnPossess(APawn* InPawn) override;

	// Implement GenericTeamAgentInterface
	FGenericTeamId GetGenericTeamId() const;
	
public:

	// [Server]
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void UpdateMovementFromAttributes();

	// [Client]
	// Replication notification
	UFUNCTION()
		void OnRep_CurrentTool();

	// [Both]
	// Binds to RunSpeedAttribute to notify on speed changes.
	UFUNCTION()
		void OnRunSpeedChanged(float NewSpeed);

	// [Both]
	// Binds to JumpForceAttribute to notify of attribute changes.
	UFUNCTION()
		void OnJumpForceChanged(float NewJumpForce);

	// [Both]
	// Binds to CollectionRangeAttribute to notify of attribute changes.
	UFUNCTION()
		void OnCollectionRangeChanged(float NewCollectionRange);

	// [Server]
	// Applies the given attribute modifiers to the controller and player state.
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ApplyAttributeModifiers(const TArray<FAttributeModifier>& NewModifiers);

	// [Server]
	// Removes the given attribute modifiers from the controller and player state.
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void RemoveAttributeModifiers(const TArray<FAttributeModifier>& ModifiersToRemove);

	// [Server]
	// Call this to add a tool to player's inventory. This handles rep. to client.
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerAddToolToInventory(UToolBase* Tool);

	// [Client]
	UFUNCTION(Client, Reliable, WithValidation)
		void ClientAddToolToInventory(const FToolData& ToolData, const FGuid AddedGuid);

	// [Server]
	// Call this to equip a tool from player's inventory. This handles rep. to client.
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerEquipTool(const FGuid ToolGuid);

	// [Client]
	UFUNCTION(Client, Reliable, WithValidation)
		void ClientEquipTool(const FGuid ToolGuid);

	// [Server]
	// Call this to unequip a tool from player. This handles rep. to client.
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerUnequipTool(const FGuid ToolGuid);

	// [Client]
	UFUNCTION(Client, Reliable, WithValidation)
		void ClientUnequipTool(const FGuid ToolGuid);

	// [Server]
	// Call this to unequip a tool from player. This handles rep. to client.
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerUnequipAllTools();

	// [Client]
	UFUNCTION(Client, Reliable, WithValidation)
		void ClientUnequipAllTools();

	// [Server]
	// Call this to apply an upgrade to a tool in player's tool inventory. This handles rep. to client.
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerUpgradeTool(const FGuid ToolGuid, const ETRToolUpgrade UpgradeType, const FResourceRateFilter RateDelta);

	// [Client]
	UFUNCTION(Client, Reliable, WithValidation)
		void ClientUpgradeTool(const FGuid ToolGuid, const ETRToolUpgrade UpgradeType, const FResourceRateFilter RateDelta);

	// [Server]
	// Call this to set the player's current active tool. This handles rep. to client.
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void ServerSetCurrentTool(const FGuid ToolGuid);

	// [Server]
	// Does the actual spawning of the current tool actor.
	// Override in BP.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void SpawnAsCurrentTool(UToolBase* NewCurrentTool);

	UFUNCTION(BlueprintCallable)
		void GetEquippedWeapons(TArray<UToolWeaponBase*>& EquippedWeapons);

	UFUNCTION(BlueprintCallable)
		void GetEquippedEquipment(TArray<UToolEquipmentBase*>& EquippedEquipment);

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
		bool GetPlayerSaveData(FPlayerSaveData& SaveData);

	// [Any]
	// Updates the controller and player state from serialized data.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		bool UpdateFromPlayerSaveData(const FPlayerSaveData& SaveData);

};
