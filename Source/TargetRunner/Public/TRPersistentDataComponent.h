// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/Delegate.h"
#include "UnrealNetwork.h"
#include "LevelTemplate.h"
#include "LevelTemplateContextStruct.h"
#include "PlayerSaveData.h"
#include "TRPersistentDataComponent.generated.h"


// Event dispatcher for when CurrentValue changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewLevelTemplatesPage, const TArray<FLevelTemplateContextStruct>&, NewLevelTemplatesPage);
// Tool Data dispatcher
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewPlayerToolData, const TArray<FToolData>&, NewPlayerToolData);


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

	//UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_PlayerToolDataLoaded)
	//	TArray<FToolData> PlayerToolData;

	//UPROPERTY(ReplicatedUsing = OnRep_PlayerToolDataLoaded)
	//	int32 PlayerToolDataRepTrigger;

	//// Delegate event when PlayerToolData array has changed.
	//UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	//	FOnNewPlayerToolData OnNewPlayerToolData;

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
		void ServerUnlockLevelTemplateForPlayer(const FName LevelId, const FGuid PlayerGuid);
	
	// [Server]
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerSetLevelTemplateForPlay(const FLevelTemplate& LevelTemplate);

	// Player Save Data

	UFUNCTION(BlueprintPure)
		FString GetPlayerSaveFilename(); 

	// [Server]
	// Save the player's data
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerSavePlayerData();

	// [Server]
	// Load the player's data
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerLoadPlayerData();

	// [Client]
	// Echo loaded player data back to client
	UFUNCTION(Client, Reliable, BlueprintCallable, WithValidation)
		void ClientEchoLoadPlayerData(const FPlayerSaveData PlayerSaveData);
};
