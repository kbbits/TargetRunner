// Fill out your copyright notice in the Description page of Project Settings.


#include "TRGameInstance.h"
#include "..\Public\TRGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "LevelTemplatesSave.h"

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


FLevelTemplate UTRGameInstance::GenerateNewLevelTemplate(const float Tier)
{
	bool bSuccessful = false;
	ULevelForgeBase* LevelForge = NewObject<ULevelForgeBase>(this, DefaultLevelForgeClass);
	if (LevelForge == nullptr) {
		UE_LOG(LogTRGame, Error, TEXT("UTRGameInstance - GenerateNewLevelTemplate Could not create LevelForge %s"), *DefaultLevelForgeClass->GetName());
		return FLevelTemplate();
	}
	FLevelTemplate TmpLevelTemplate = FLevelTemplate();
	int32 NewSeed = static_cast<int32>(LevelRandStream.FRandRange(static_cast<float>(INT_MIN + 10), static_cast<float>(INT_MAX - 10)));
	UE_LOG(LogTRGame, Log, TEXT("PersistentDataComponent - generating new level template for seed: %d. (stream seed: %d)"), NewSeed, LevelRandStream.GetCurrentSeed());
	LevelForge->GenerateNewLevelTemplate(NewSeed, Tier, TmpLevelTemplate, bSuccessful);
	if (bSuccessful)
	{
		LevelTemplates.Add(TmpLevelTemplate);
	}
	else
	{
		UE_LOG(LogTRGame, Error, TEXT("UTRGameInstance - Error generating new level template."));
	}
	return TmpLevelTemplate;
}


void UTRGameInstance::SetLevelTemplate(const FLevelTemplate& LevelTemplate)
{
	SelectedLevelTemplate = LevelTemplate;
}


FLevelTemplate& UTRGameInstance::GetLevelTemplate()
{
	return SelectedLevelTemplate;
}


void UTRGameInstance::SaveLevelTemplatesData_Implementation()
{
	//UE_LOG(LogTRGame, Log, TEXT("UTRGameInstance - Save level templates starting"));
	ULevelTemplatesSave* SaveGame = Cast<ULevelTemplatesSave>(UGameplayStatics::CreateSaveGameObject(ULevelTemplatesSave::StaticClass()));
	SaveGame->LevelTemplates = LevelTemplates;
	FAsyncSaveGameToSlotDelegate Callback = FAsyncSaveGameToSlotDelegate::CreateUObject(this, &UTRGameInstance::OnLevelTemplatesSaveComplete);	
	UGameplayStatics::AsyncSaveGameToSlot(SaveGame, GetLevelTemplatesSaveFilename(), 0, Callback);
}


void UTRGameInstance::LoadLevelTemplatesData_Implementation()
{
	if (bLevelTemplatesLoaded) { return; }
	bLevelTemplatesLoaded = false;
	LevelTemplates.Empty();
	if (UGameplayStatics::DoesSaveGameExist(GetLevelTemplatesSaveFilename(), 0))
	{
		ULevelTemplatesSave* SaveGame = Cast<ULevelTemplatesSave>(UGameplayStatics::LoadGameFromSlot(GetLevelTemplatesSaveFilename(), 0));
		if (SaveGame)
		{
			LevelTemplates = SaveGame->LevelTemplates;
			bLevelTemplatesLoaded = true;
		}
	}
}


void UTRGameInstance::OnLevelTemplatesSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccessful)
{
	// Just calls the delegate for notifications
	OnLevelTemplatesSaved.Broadcast(bSuccessful);
}

FLevelTemplate UTRGameInstance::UnlockLevelTemplateForPlayer(const FName LevelId, const FGuid PlayerGuid)
{
	FLevelTemplate* TargetTemplate = nullptr;
	LoadLevelTemplatesData();
	for (FLevelTemplate TmpTemplate : LevelTemplates)
	{
		if (TmpTemplate.LevelId == LevelId)
		{
			TargetTemplate = &TmpTemplate;
			break;
		}
	}
	if (TargetTemplate)
	{
		if (TargetTemplate->PlayerLevelRecords.Contains(PlayerGuid))
		{
			TargetTemplate->PlayerLevelRecords[PlayerGuid].Unlocked = true;
			TargetTemplate->PlayerLevelRecords[PlayerGuid].UnlockedAt = FDateTime::Now();
		}
		else
		{
			FPlayerLevelRecord NewLevelRecord;
			NewLevelRecord.PlayerGuid = PlayerGuid;
			NewLevelRecord.LevelId = LevelId;
			NewLevelRecord.Unlocked = true;
			NewLevelRecord.UnlockedAt = FDateTime::Now();
			TargetTemplate->PlayerLevelRecords.Add(PlayerGuid, NewLevelRecord);
		}
		return *TargetTemplate;
	}	
	else
	{
		UE_LOG(LogTRGame, Error, TEXT("GameInstance - UnlockLevelTemplateForPlayer - no level template with id: %s"), *LevelId.ToString());
		return FLevelTemplate();
	}
}

FPlayerLevelRecord UTRGameInstance::GetLevelTemplatePlayerRecord(const FName LevelId, const FGuid PlayerGuid)
{
	FLevelTemplate* TargetTemplate = nullptr;
	LoadLevelTemplatesData();
	for (FLevelTemplate TmpTemplate : LevelTemplates)
	{
		if (TmpTemplate.LevelId == LevelId)
		{
			TargetTemplate = &TmpTemplate;
			break;
		}
	}
	if (TargetTemplate)
	{
		return TargetTemplate->PlayerLevelRecords.FindRef(PlayerGuid);
	}
	return FPlayerLevelRecord();
}

void UTRGameInstance::UpdateLevelTemplatePlayerRecord(const FPlayerLevelRecord& PlayerRecord)
{
	FLevelTemplate* TargetTemplate = nullptr;
	LoadLevelTemplatesData();
	for (FLevelTemplate TmpTemplate : LevelTemplates)
	{
		if (TmpTemplate.LevelId == PlayerRecord.LevelId)
		{
			TargetTemplate = &TmpTemplate;
			break;
		}
	}
	if (TargetTemplate)
	{
		TargetTemplate->PlayerLevelRecords.Add(PlayerRecord.PlayerGuid, PlayerRecord);
		//if (TargetTemplate->PlayerLevelRecords.Contains(PlayerRecord.PlayerGuid))
		//{
		//	TargetTemplate->PlayerLevelRecords[PlayerRecord.PlayerGuid]
		//}
		//else
		//{
		//}
	}
}
