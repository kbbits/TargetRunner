// Fill out your copyright notice in the Description page of Project Settings.


#include "TRPlayerState.h"


ATRPlayerState::ATRPlayerState()
	:Super()
{
	EnergyAttribute = CreateDefaultSubobject<UActorAttributeComponent>(TEXT("EnergyAttribute"));
	AddOwnedComponent(EnergyAttribute);
	EnergyAttribute->AttributeData.Name = FName(TEXT("Energy"));
	
	AnimusAttribute = CreateDefaultSubobject<UActorAttributeComponent>(TEXT("AnimusAttribute"));
	AddOwnedComponent(AnimusAttribute);
	AnimusAttribute->AttributeData.Name = FName(TEXT("Animus"));
	
	HealthAttribute = CreateDefaultSubobject<UActorAttributeComponent>(TEXT("HealthAttribute"));
	AddOwnedComponent(HealthAttribute);
	HealthAttribute->AttributeData.Name = FName(TEXT("Health"));
}

void ATRPlayerState::GetPlayerSaveData_Implementation(FPlayerSaveData& SaveData)
{
	SaveData.PlayerGuid = PlayerGuid;
	SaveData.ProfileName = ProfileName;
	SaveData.DisplayName = DisplayName;
	SaveData.MaxTierCompleted = MaxTierCompleted;
	SaveData.TotalRunsPlayed = TotalRunsPlayed;
	SaveData.TotalPlaytimeInRuns = TotalPlaytimeInRuns;
	// Do the attribute components
	TArray<UActorAttributeComponent*> AttributeComps;
	GetComponents<UActorAttributeComponent>(AttributeComps);
	for (UActorAttributeComponent* Attr : AttributeComps)
	{
		Attr->FillAttributeDataMap(SaveData.AttributeData.Attributes);
	}
}

void ATRPlayerState::UpdateFromPlayerSaveData_Implementation(const FPlayerSaveData& SaveData)
{
	PlayerGuid = SaveData.PlayerGuid;
	ProfileName = SaveData.ProfileName;
	DisplayName = SaveData.DisplayName;
	MaxTierCompleted = SaveData.MaxTierCompleted;
	TotalRunsPlayed = SaveData.TotalRunsPlayed;
	TotalPlaytimeInRuns = SaveData.TotalPlaytimeInRuns;
	// Do the attribute components
	TArray<UActorAttributeComponent*> AttributeComps;
	GetComponents<UActorAttributeComponent>(AttributeComps);
	for (UActorAttributeComponent* Attr : AttributeComps)
	{
		Attr->UpdateFromAttributeDataMap(SaveData.AttributeData.Attributes);
	}
}