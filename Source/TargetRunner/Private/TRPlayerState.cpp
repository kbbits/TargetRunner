// Fill out your copyright notice in the Description page of Project Settings.


#include "TRPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "TRGameInstance.h"


ATRPlayerState::ATRPlayerState()
	:Super()
{
	EnergyAttribute = CreateDefaultSubobject<UActorAttributeComponent>(TEXT("EnergyAttribute"));
	if (EnergyAttribute)
	{
		AddOwnedComponent(EnergyAttribute);
		EnergyAttribute->SetIsReplicated(true); // Enable replication by default
		EnergyAttribute->AttributeData.Name = FName(TEXT("Energy"));
	}
	else { UE_LOG(LogTRGame, Error, TEXT("TRPlayerState constructor failed to create EnergyAttribute component")); }
	
	AnimusAttribute = CreateDefaultSubobject<UActorAttributeComponent>(TEXT("AnimusAttribute"));
	if (AnimusAttribute)
	{
		AddOwnedComponent(AnimusAttribute);
		AnimusAttribute->SetIsReplicated(true); // Enable replication by default
		AnimusAttribute->AttributeData.Name = FName(TEXT("Animus"));
	}
	else { UE_LOG(LogTRGame, Error, TEXT("TRPlayerState constructor failed to create AnimusAttribute component")); }
	
	HealthAttribute = CreateDefaultSubobject<UActorAttributeComponent>(TEXT("HealthAttribute"));
	if (HealthAttribute)
	{
		AddOwnedComponent(HealthAttribute);
		HealthAttribute->SetIsReplicated(true); // Enable replication by default
		HealthAttribute->AttributeData.Name = FName(TEXT("Health"));
	}
	else { UE_LOG(LogTRGame, Error, TEXT("TRPlayerState constructor failed to create HealthAttribute component")); }

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


void ATRPlayerState::CopyProperties(APlayerState* NewPlayerState)
{
	Super::CopyProperties(NewPlayerState);

	UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("PlayerState::CopyProperties to new player state: %s"), *NewPlayerState->GetName());
	ATRPlayerState* NewTRPlayerState = Cast<ATRPlayerState>(NewPlayerState);
	if (NewTRPlayerState)
	{
		UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("PlayerState::CopyProperties - current PS with guid %s overwriting new PS with guid %s"), *PlayerGuid.ToString(EGuidFormats::Digits), *NewTRPlayerState->PlayerGuid.ToString(EGuidFormats::Digits));
		NewTRPlayerState->PlayerGuid = PlayerGuid;
		NewTRPlayerState->ProfileName = ProfileName;
		NewTRPlayerState->DisplayName = DisplayName;
		NewTRPlayerState->ExperienceLevel = ExperienceLevel < 1 ? 1 : ExperienceLevel;
		NewTRPlayerState->LevelUpGoodsProgress = LevelUpGoodsProgress;
		NewTRPlayerState->MaxTierCompleted = MaxTierCompleted;
		NewTRPlayerState->TotalRunsPlayed = TotalRunsPlayed;
		NewTRPlayerState->TotalPlaytimeInRuns = TotalPlaytimeInRuns;
		if (EnergyAttribute && NewTRPlayerState->EnergyAttribute) {
			NewTRPlayerState->EnergyAttribute->CopyPropertiesFromOther(EnergyAttribute);
		}
		if (AnimusAttribute && NewTRPlayerState->AnimusAttribute) {
			NewTRPlayerState->AnimusAttribute->CopyPropertiesFromOther(AnimusAttribute);
		}
		if (HealthAttribute && NewTRPlayerState->HealthAttribute) {
			NewTRPlayerState->HealthAttribute->CopyPropertiesFromOther(HealthAttribute);
		}
	}
	else {
		UE_LOG(LogTRGame, Error, TEXT("PlayerState::CopyProperties - New player state is null."));
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
		// RemoveModifiers will filter for modifiers relevant to the attribute
		Attr->RemoveModifiers(ModifiersToRemove);
	}
}


void ATRPlayerState::GetPlayerSaveData_Implementation(FPlayerSaveData& SaveData)
{
	UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("PlayerState::GetPlayerSaveData - Called for player state guid %s"), *PlayerGuid.ToString(EGuidFormats::Digits));
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
		Attr->FillAttributeDataArray(SaveData.AttributeData.Attributes);
	}
	// Goods quantities properties
	SaveData.AttributeData.GoodsQuantitiesAttributes.Add(LevelUpGoodsProgress);
}


void ATRPlayerState::UpdateFromPlayerSaveData_Implementation(const FPlayerSaveData& SaveData)
{
	UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("TRPlayerState - UpdateFromPlayerSaveData - Current guid %s. Save data info %s : %s."), *PlayerGuid.ToString(EGuidFormats::Digits), *SaveData.ProfileName.ToString(), *SaveData.PlayerGuid.ToString(EGuidFormats::Digits));
	PlayerGuid = SaveData.PlayerGuid;		// replicated
	ProfileName = SaveData.ProfileName;		// replicated
	DisplayName = SaveData.DisplayName;		// replicated
	MaxTierCompleted = SaveData.MaxTierCompleted;
	TotalRunsPlayed = SaveData.TotalRunsPlayed;
	TotalPlaytimeInRuns = SaveData.TotalPlaytimeInRuns;
	ExperienceLevel = SaveData.ExperienceLevel < 1 ? 1 : SaveData.ExperienceLevel;	// replicated
	// Do the attribute components
	TArray<UActorAttributeComponent*> AttributeComps;
	GetComponents<UActorAttributeComponent>(AttributeComps);
	for (UActorAttributeComponent* Attr : AttributeComps)
	{
		// This call does nothing if the data is not for that attribute.
		Attr->UpdateFromAttributeDataArray(SaveData.AttributeData.Attributes);
	}
	// Goods quantities attributes
	// LevelUpProgress
	const FNamedGoodsQuantitySet* TmpGoodsSet = FindInNamedArray<FNamedGoodsQuantitySet>(SaveData.AttributeData.GoodsQuantitiesAttributes, LevelUpGoodsProgress.Name);
	if (TmpGoodsSet) {
		LevelUpGoodsProgress.GoodsQuantitySet = TmpGoodsSet->GoodsQuantitySet;  // replicated
	}
}