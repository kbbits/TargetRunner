// Fill out your copyright notice in the Description page of Project Settings.


#include "TRPersistentDataComponent.h"
#include "..\Public\TRPersistentDataComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TRGameInstance.h"

// Sets default values for this component's properties
UTRPersistentDataComponent::UTRPersistentDataComponent()
	: Super()
{
	SetIsReplicated(true);
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

void UTRPersistentDataComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTRPersistentDataComponent, LevelTemplatesPage); // owner only
	DOREPLIFETIME(UTRPersistentDataComponent, LevelTemplatesRepTrigger);
}


// Called when the game starts
void UTRPersistentDataComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void UTRPersistentDataComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UTRPersistentDataComponent::OnRep_LevelTemplatesPageLoaded()
{
	if (GetOwnerRole() == ROLE_Authority) {
		UE_LOG(LogTRGame, Log, TEXT("UTRPersistentDataComponent - (Server) OnRep level templates loaded %d"), LevelTemplatesPage.Num());
	}
	else {
		UE_LOG(LogTRGame, Log, TEXT("UTRPersistentDataComponent - (Client) OnRep level templates loaded %d"), LevelTemplatesPage.Num());
	}
	OnNewLevelTemplatesPage.Broadcast(LevelTemplatesPage);
}


void UTRPersistentDataComponent::ServerGenerateNewLevelTemplate_Implementation(const float Tier)
{
	UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
	if (GameInst)
	{
		ULevelTemplateContext* NewTemplate = GameInst->GenerateNewLevelTemplate(Tier);
		if (NewTemplate && NewTemplate->LevelTemplate.IsValid()) {
			LevelTemplatesPage.Add(NewTemplate->ToStruct());
			LevelTemplatesRepTrigger++;
			// Manually call rep_notify on server
			if (GetOwnerRole() == ROLE_Authority) { OnRep_LevelTemplatesPageLoaded(); }
		}
	}
	else
	{
		UE_LOG(LogTRGame, Error, TEXT("PersistentDataComponent - ServerGenerateNewLevelTemplate could not get game instance."));
	}
}

bool UTRPersistentDataComponent::ServerGenerateNewLevelTemplate_Validate(const float Tier)
{
	return true;
}

void UTRPersistentDataComponent::ServerSaveLevelTemplatesData_Implementation()
{
	UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
	if (GameInst)
	{
		GameInst->SaveLevelTemplatesData();
	}
	else
	{
		UE_LOG(LogTRGame, Error, TEXT("PersistentDataComponent - ServerSaveLevelTemplatesData could not get game instance."));
	}
}


bool UTRPersistentDataComponent::ServerSaveLevelTemplatesData_Validate()
{
	return true;
}


void UTRPersistentDataComponent::ServerLoadLevelTemplatesData_Implementation()
{
	UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
	if (GameInst)
	{
		GameInst->LoadLevelTemplatesData();
		if (GameInst->bLevelTemplatesLoaded)
		{
			// TODO: implement paging of results. Currenty putting them all in here.
			TArray<ULevelTemplateContext*> TmpLTCArray;
			GameInst->LevelTemplatesMap.GenerateValueArray(TmpLTCArray);
			LevelTemplatesPage.Empty();
			LevelTemplatesPage.Append(ULevelTemplateContext::ToStructArray(TmpLTCArray));
			LevelTemplatesRepTrigger++;
			UE_LOG(LogTRGame, Log, TEXT("UTRPersistentDataComponent - Load level templates loaded %d"), LevelTemplatesPage.Num());
			// Manually call rep_notify on server
			if (GetOwnerRole() == ROLE_Authority) { OnRep_LevelTemplatesPageLoaded(); }
		}
		else
		{
			UE_LOG(LogTRGame, Error, TEXT("PersistentDataComponent - ServerLoadLevelTemplatesData failed loading level template data."));
		}
	}
	else
	{
		UE_LOG(LogTRGame, Error, TEXT("PersistentDataComponent - ServerLoadLevelTemplatesData could not get game instance."));
	}
}

bool UTRPersistentDataComponent::ServerLoadLevelTemplatesData_Validate()
{
	return true;
}


void UTRPersistentDataComponent::ServerSetLevelTemplateForPlay_Implementation(const FLevelTemplate& LevelTemplate)
{
	UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
	if (GameInst) {
		GameInst->SetSelectedLevelTemplate(LevelTemplate);
	}
	else {
		UE_LOG(LogTRGame, Error, TEXT("ServerSetLevelTemplateForPlay - Could not get game instance."))
	}
}

bool UTRPersistentDataComponent::ServerSetLevelTemplateForPlay_Validate(const FLevelTemplate& LevelTemplate)
{
	return true;
}