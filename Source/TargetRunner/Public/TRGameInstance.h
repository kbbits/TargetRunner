// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Delegates/Delegate.h"
#include "LevelTemplate.h"
#include "LevelForgeBase.h"
#include "LevelTemplateContext.h"
#include "PlayerLevelRecord.h"

#include "TRGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelTemplatesSaved, const bool, bSuccessful);

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API UTRGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UTRGameInstance();

public:
	// [Client]
	// Store the local client's profile name in the game instance so it is always available locally.
	UPROPERTY(BlueprintReadWrite)
		FName ClientLocalProfileName;

	// [Client]
	// Store the local client's profile name in the game instance so it is always available locally.
	UPROPERTY(BlueprintReadWrite)
		FGuid ClientLocalPlayerGuid;

	// [Server]
	UPROPERTY(BlueprintReadWrite, Replicated)
		FGuid HostProfileGuid;

	// The LevelForge class to use.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<ULevelForgeBase> DefaultLevelForgeClass;

	// [Server]
	// Array of all level templates. Will only be valid on server.
	//UPROPERTY(BlueprintReadWrite)
	//	TArray<FLevelTemplate> LevelTemplates;

	// [Server]
	// Map of all level templates with LevelId as key. Will only be valid on server.
	UPROPERTY(BlueprintReadWrite)
		TMap<FName, ULevelTemplateContext*> LevelTemplatesMap;

	UPROPERTY(BlueprintReadWrite)
		bool bLevelTemplatesLoaded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRandomStream LevelRandStream;

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnLevelTemplatesSaved OnLevelTemplatesSaved;

protected:

	// Level Template selected by user to be used for next game match. Will be set in TRGameState.
	// Between lobby and main game this keeps the selected level template.
	UPROPERTY(BlueprintSetter=SetSelectedLevelTemplate, BlueprintGetter=GetSelectedLevelTemplate)
		FLevelTemplate SelectedLevelTemplate;

	/* =======  Functions ======= */

protected:

	// [Server Only]
	// The filename of the level templates data. 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		FString GetLevelTemplatesSaveFilename();

	// [Server Only]
	// The filename of the level templates data. 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		FString GetPlayerRecordsSaveFilename();

public:

	// [Server]
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void ServerSaveAllPlayerData();

	// [Server Only]
	UFUNCTION(BlueprintCallable)
		ULevelTemplateContext* GenerateNewLevelTemplate(const float Tier);

	// [Server only]
	UFUNCTION(BlueprintCallable)
		void SetSelectedLevelTemplate(const FLevelTemplate& LevelTemplate);

	// [Server only]
	UFUNCTION(BlueprintPure)
		FLevelTemplate& GetSelectedLevelTemplate();

	// [Server Only]
	// This will call SavePlayerRecordsData. No need to call it manualy.
	UFUNCTION(BlueprintNativeEvent)
		void SaveLevelTemplatesData();

	// [Server Only]
	UFUNCTION(BlueprintNativeEvent)
		void SavePlayerRecordsData();

	// [Server Only]
	UFUNCTION(BlueprintNativeEvent)
		void LoadLevelTemplatesData();

	// [Async callback]
	UFUNCTION()
		void OnLevelTemplatesSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccessful);

	// [Async callback]
	UFUNCTION()
		void OnPlayerRecordsSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccessful);

	// [Server only]
	UFUNCTION(BlueprintCallable)
		ULevelTemplateContext* UnlockLevelTemplateForPlayer(const FName LevelId, const FGuid PlayerGuid);

	// [Server Only]
	UFUNCTION(BlueprintCallable)
		TArray<ULevelTemplateContext*> GetLevelTemplatesForPlayer(const FGuid PlayerGuid);

	// [Server Only]
	UFUNCTION(BlueprintCallable)
		FPlayerLevelRecord GetLevelTemplatePlayerRecord(const FName LevelId, const FGuid PlayerGuid);

	// [Server Only]
	UFUNCTION(BlueprintCallable)
		void UpdateLevelTemplatePlayerRecord(const FPlayerLevelRecord& PlayerRecord);
};
