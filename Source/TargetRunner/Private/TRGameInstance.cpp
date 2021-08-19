// Fill out your copyright notice in the Description page of Project Settings.


#include "TRGameInstance.h"
#include "..\Public\TRGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "TRPlayerControllerBase.h"
#include "LevelTemplatesSave.h"
#include "PlayerLevelRecordsSave.h"

UTRGameInstance::UTRGameInstance()
	: Super()
{
	// TODO:  Set this from last saved value (if any)
	LevelRandStream.GenerateNewSeed();	
}


FString UTRGameInstance::GetLevelTemplatesSaveFilename_Implementation()
{
	return FString::Printf(TEXT("%s_LT"), *HostProfileGuid.ToString(EGuidFormats::Digits));
	//return FString::Printf(TEXT("GlobalLevelTemplates_LT"));
}


FString UTRGameInstance::GetPlayerRecordsSaveFilename_Implementation()
{
	//return FString::Printf(TEXT("%s_PLR"), *HostProfileGuid.ToString(EGuidFormats::Digits));
	return FString::Printf(TEXT("GlobalPLRs"));
}


void UTRGameInstance::SaveAllPlayerData()
{
	ATRPlayerControllerBase* PController = nullptr;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		PController = Cast<ATRPlayerControllerBase>(*Iterator);
		if (IsValid(PController))
		{
			PController->PersistentDataComponent->ServerSavePlayerData();
		}
	}
}

void UTRGameInstance::ReloadAllPlayerData()
{
	ATRPlayerControllerBase* PController = nullptr;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		PController = Cast<ATRPlayerControllerBase>(*Iterator);
		if (IsValid(PController))
		{
			PController->PersistentDataComponent->ServerLoadPlayerData();
		}
	}
}


ULevelTemplateContext* UTRGameInstance::GenerateNewLevelTemplate(const float Tier, const int32 DifficultyLevel)
{
	bool bSuccessful = false;
	ULevelForgeBase* LevelForge = NewObject<ULevelForgeBase>(this, DefaultLevelForgeClass);
	if (LevelForge == nullptr) {
		UE_LOG(LogTRGame, Error, TEXT("UTRGameInstance - GenerateNewLevelTemplate Could not create LevelForge %s"), *DefaultLevelForgeClass->GetName());
		return nullptr;
	}
	ULevelTemplateContext* NewLTC = NewObject<ULevelTemplateContext>(this);
	int32 NewSeed = LevelRandStream.RandRange(1, INT_MAX - 1);
	//UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - generating new level template for seed: %d. (stream seed: %d)"), NewSeed, LevelRandStream.GetCurrentSeed());
	LevelForge->GenerateNewLevelTemplate(NewSeed, Tier, DifficultyLevel, NewLTC->LevelTemplate, bSuccessful);
	if (bSuccessful)
	{
		LevelTemplatesMap.Add(NewLTC->LevelTemplate.LevelId, NewLTC);
	}
	else
	{
		UE_LOG(LogTRGame, Error, TEXT("UTRGameInstance - Error generating new level template."));
	}
	return NewLTC;
}


void UTRGameInstance::SetSelectedLevelTemplate(const FLevelTemplate& LevelTemplate)
{
	SelectedLevelTemplate = LevelTemplate;
	// Debug log
	UE_LOG(LogTRGame, Log, TEXT("TRGameInstance - Set level template %s seed: %s"), *LevelTemplate.DisplayName.ToString(), *LevelTemplate.LevelId.ToString());
}


FLevelTemplate& UTRGameInstance::GetSelectedLevelTemplate()
{
	return SelectedLevelTemplate;
}


void UTRGameInstance::SaveLevelTemplatesData_Implementation()
{
	//UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Save level templates"));
	ULevelTemplatesSave* SaveGame = Cast<ULevelTemplatesSave>(UGameplayStatics::CreateSaveGameObject(ULevelTemplatesSave::StaticClass()));
	TArray<FLevelTemplate> AllTemplates;
	for (TPair<FName, ULevelTemplateContext*> LtcElem : LevelTemplatesMap)
	{
		AllTemplates.Add(LtcElem.Value->LevelTemplate);
	}
	SaveGame->LevelTemplates = AllTemplates;
	FAsyncSaveGameToSlotDelegate Callback = FAsyncSaveGameToSlotDelegate::CreateUObject(this, &UTRGameInstance::OnLevelTemplatesSaveComplete);	
	//UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Saving %d templates"), AllTemplates.Num());
	UGameplayStatics::AsyncSaveGameToSlot(SaveGame, GetLevelTemplatesSaveFilename(), 0, Callback);
}


