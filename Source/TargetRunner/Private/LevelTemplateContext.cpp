// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelTemplateContext.h"
#include "..\Public\LevelTemplateContext.h"

ULevelTemplateContext::ULevelTemplateContext()
	: Super()
{
}


FPlayerLevelRecord* ULevelTemplateContext::PlayerRecord(const FGuid PlayerGuid)
{
	FPlayerLevelRecord* TmpRecord = PlayerRecords.Find(PlayerGuid);
	return TmpRecord;
}


void ULevelTemplateContext::GetPlayerRecord(const FGuid PlayerGuid, bool& bFound, FPlayerLevelRecord& PlayerLevelRecord)
{
	FPlayerLevelRecord* TmpRecord = PlayerRecord(PlayerGuid);
	if (TmpRecord)
	{
		bFound = true;
		PlayerLevelRecord = *TmpRecord;
	}
	else
	{
		bFound = false;
		PlayerLevelRecord = FPlayerLevelRecord();
	}
}


bool ULevelTemplateContext::IsUnlockedForPlayer(const FGuid PlayerGuid)
{
	FPlayerLevelRecord* TmpPlayerRecord = PlayerRecord(PlayerGuid);
	return TmpPlayerRecord && TmpPlayerRecord->Unlocked;
}


FLevelTemplateContextStruct ULevelTemplateContext::ToStruct()
{
	FLevelTemplateContextStruct NewStruct;
	NewStruct.LevelTemplate = LevelTemplate;
	PlayerRecords.GenerateValueArray(NewStruct.PlayerRecords);
	return NewStruct;
}


TArray<FLevelTemplateContextStruct> ULevelTemplateContext::ToStructArray(const TArray<ULevelTemplateContext*> InArray)
{
	TArray<FLevelTemplateContextStruct> StructArray;
	for (ULevelTemplateContext* TmpLTC : InArray)
	{
		StructArray.Add(TmpLTC->ToStruct());
	}
	return StructArray;
}


ULevelTemplateContext* ULevelTemplateContext::LevelTemplateContextFromStruct(const FLevelTemplateContextStruct& InStruct, UObject* Outer)
{
	ULevelTemplateContext* NewLTC = NewObject<ULevelTemplateContext>(Outer);
	NewLTC->LevelTemplate = InStruct.LevelTemplate;
	for (FPlayerLevelRecord TmpPlayerRecord : InStruct.PlayerRecords)
	{
		NewLTC->PlayerRecords.Add(TmpPlayerRecord.PlayerGuid, TmpPlayerRecord);
	}
	return NewLTC;
}