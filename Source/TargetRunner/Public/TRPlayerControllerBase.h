// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Delegates/Delegate.h"
#include "RoomPlatformGridMgr.h"
#include "RoomGridTemplate.h"
#include "ToolBase.h"
#include "TRPersistentDataComponent.h"
#include "TRPlayerControllerBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToolInventoryAdded, const FToolData&, ToolDataAdded);

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

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		UTRPersistentDataComponent* PersistentDataComponent;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		TMap<FGuid, FToolData> ToolInventory;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		TArray<UToolBase*> EquippedTools;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		UToolBase* CurrentTool;

	// Delegate event when Tool has been added to ToolInventory.
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnToolInventoryAdded OnToolInventoryAdded;
	
public:

	// [Server]
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerAddToolToInventory(TSubclassOf<UToolBase> ToolClass);

	UFUNCTION(Client, Reliable, BlueprintCallable, WithValidation)
		void ClientAddToolToInventory(TSubclassOf<UToolBase> ToolClass);

	// [Any]
	// Actually does the adding to inventory. Do not call directly. Call ServerAddToolToInventory.
	UFUNCTION()
		void AddToolToInventory(TSubclassOf<UToolBase> ToolClass);

	// Finds the current grid manager in the level.
	// Returns nullptr if no grid manager could be found.
	UFUNCTION(BlueprintCallable)
		ARoomPlatformGridMgr* FindGridManager();

	// Called from server to update the client's room grid template
	UFUNCTION(BlueprintCallable, Client, Reliable)
		void ClientUpdateRoomGridTemplate(const FRoomGridTemplate& UpdatedTemplate, const TArray<FVector2D>& RoomCoords, const TArray<FRoomTemplate>& RoomTemplates);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void GetPlayerSaveData(FPlayerSaveData& SaveData);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void UpdateFromPlayerSaveData(const FPlayerSaveData& SaveData);
};