void UTRGameInstance::SavePlayerRecordsData_Implementation()
{
	//UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Save player level records"));
	UPlayerLevelRecordsSave* SaveGame = nullptr;
	// First get the existing player records. We only have records for level templates used by the host profile now.
	// But PLRs are stored globally. So, the save file will have data we don't have loaded. We don't want to overwrite it.
	if (UGameplayStatics::DoesSaveGameExist(GetPlayerRecordsSaveFilename(), 0))
	{
		SaveGame = Cast<UPlayerLevelRecordsSave>(UGameplayStatics::LoadGameFromSlot(GetPlayerRecordsSaveFilename(), 0));		
	}
	if (!SaveGame)
	{
		SaveGame = Cast<UPlayerLevelRecordsSave>(UGameplayStatics::CreateSaveGameObject(UPlayerLevelRecordsSave::StaticClass()));
	}
	FAsyncSaveGameToSlotDelegate Callback = FAsyncSaveGameToSlotDelegate::CreateUObject(this, &UTRGameInstance::OnPlayerRecordsSaveComplete);
	// Put all player records in single array.
	TArray<FPlayerLevelRecord> AllPLRs;
	TArray<FPlayerLevelRecord> TmpPLRs;
	int32 SavePLRIndex = INDEX_NONE;
	int32 RecordsUpdated = 0;
	int32 RecordsAdded = 0;
	// Each LevelTemplatesMap value is a sub-map of player guids to a PlayerLevelRecord.
	for (TPair<FName, ULevelTemplateContext*> LtcElem : LevelTemplatesMap)
	{
		// Get each level's PlayerLevelRecords from the map and add it to our array.
		LtcElem.Value->PlayerRecords.GenerateValueArray(TmpPLRs);
		AllPLRs.Append(TmpPLRs);
	}
	if (SaveGame->PlayerRecords.Num() > 0)
	{
		// Find existing records in save file and update it or add one if not found.
		for (FPlayerLevelRecord CurPLR : AllPLRs)
		{
			// FPlayerLevelRecords implement the == comparison operator. So we can use IndexOfByKey.
			SavePLRIndex = SaveGame->PlayerRecords.IndexOfByKey<FPlayerLevelRecord>(CurPLR);
			if (SavePLRIndex == INDEX_NONE)
			{
				SaveGame->PlayerRecords.Add(CurPLR);
				RecordsAdded++;
			}
			else
			{
				SaveGame->PlayerRecords[SavePLRIndex] = CurPLR;
				RecordsUpdated++;
			}
		}
	}
	else
	{
		// Just add all the PLRs to an empty save file
		SaveGame->PlayerRecords = AllPLRs;
		RecordsAdded = AllPLRs.Num();
	}
	//UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Saving player level records: Added %d  Updated %d  Total from game %d  Total in file %d"), RecordsAdded, RecordsUpdated, AllPLRs.Num(), SaveGame->PlayerRecords.Num());
	UGameplayStatics::AsyncSaveGameToSlot(SaveGame, GetPlayerRecordsSaveFilename(), 0, Callback);
}


void UTRGameInstance::LoadLevelTemplatesData_Implementation()
{
	//UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Load level templates - Already loaded: %s"), bLevelTemplatesLoaded ? TEXT("true") : TEXT("false"));
	if (bLevelTemplatesLoaded) { return; }
	bLevelTemplatesLoaded = false;
	//LevelTemplatesMap.Empty();
	if (UGameplayStatics::DoesSaveGameExist(GetLevelTemplatesSaveFilename(), 0))
	{
		ULevelTemplatesSave* SaveGame = Cast<ULevelTemplatesSave>(UGameplayStatics::LoadGameFromSlot(GetLevelTemplatesSaveFilename(), 0));
		if (SaveGame)
		{
			// Add each level template from the save into our LevelTemplatesMap
			ULevelTemplateContext* NewLTC = nullptr;
			for (FLevelTemplate TmpTemplate : SaveGame->LevelTemplates)
			{
				NewLTC = NewObject<ULevelTemplateContext>(this);
				NewLTC->LevelTemplate = TmpTemplate;
				LevelTemplatesMap.Add(NewLTC->LevelTemplate.LevelId, NewLTC);
			}
			bLevelTemplatesLoaded = true;
			//UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Load level templates loaded %d"), LevelTemplatesMap.Num());
		}
		else
		{
			UE_LOG(LogTRGame, Warning, TEXT("UTRGameInstance - LoadLevelTemplates -  No level templates save file found."));
		}
	}
	if (bLevelTemplatesLoaded)
	{
		// Load the player records
		if (UGameplayStatics::DoesSaveGameExist(GetPlayerRecordsSaveFilename(), 0))
		{
			int32 PlayerRecordsLoaded = 0;
			UPlayerLevelRecordsSave* PlayerSaveGame = Cast<UPlayerLevelRecordsSave>(UGameplayStatics::LoadGameFromSlot(GetPlayerRecordsSaveFilename(), 0));
			if (PlayerSaveGame)
			{
				// Load the "global" player level records from the save and add them to the sub-map for each level in our LevelTemplatesMap
				ULevelTemplateContext* TmpLTC = nullptr;
				for (FPlayerLevelRecord TmpPlayerRecord : PlayerSaveGame->PlayerRecords)
				{
					// Find the Level in our LevelTemplatesMap
					TmpLTC = LevelTemplatesMap.FindRef(TmpPlayerRecord.LevelId);
					if (TmpLTC)
					{
						// Add (or update) the player level record in the found level template's player records sub-map.
						TmpLTC->PlayerRecords.Add(TmpPlayerRecord.PlayerGuid, TmpPlayerRecord);
						PlayerRecordsLoaded++;
					}
					else
					{
						// May find player level records for levels we don't have. This is expected.
						//UE_LOG(LogTRGame, Warning, TEXT("UTRGameInstance - LoadLevelTemplates - No level template with id: %s"), *TmpPlayerRecord.LevelId.ToString());
					}
				}
				//UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Load level templates loaded %d player records"), PlayerRecordsLoaded);
			}
			else
			{
				UE_LOG(LogTRGame, Warning, TEXT("UTRGameInstance - LoadLevelTemplates - No player level records save file found."));
			}
		}
	}
}


