// Fill out your copyright notice in the Description page of Project Settings.


#include "TRPersistentDataComponent.h"
#include "..\Public\TRPersistentDataComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LevelTemplatesSave.h"

// Sets default values for this component's properties
UTRPersistentDataComponent::UTRPersistentDataComponent()
	: Super()
{
	SetIsReplicated(true);
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	//LevelForge = CreateDefaultSubobject<DefaultLevelForgeClass>(TEXT("LevelForge"));
	LevelRandStream.GenerateNewSeed();
}

void UTRPersistentDataComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTRPersistentDataComponent, LevelTemplatesPage);
	//DOREPLIFETIME(UTRPersistentDataComponent, LevelTemplatesRepTrigger);
}


// Called when the game starts
void UTRPersistentDataComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


ULevelForgeBase* UTRPersistentDataComponent::GetLevelForge()
{
	if (LevelForge == nullptr)
	{
		LevelForge = NewObject<ULevelForgeBase>(this, DefaultLevelForgeClass);
		if (LevelForge != nullptr) {
			//UE_LOG(LogTRGame, Log, TEXT("PersistentDataComponent - Created new LevelForge %s"), *DefaultLevelForgeClass->GetName());
		} else {
			UE_LOG(LogTRGame, Error, TEXT("PersistentDataComponent - Could not create LevelForge %s"), *DefaultLevelForgeClass->GetName());
		}
	}
	return LevelForge;
}


// Called every frame
void UTRPersistentDataComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UTRPersistentDataComponent::OnRep_LevelTemplatesPageLoaded()
{
	OnNewLevelTemplatesPage.Broadcast(LevelTemplatesPage);
	/*if (GetOwnerRole() == ROLE_Authority) {
		UE_LOG(LogTRGame, Log, TEXT("PersistentDataComponent - (Server) OnRep_LevelTemplatesPageLoaded"));
	}
	else {
		UE_LOG(LogTRGame, Log, TEXT("PersistentDataComponent - (Client) OnRep_LevelTemplatesPageLoaded"));
	}*/
}


void UTRPersistentDataComponent::ServerGenerateNewLevelTemplate_Implementation(const float Tier)
{
	bool bSuccessful = false;
	if (GetLevelForge())
	{
		FLevelTemplate LevelTemplate = FLevelTemplate();
		int32 NewSeed = static_cast<int32>(LevelRandStream.FRandRange(static_cast<float>(INT_MIN + 10), static_cast<float>(INT_MAX - 10)));
		UE_LOG(LogTRGame, Log, TEXT("PersistentDataComponent - generating new level template for seed: %d. (stream seed: %d)"), NewSeed, LevelRandStream.GetCurrentSeed());
		GetLevelForge()->GenerateNewLevelTemplate(NewSeed, Tier, LevelTemplate, bSuccessful);
		if (bSuccessful)
		{
			LevelTemplates.Add(LevelTemplate);
			LevelTemplatesPage.Add(LevelTemplate);
			//LevelTemplatesRepTrigger++;
			// Manually call rep_notify on server
			if (GetOwnerRole() == ROLE_Authority) { OnRep_LevelTemplatesPageLoaded(); }
		}
		else
		{
			UE_LOG(LogTRGame, Error, TEXT("PersistentDataComponent - Error generating new level template."));
		}
	}	
}

bool UTRPersistentDataComponent::ServerGenerateNewLevelTemplate_Validate(const float Tier)
{
	return true;
}


FString UTRPersistentDataComponent::GetLevelTemplatesSaveFilename_Implementation()
{
	return FString::Printf(TEXT("%s_LevelTemplates"), *SaveProfile);
}


void UTRPersistentDataComponent::ServerSaveLevelTemplatesData_Implementation()
{
	ServerSaveLevelTemplatesDataImpl();
}

bool UTRPersistentDataComponent::ServerSaveLevelTemplatesData_Validate()
{
	return true;
}

void UTRPersistentDataComponent::ServerSaveLevelTemplatesDataImpl_Implementation()
{
	//UE_LOG(LogTRGame, Log, TEXT("PersistentDataComponent - Save level templates starting"));
	ULevelTemplatesSave* SaveGame = Cast<ULevelTemplatesSave>(UGameplayStatics::CreateSaveGameObject(ULevelTemplatesSave::StaticClass()));
	SaveGame->LevelTemplates = LevelTemplates;
	FAsyncSaveGameToSlotDelegate Callback = FAsyncSaveGameToSlotDelegate::CreateUObject(this, &UTRPersistentDataComponent::OnLevelTemplatesSaveComplete);
	UGameplayStatics::AsyncSaveGameToSlot(SaveGame, GetLevelTemplatesSaveFilename(), 0, Callback);
}


void UTRPersistentDataComponent::ServerLoadLevelTemplatesData_Implementation()
{
	ServerLoadLevelTemplatesDataImpl();
}

bool UTRPersistentDataComponent::ServerLoadLevelTemplatesData_Validate()
{
	return true;
}

void UTRPersistentDataComponent::ServerLoadLevelTemplatesDataImpl_Implementation()
{
	//if (GetOwnerRole() == ROLE_Authority) {
	//	UE_LOG(LogTRGame, Log, TEXT("PersistentDataComponent - (Server) Load level templates already loaded: %s."), bLevelTemplatesLoaded ? TEXT("true") : TEXT("false"));
	//}
	//else {
	//	UE_LOG(LogTRGame, Log, TEXT("PersistentDataComponent - (Client) Load level templates already loaded: %s."), bLevelTemplatesLoaded ? TEXT("true") : TEXT("false"));
	//}
	bLevelTemplatesLoaded = false;
	if (UGameplayStatics::DoesSaveGameExist(GetLevelTemplatesSaveFilename(), 0))
	{
		ULevelTemplatesSave* SaveGame = Cast<ULevelTemplatesSave>(UGameplayStatics::LoadGameFromSlot(GetLevelTemplatesSaveFilename(), 0));
		if (SaveGame)
		{
			LevelTemplates = SaveGame->LevelTemplates;
			bLevelTemplatesLoaded = true;
			// TODO: implement paging of results. Currenty putting them all in here.
			LevelTemplatesPage = LevelTemplates;
			//LevelTemplatesRepTrigger++;
			// Manually call rep_notify on server
			if (GetOwnerRole() == ROLE_Authority) { OnRep_LevelTemplatesPageLoaded(); }
		}
	}
	//UE_LOG(LogTRGame, Log, TEXT("PersistentDataComponent - Load level templates: %s."), bLevelTemplatesLoaded ? TEXT("true") : TEXT("false"));
}


void UTRPersistentDataComponent::OnLevelTemplatesSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccessful)
{
	// Just calls the delegate for notifications
	OnLevelTemplatesSaved.Broadcast(bSuccessful);
	//if (GetOwnerRole() == ROLE_Authority) {
	//	UE_LOG(LogTRGame, Log, TEXT("PersistentDataComponent - (Server) Save level templates complete: %s %s"), *SlotName, bSuccessful ? TEXT("true") : TEXT("false"));
	//}
	//else {
	//	UE_LOG(LogTRGame, Log, TEXT("PersistentDataComponent - (Client) Save level templates complete: %s %s"), *SlotName, bSuccessful ? TEXT("true") : TEXT("false"));
	//}
}
