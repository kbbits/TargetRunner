// Fill out your copyright notice in the Description page of Project Settings.


#include "TRPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "TRGameInstance.h"


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


void ATRPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATRPlayerState, PlayerGuid);
	DOREPLIFETIME(ATRPlayerState, ProfileName);
	DOREPLIFETIME(ATRPlayerState, DisplayName);
}


void ATRPlayerState::BeginPlay()
{
	Super::BeginPlay();
	UTRGameInstance* GameInst = Cast<UTRGameInstance>(GetGameInstance());
	if (GameInst)
	{
		if (!GameInst->ClientLocalProfileName.IsNone()) { ProfileName = GameInst->ClientLocalProfileName; }
		if (GameInst->ClientLocalPlayerGuid.IsValid()) { PlayerGuid = GameInst->ClientLocalPlayerGuid; }
	}
	else {
		UE_LOG(LogTRGame, Error, TEXT("TRPlayerState - BeginPlay - Could not get game instance."));
	}
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
	UE_LOG(LogTRGame, Log, TEXT("TRPlayerState - UpdateFromPlayerSaveData - new player guid: %s."), *SaveData.PlayerGuid.ToString(EGuidFormats::Digits));
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