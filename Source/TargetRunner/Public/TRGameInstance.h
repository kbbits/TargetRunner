// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Delegates/Delegate.h"
#include "LevelTemplate.h"
#include "LevelForgeBase.h"
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
	// [Server]
	UPROPERTY(BlueprintReadWrite)
		FName HostProfileName;

	// The LevelForge class to use.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<ULevelForgeBase> DefaultLevelForgeClass;

	// [Server]
	// Array of all level templates. Will only be valid on server.
	UPROPERTY(BlueprintReadWrite)
		TArray<FLevelTemplate> LevelTemplates;

	UPROPERTY(BlueprintReadWrite)
		bool bLevelTemplatesLoaded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRandomStream LevelRandStream;

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnLevelTemplatesSaved OnLevelTemplatesSaved;

protected:

	// Level Template selected by user to be used for next game match. Will be set in TRGameState.
	// Between lobby and main game this keeps the selected level template.
	UPROPERTY(BlueprintSetter=SetLevelTemplate, BlueprintGetter=GetLevelTemplate)
		FLevelTemplate SelectedLevelTemplate;

	/* =======  Functions ======= */

protected:

	// [Server Only]
	// The filename of the level templates data. 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		FString GetLevelTemplatesSaveFilename();

public:

	// [Server Only]
	UFUNCTION(BlueprintCallable)
		FLevelTemplate GenerateNewLevelTemplate(const float Tier);

	// [Server only]
	UFUNCTION(BlueprintCallable)
		void SetLevelTemplate(const FLevelTemplate& LevelTemplate);

	// [Server only]
	UFUNCTION(BlueprintPure)
		FLevelTemplate& GetLevelTemplate();

	// [Server Only]
	UFUNCTION(BlueprintNativeEvent)
		void SaveLevelTemplatesData();

	// [Server Only]
	UFUNCTION(BlueprintNativeEvent)
		void LoadLevelTemplatesData();

	// [Async callback]
	UFUNCTION()
		void OnLevelTemplatesSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccessful);

	// [Server only]
	UFUNCTION(BlueprintCallable)
		FLevelTemplate UnlockLevelTemplateForPlayer(const FName LevelId, const FGuid PlayerGuid);

	// [Server Only]
	UFUNCTION(BlueprintCallable)
		FPlayerLevelRecord GetLevelTemplatePlayerRecord(const FName LevelId, const FGuid PlayerGuid);

	// [Server Only]
	UFUNCTION(BlueprintCallable)
		void UpdateLevelTemplatePlayerRecord(const FPlayerLevelRecord& PlayerRecord);
};
