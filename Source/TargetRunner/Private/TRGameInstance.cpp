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


void UTRGameInstance::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTRGameInstance, HostProfileName);
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
	UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - generating new level template for seed: %d. (stream seed: %d)"), NewSeed, LevelRandStream.GetCurrentSeed());
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
	// Debug log
	UE_LOG(LogTRGame, Log, TEXT("TRGameInstance - Set level template %s seed: %s"), *LevelTemplate.DisplayName.ToString(), *LevelTemplate.LevelId.ToString());
}


FLevelTemplate& UTRGameInstance::GetSelectedLevelTemplate()
{
	return SelectedLevelTemplate;
}


void UTRGameInstance::SaveLevelTemplatesData_Implementation()
{
	UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Save level templates: %d"), LevelTemplatesMap.Num());
	ULevelTemplatesSave* SaveGame = Cast<ULevelTemplatesSave>(UGameplayStatics::CreateSaveGameObject(ULevelTemplatesSave::StaticClass()));
	TArray<FLevelTemplate> AllTemplates;
	for (TPair<FName, ULevelTemplateContext*> LtcElem : LevelTemplatesMap)
	{
		AllTemplates.Add(LtcElem.Value->LevelTemplate);
	}
	SaveGame->LevelTemplates = AllTemplates;
	FAsyncSaveGameToSlotDelegate Callback = FAsyncSaveGameToSlotDelegate::CreateUObject(this, &UTRGameInstance::OnLevelTemplatesSaveComplete);	
	UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Saving %d templates"), AllTemplates.Num());
	UGameplayStatics::AsyncSaveGameToSlot(SaveGame, GetLevelTemplatesSaveFilename(), 0, Callback);
}


void UTRGameInstance::SavePlayerRecordsData_Implementation()
{
	UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Save player level records"));
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
	UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Saving %d player level records"), AllPLRs.Num());
	UGameplayStatics::AsyncSaveGameToSlot(SaveGame, GetPlayerRecordsSaveFilename(), 0, Callback);
}


void UTRGameInstance::LoadLevelTemplatesData_Implementation()
{
	UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Load level templates - Already loaded: %s"), bLevelTemplatesLoaded ? TEXT("true") : TEXT("false"));
	if (bLevelTemplatesLoaded) { return; }
	bLevelTemplatesLoaded = false;
	//LevelTemplatesMap.Empty();
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
			UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Load level templates loaded %d"), LevelTemplatesMap.Num());
		}
		else
		{
			UE_LOG(LogTRGame, Warning, TEXT("UTRGameInstance - LoadLevelTemplates -  No level templates save file found."));
		}
	}
	if (bLevelTemplatesLoaded)
	{
		if (UGameplayStatics::DoesSaveGameExist(GetPlayerRecordsSaveFilename(), 0))
		{
			int32 PlayerRecordsLoaded = 0;
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
						PlayerRecordsLoaded++;
					}
					else
					{
						UE_LOG(LogTRGame, Warning, TEXT("UTRGameInstance - LoadLevelTemplates - No level template with id: %s"), *TmpPlayerRecord.LevelId.ToString());
					}
				}
				UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Load level templates loaded %d player records"), PlayerRecordsLoaded);
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
		UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Save level templates successful"));
		SavePlayerRecordsData(); 
	}
	else { 
		UE_LOG(LogTRGame, Warning, TEXT("UTRGameInstance - Save level templates failed"));
		OnLevelTemplatesSaved.Broadcast(bSuccessful); 
	}	
}


void UTRGameInstance::OnPlayerRecordsSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccessful)
{
	UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Save player level records %s"), bSuccessful ? TEXT("success") : TEXT("failed"));
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
			UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - UnlockLevelTemplateForPlayer updating for player: %s"), *PlayerGuid.ToString(EGuidFormats::Digits));
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
			UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - UnlockLevelTemplateForPlayer new record for player: %s"), *PlayerGuid.ToString(EGuidFormats::Digits));
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
