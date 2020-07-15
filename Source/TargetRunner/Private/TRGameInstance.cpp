// Fill out your copyright notice in the Description page of Project Settings.


#include "TRGameInstance.h"
#include "..\Public\TRGameInstance.h"
#include "Kismet/GameplayStatics.h"
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
	return FString::Printf(TEXT("%s_LevelTemplates"), *HostProfileName.ToString());
}


FString UTRGameInstance::GetPlayerRecordsSaveFilename_Implementation()
{
	return FString::Printf(TEXT("%s_PlayerLevelRecords"), *HostProfileName.ToString());
}


ULevelTemplateContext* UTRGameInstance::GenerateNewLevelTemplate(const float Tier)
{
	bool bSuccessful = false;
	ULevelForgeBase* LevelForge = NewObject<ULevelForgeBase>(this, DefaultLevelForgeClass);
	if (LevelForge == nullptr) {
		UE_LOG(LogTRGame, Error, TEXT("UTRGameInstance - GenerateNewLevelTemplate Could not create LevelForge %s"), *DefaultLevelForgeClass->GetName());
		return nullptr;
	}
	ULevelTemplateContext* NewLTC = NewObject<ULevelTemplateContext>(this);
	//FLevelTemplate TmpLevelTemplate = FLevelTemplate();
	int32 NewSeed = static_cast<int32>(LevelRandStream.FRandRange(static_cast<float>(INT_MIN + 10), static_cast<float>(INT_MAX - 10)));
	UE_LOG(LogTRGame, Log, TEXT("PersistentDataComponent - generating new level template for seed: %d. (stream seed: %d)"), NewSeed, LevelRandStream.GetCurrentSeed());
	LevelForge->GenerateNewLevelTemplate(NewSeed, Tier, NewLTC->LevelTemplate, bSuccessful);
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
}


FLevelTemplate& UTRGameInstance::GetSelectedLevelTemplate()
{
	return SelectedLevelTemplate;
}


void UTRGameInstance::SaveLevelTemplatesData_Implementation()
{
	//UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Save level templates starting"));
	ULevelTemplatesSave* SaveGame = Cast<ULevelTemplatesSave>(UGameplayStatics::CreateSaveGameObject(ULevelTemplatesSave::StaticClass()));
	TArray<FLevelTemplate> AllTemplates;
	for (TPair<FName, ULevelTemplateContext*> LtcElem : LevelTemplatesMap)
	{
		AllTemplates.Add(LtcElem.Value->LevelTemplate);
	}
	SaveGame->LevelTemplates = AllTemplates;
	FAsyncSaveGameToSlotDelegate Callback = FAsyncSaveGameToSlotDelegate::CreateUObject(this, &UTRGameInstance::OnLevelTemplatesSaveComplete);	
	UGameplayStatics::AsyncSaveGameToSlot(SaveGame, GetLevelTemplatesSaveFilename(), 0, Callback);
}


void UTRGameInstance::SavePlayerRecordsData_Implementation()
{
	UPlayerLevelRecordsSave* SaveGame = Cast<UPlayerLevelRecordsSave>(UGameplayStatics::CreateSaveGameObject(UPlayerLevelRecordsSave::StaticClass()));
	FAsyncSaveGameToSlotDelegate Callback = FAsyncSaveGameToSlotDelegate::CreateUObject(this, &UTRGameInstance::OnPlayerRecordsSaveComplete);
	// Put all player records in single array.
	TArray<FPlayerLevelRecord> AllPLRs;
	TArray<FPlayerLevelRecord> TmpPLRs;
	for (TPair<FName, ULevelTemplateContext*> LtcElem : LevelTemplatesMap)
	{
		LtcElem.Value->PlayerRecords.GenerateValueArray(TmpPLRs);
		AllPLRs.Append(TmpPLRs);
	}
	SaveGame->PlayerRecords = AllPLRs;
	UGameplayStatics::AsyncSaveGameToSlot(SaveGame, GetPlayerRecordsSaveFilename(), 0, Callback);
}


