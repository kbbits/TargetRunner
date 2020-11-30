// Fill out your copyright notice in the Description page of Project Settings.


#include "TRPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "TRGameInstance.h"


ATRPlayerState::ATRPlayerState()
	:Super()
{
	EnergyAttribute = CreateDefaultSubobject<UActorAttributeComponent>(TEXT("EnergyAttribute"));
	AddOwnedComponent(EnergyAttribute);
	EnergyAttribute->SetIsReplicated(true); // Enable replication by default
	EnergyAttribute->AttributeData.Name = FName(TEXT("Energy"));
	
	AnimusAttribute = CreateDefaultSubobject<UActorAttributeComponent>(TEXT("AnimusAttribute"));
	AddOwnedComponent(AnimusAttribute);
	AnimusAttribute->SetIsReplicated(true); // Enable replication by default
	AnimusAttribute->AttributeData.Name = FName(TEXT("Animus"));
	
	HealthAttribute = CreateDefaultSubobject<UActorAttributeComponent>(TEXT("HealthAttribute"));
	AddOwnedComponent(HealthAttribute);
	HealthAttribute->SetIsReplicated(true); // Enable replication by default
	HealthAttribute->AttributeData.Name = FName(TEXT("Health"));

	if (ExperienceLevel <= 0) { ExperienceLevel = 1; }
	LevelUpGoodsProgress.Name = FName(TEXT("LevelUpGoodsProgress"));
}


void ATRPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATRPlayerState, PlayerGuid);
	DOREPLIFETIME(ATRPlayerState, ProfileName);
	DOREPLIFETIME(ATRPlayerState, DisplayName);
	DOREPLIFETIME(ATRPlayerState, ExperienceLevel);
	DOREPLIFETIME(ATRPlayerState, LevelUpGoodsProgress);
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

void ATRPlayerState::ApplyAttributeModifiers_Implementation(const TArray<FAttributeModifier>& NewModifiers)
{
	TArray<UActorAttributeComponent*> AttributeComps;
	GetComponents<UActorAttributeComponent>(AttributeComps);
	for (UActorAttributeComponent* Attr : AttributeComps)
	{
		// AddModifiers will filter for modifiers relevant to the attribute
		Attr->AddModifiers(NewModifiers);
	}
}


void ATRPlayerState::RemoveAttributeModifiers_Implementation(const TArray<FAttributeModifier>& ModifiersToRemove)
{
	TArray<UActorAttributeComponent*> AttributeComps;
	GetComponents<UActorAttributeComponent>(AttributeComps);
	for (UActorAttributeComponent* Attr : AttributeComps)
	{
		// RemoveModifiers filters for modifiers relevant to the attribute
		Attr->RemoveModifiers(ModifiersToRemove);
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
	SaveData.ExperienceLevel = ExperienceLevel;
	// Do the attribute components
	TArray<UActorAttributeComponent*> AttributeComps;
	GetComponents<UActorAttributeComponent>(AttributeComps);
	for (UActorAttributeComponent* Attr : AttributeComps)
	{
		Attr->FillAttributeDataMap(SaveData.AttributeData.Attributes);
	}
	// Goods quantities properties
	SaveData.AttributeData.GoodsQuantitiesAttributes.Add(LevelUpGoodsProgress.Name, LevelUpGoodsProgress.GoodsQuantitySet);
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
	ExperienceLevel = SaveData.ExperienceLevel;
	// Do the attribute components
	TArray<UActorAttributeComponent*> AttributeComps;
	GetComponents<UActorAttributeComponent>(AttributeComps);
	for (UActorAttributeComponent* Attr : AttributeComps)
	{
		Attr->UpdateFromAttributeDataMap(SaveData.AttributeData.Attributes);
	}
	// Goods quantities attributes
	if (SaveData.AttributeData.GoodsQuantitiesAttributes.Contains(LevelUpGoodsProgress.Name))
	{
		LevelUpGoodsProgress.GoodsQuantitySet = SaveData.AttributeData.GoodsQuantitiesAttributes[LevelUpGoodsProgress.Name];
	}
}