void UTRGameInstance::OnLevelTemplatesSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccessful)
{
	if (bSuccessful) { 
		// After successful save of template data, save the associated player level records
		//UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Save level templates successful"));
		SavePlayerRecordsData(); 
	}
	else { 
		UE_LOG(LogTRGame, Warning, TEXT("UTRGameInstance %s - Save level templates failed"), (GetWorld()->IsServer() ? TEXT("Server") : TEXT("Client")));
		OnLevelTemplatesSaved.Broadcast(bSuccessful); 
	}	
}


void UTRGameInstance::OnPlayerRecordsSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccessful)
{
	//UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Save player level records %s"), bSuccessful ? TEXT("success") : TEXT("failed"));
	// Just calls the delegate for notifications
	OnLevelTemplatesSaved.Broadcast(bSuccessful);
}


ULevelTemplateContext* UTRGameInstance::UnlockLevelTemplateForPlayer(const FName LevelId, const FGuid PlayerGuid)
{
	ULevelTemplateContext* TargetTemplate = nullptr;
	LoadLevelTemplatesData();
	// Find the level in the LevelTemplatesMap
	TargetTemplate = LevelTemplatesMap.FindRef(LevelId);
	if (TargetTemplate)
	{
		// If the level already has a player record for this player guid:
		if (TargetTemplate->PlayerRecords.Contains(PlayerGuid))
		{
			// Then update the player level record
			//UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - UnlockLevelTemplateForPlayer updating for player: %s"), *PlayerGuid.ToString(EGuidFormats::Digits));
			TargetTemplate->PlayerRecords[PlayerGuid].Unlocked = true;
			TargetTemplate->PlayerRecords[PlayerGuid].UnlockedAt = FDateTime::Now();
		}
		else
		{
			// Otherwise create a new player level record for the player guid
			FPlayerLevelRecord NewLevelRecord;
			NewLevelRecord.PlayerGuid = PlayerGuid;
			NewLevelRecord.LevelId = LevelId;
			NewLevelRecord.Unlocked = true;
			NewLevelRecord.UnlockedAt = FDateTime::Now();
			// And add the PLR to the level's player record sub-map
			TargetTemplate->PlayerRecords.Add(PlayerGuid, NewLevelRecord);
			//UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - UnlockLevelTemplateForPlayer new record for player: %s"), *PlayerGuid.ToString(EGuidFormats::Digits));
		}
		return TargetTemplate;
	}	
	else
	{
		UE_LOG(LogTRGame, Error, TEXT("GameInstance - UnlockLevelTemplateForPlayer - no level template with id: %s"), *LevelId.ToString());
		return nullptr;
	}
}


TArray<ULevelTemplateContext*> UTRGameInstance::GetLevelTemplatesForPlayer(const FGuid PlayerGuid)
{
	TArray<ULevelTemplateContext*> PlayerLTCs;
	for (TPair<FName, ULevelTemplateContext*> LtcElem : LevelTemplatesMap)
	{
		if (LtcElem.Value->PlayerRecords.Contains(PlayerGuid))
		{
			PlayerLTCs.Add(LtcElem.Value);
		}
	}
	return PlayerLTCs;
}


FPlayerLevelRecord UTRGameInstance::GetLevelTemplatePlayerRecord(const FName LevelId, const FGuid PlayerGuid)
{
	LoadLevelTemplatesData();
	ULevelTemplateContext* TargetTemplate = LevelTemplatesMap.FindRef(LevelId);
	if (TargetTemplate)
	{
		return TargetTemplate->PlayerRecords.FindRef(PlayerGuid);
	}
	return FPlayerLevelRecord();
}


void UTRGameInstance::UpdateLevelTemplatePlayerRecord(const FPlayerLevelRecord& PlayerRecord)
{
	LoadLevelTemplatesData();
	ULevelTemplateContext* TargetTemplate = LevelTemplatesMap.FindRef(PlayerRecord.LevelId);
	if (TargetTemplate)
	{
		TargetTemplate->PlayerRecords.Add(PlayerRecord.PlayerGuid, PlayerRecord);
	}
	else {
		UE_LOG(LogTRGame, Error, TEXT("GameInstance - UpdateLevelTemplatePlayerRecord - no level template with id: %s"), *PlayerRecord.LevelId.ToString());
	}
}