void UTRGameInstance::LoadLevelTemplatesData_Implementation()
{
	UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Load level templates - Already loaded: %s"), bLevelTemplatesLoaded ? TEXT("true") : TEXT("false"));
	if (bLevelTemplatesLoaded) { return; }
	bLevelTemplatesLoaded = false;
	LevelTemplatesMap.Empty();
	if (UGameplayStatics::DoesSaveGameExist(GetLevelTemplatesSaveFilename(), 0))
	{
		ULevelTemplatesSave* SaveGame = Cast<ULevelTemplatesSave>(UGameplayStatics::LoadGameFromSlot(GetLevelTemplatesSaveFilename(), 0));
		if (SaveGame)
		{
			ULevelTemplateContext* NewLTC = nullptr;
			for (FLevelTemplate TmpTemplate : SaveGame->LevelTemplates)
			{
				NewLTC = NewObject<ULevelTemplateContext>(this);
				NewLTC->LevelTemplate = TmpTemplate;
				LevelTemplatesMap.Add(NewLTC->LevelTemplate.LevelId, NewLTC);
			}
			bLevelTemplatesLoaded = true;
		}
	}
	if (bLevelTemplatesLoaded)
	{
		if (UGameplayStatics::DoesSaveGameExist(GetPlayerRecordsSaveFilename(), 0))
		{
			UPlayerLevelRecordsSave* PlayerSaveGame = Cast<UPlayerLevelRecordsSave>(UGameplayStatics::LoadGameFromSlot(GetPlayerRecordsSaveFilename(), 0));
			if (PlayerSaveGame)
			{
				ULevelTemplateContext* TmpLTC = nullptr;
				for (FPlayerLevelRecord TmpPlayerRecord : PlayerSaveGame->PlayerRecords)
				{
					TmpLTC = LevelTemplatesMap.FindRef(TmpPlayerRecord.LevelId);
					if (TmpLTC)
					{
						TmpLTC->PlayerRecords.Add(TmpPlayerRecord.PlayerGuid, TmpPlayerRecord);
					}
				}
			}
			else
			{
				UE_LOG(LogTRGame, Warning, TEXT("UTRGameInstance - No player level records save file found."));
			}
		}
	}
}


void UTRGameInstance::OnLevelTemplatesSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccessful)
{
	if (bSuccessful) { 
		SavePlayerRecordsData(); 
	}
	else { 
		OnLevelTemplatesSaved.Broadcast(bSuccessful); 
	}	
}


void UTRGameInstance::OnPlayerRecordsSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccessful)
{
	// Just calls the delegate for notifications
	OnLevelTemplatesSaved.Broadcast(bSuccessful);
}


ULevelTemplateContext* UTRGameInstance::UnlockLevelTemplateForPlayer(const FName LevelId, const FGuid PlayerGuid)
{
	ULevelTemplateContext* TargetTemplate = nullptr;
	LoadLevelTemplatesData();
	TargetTemplate = LevelTemplatesMap.FindRef(LevelId);
	if (TargetTemplate)
	{
		if (TargetTemplate->PlayerRecords.Contains(PlayerGuid))
		{
			TargetTemplate->PlayerRecords[PlayerGuid].Unlocked = true;
			TargetTemplate->PlayerRecords[PlayerGuid].UnlockedAt = FDateTime::Now();
		}
		else
		{
			FPlayerLevelRecord NewLevelRecord;
			NewLevelRecord.PlayerGuid = PlayerGuid;
			NewLevelRecord.LevelId = LevelId;
			NewLevelRecord.Unlocked = true;
			NewLevelRecord.UnlockedAt = FDateTime::Now();
			TargetTemplate->PlayerRecords.Add(PlayerGuid, NewLevelRecord);
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
