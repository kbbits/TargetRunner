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
 * Most functions are to be called on the server only.
 */
UCLASS()
class TARGETRUNNER_API UTRGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UTRGameInstance();

public:
	// Store the local client's profile name in the game instance so it is always available locally.
	UPROPERTY(BlueprintReadWrite)
		FName ClientLocalProfileName;

	// Store the local client's profile guid in the game instance so it is always available locally.
	UPROPERTY(BlueprintReadWrite)
		FGuid ClientLocalPlayerGuid;

	// The player GUID of the host
	UPROPERTY(BlueprintReadWrite)
		FGuid HostProfileGuid;

	// The LevelForge class to use.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<ULevelForgeBase> DefaultLevelForgeClass;

	// Map of all level templates with LevelId as key. Will be different on each client/server.
	UPROPERTY(BlueprintReadWrite)
		TMap<FName, ULevelTemplateContext*> LevelTemplatesMap;

	UPROPERTY(BlueprintReadWrite)
		bool bLevelTemplatesLoaded = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRandomStream LevelRandStream;

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnLevelTemplatesSaved OnLevelTemplatesSaved;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bEnableClassDebug = false;

protected:

	// Level Template selected by user to be used for next game match. Will be set in TRGameState.
	// Between lobby and main game this keeps the selected level template.
	UPROPERTY(BlueprintSetter=SetSelectedLevelTemplate, BlueprintGetter=GetSelectedLevelTemplate)
		FLevelTemplate SelectedLevelTemplate;


	/* =======  Functions ======= */

protected:

	// The filename of the level templates data. 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		FString GetLevelTemplatesSaveFilename();

	// The filename of the level templates data. 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		FString GetPlayerRecordsSaveFilename();

public:
		
	UFUNCTION(BlueprintCallable)
		ULevelTemplateContext* GenerateNewLevelTemplate(const float Tier, const int32 DifficultyLevel);

	UFUNCTION(BlueprintCallable)
		void AddLevelTemplate(const FLevelTemplate& LevelTemplate);

	UFUNCTION(BlueprintCallable)
		void SetSelectedLevelTemplate(const FLevelTemplate& LevelTemplate);

	UFUNCTION(BlueprintPure)
		FLevelTemplate& GetSelectedLevelTemplate();

	// This will call SavePlayerRecordsData. No need to call it manualy.
	UFUNCTION(BlueprintNativeEvent)
		void SaveLevelTemplatesData();

	// Saves the level template data related to each player. (template unlock, total runs, etc.)
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

	UFUNCTION(BlueprintCallable)
		ULevelTemplateContext* UnlockLevelTemplateForPlayer(const FName LevelId, const FGuid PlayerGuid);

	UFUNCTION(BlueprintCallable)
		TArray<ULevelTemplateContext*> GetLevelTemplatesForPlayer(const FGuid PlayerGuid);

	UFUNCTION(BlueprintCallable)
		FPlayerLevelRecord GetLevelTemplatePlayerRecord(const FName LevelId, const FGuid PlayerGuid);

	UFUNCTION(BlueprintCallable)
		void UpdateLevelTemplatePlayerRecord(const FPlayerLevelRecord& PlayerRecord);
};
