// Fill out your copyright notice in the Description page of Project Settings.


#include "TRPersistentDataComponent.h"
#include "..\Public\TRPersistentDataComponent.h"
#include "TRGameModeLobby.h"
#include "TRPlayerControllerBase.h"
#include "TRPlayerState.h"
#include "TRGameModeLobby.h"
#include "PlayerSave.h"
#include "LevelTemplateContextStruct.h"
#include "Kismet/GameplayStatics.h"
#include "Paths.h"
#include "PlatformFile.h"
#include "PlatformFilemanager.h"
#include "PlatformFeatures.h"
#include "GameFramework/SaveGame.h"
#include "TRGameInstance.h"

const FString UTRPersistentDataComponent::LocalPlayerFilenameSuffix = FString(TEXT("_player"));
const FString UTRPersistentDataComponent::RemotePlayerFilenameSuffix = FString(TEXT("_rmtplr"));

// Sets default values for this component's properties
UTRPersistentDataComponent::UTRPersistentDataComponent()
	: Super()
{
	//SetIsReplicated(true);
	SetIsReplicatedByDefault(true);
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

void UTRPersistentDataComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTRPersistentDataComponent, LevelTemplatesPage); // owner only
	DOREPLIFETIME(UTRPersistentDataComponent, LevelTemplatesRepTrigger);
	//DOREPLIFETIME(UTRPersistentDataComponent, PlayerToolData); // owner only
	//DOREPLIFETIME(UTRPersistentDataComponent, PlayerToolDataRepTrigger);	
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
	if (bEnableClassDebug)
	{
		if (GetOwnerRole() == ROLE_Authority) {
			UE_LOG(LogTRGame, Log, TEXT("UTRPersistentDataComponent - (Server) OnRep level templates loaded %d"), LevelTemplatesPage.Num());
		}
		else {
			UE_LOG(LogTRGame, Log, TEXT("UTRPersistentDataComponent - (Client) OnRep level templates loaded %d"), LevelTemplatesPage.Num());
		}
	}
	OnNewLevelTemplatesPage.Broadcast(LevelTemplatesPage);
}


void UTRPersistentDataComponent::ServerGenerateNewLevelTemplate_Implementation(const float Tier, const int32 DifficultyLevel, const bool bUnlockForPlayer)
{
	UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("UTRPersistentDataComponent - ServerGenerateNewLevelTemplate"));
	UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
	if (GameInst)
	{
		ULevelTemplateContext* NewTemplate = GameInst->GenerateNewLevelTemplate(Tier, DifficultyLevel);
		if (NewTemplate && NewTemplate->LevelTemplate.IsValid()) 
		{
			UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("UTRPersistentDataComponent - ServerGenerateNewLevelTemplate generated %s"), *NewTemplate->LevelTemplate.LevelId.ToString());
			if (bUnlockForPlayer)
			{
				// The unlock will do replication
				ServerUnlockLevelTemplateForPlayer(NewTemplate->LevelTemplate.LevelId);
			}
			else
			{	
				// Call to replicate to client
				ServerLoadLevelTemplatesData();
				//LevelTemplatesPage.Add(NewTemplate->ToStruct());
				//LevelTemplatesRepTrigger++; 
				// Manually call rep_notify on server
				//if (GetOwnerRole() == ROLE_Authority) { OnRep_LevelTemplatesPageLoaded(); }
			}
		}
	}
	else
	{
		UE_LOG(LogTRGame, Error, TEXT("PersistentDataComponent - ServerGenerateNewLevelTemplate could not get game instance."));
	}
}

bool UTRPersistentDataComponent::ServerGenerateNewLevelTemplate_Validate(const float Tier, const int32 DifficultyLevel, const bool bUnlockForPlayer)
{
	return true;
}

void UTRPersistentDataComponent::ServerSaveLevelTemplatesData_Implementation()
{
	UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
	if (GameInst) {
		GameInst->SaveLevelTemplatesData();
	}
	else {
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
			UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("UTRPersistentDataComponent - Load level templates loaded %d"), LevelTemplatesPage.Num());
			// Manually call rep_notify on server
			if (GetOwnerRole() == ROLE_Authority) { 
				OnRep_LevelTemplatesPageLoaded(); 
			}
		}
		else {
			UE_LOG(LogTRGame, Error, TEXT("PersistentDataComponent - ServerLoadLevelTemplatesData failed loading level template data."));
		}
	}
	else {
		UE_LOG(LogTRGame, Error, TEXT("PersistentDataComponent - ServerLoadLevelTemplatesData could not get game instance."));
	}
}

bool UTRPersistentDataComponent::ServerLoadLevelTemplatesData_Validate()
{
	return true;
}


void UTRPersistentDataComponent::ServerUnlockLevelTemplateForPlayer_Implementation(const FName LevelId)
{
	UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
	if (GameInst)
	{
		ATRPlayerControllerBase* TRPlayerController = Cast<ATRPlayerControllerBase>(GetOwner());
		if (TRPlayerController)
		{
			ATRPlayerState* TRPlayerState = Cast<ATRPlayerState>(TRPlayerController->PlayerState);
			if (TRPlayerState) {
				ULevelTemplateContext* UnlockedTemplate = GameInst->UnlockLevelTemplateForPlayer(LevelId, TRPlayerState->PlayerGuid);
				if (UnlockedTemplate)
				{
					if (!TRPlayerController->IsLocalController()) {
						ClientUnlockLevelTemplateForPlayer(UnlockedTemplate->LevelTemplate, TRPlayerState->PlayerGuid);
					}
					TArray<ULevelTemplateContext*> TmpLTCArray;
					GameInst->LevelTemplatesMap.GenerateValueArray(TmpLTCArray);
					LevelTemplatesPage.Empty();
					LevelTemplatesPage.Append(ULevelTemplateContext::ToStructArray(TmpLTCArray));
					LevelTemplatesRepTrigger++;
					if (bEnableClassDebug)
					{
						UE_LOG(LogTRGame, Log, TEXT("UTRPersistentDataComponent - level template %s unlocked for player %s"), *LevelId.ToString(), *TRPlayerState->PlayerGuid.ToString(EGuidFormats::Digits));
						for (FLevelTemplateContextStruct TmpLTCS : LevelTemplatesPage) {
							UE_LOG(LogTRGame, Log, TEXT("UTRPersistentDataComponent - template now has %s player records: %d"), *TmpLTCS.LevelTemplate.LevelId.ToString(), TmpLTCS.PlayerRecords.Num());
						}
					}
					// Manually call rep_notify on server
					if (GetOwnerRole() == ROLE_Authority) { 
						OnRep_LevelTemplatesPageLoaded(); 
					}
				}
				else {
					UE_LOG(LogTRGame, Error, TEXT("UTRPersistentDataComponent - Could not unlock level template %s for player %s"), *LevelId.ToString(), *TRPlayerState->PlayerGuid.ToString(EGuidFormats::Digits));
				}
			}
		}
	}
}

bool UTRPersistentDataComponent::ServerUnlockLevelTemplateForPlayer_Validate(const FName LevelId)
{
	return true;
}


void UTRPersistentDataComponent::ClientUnlockLevelTemplateForPlayer_Implementation(const FLevelTemplate& LevelTemplate, const FGuid PlayerGuid)
{
	UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
	if (GameInst)
	{
		GameInst->LoadLevelTemplatesData();
		ATRPlayerControllerBase* TRPlayerController = Cast<ATRPlayerControllerBase>(GetOwner());
		if (TRPlayerController)
		{
			ATRPlayerState* TRPlayerState = Cast<ATRPlayerState>(TRPlayerController->PlayerState);
			if (TRPlayerState) 
			{	
				if (TRPlayerState->PlayerGuid == PlayerGuid)
				{
					UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("UTRPersistentDataComponent::ClientUnlockLevelTemplateForPlayer - level template %s unlocked for player %s"), *LevelTemplate.LevelId.ToString(), *TRPlayerState->PlayerGuid.ToString(EGuidFormats::Digits));
					GameInst->AddLevelTemplate(LevelTemplate);
					ULevelTemplateContext* UnlockedTemplate = GameInst->UnlockLevelTemplateForPlayer(LevelTemplate.LevelId, TRPlayerState->PlayerGuid);
				}
				else {
					UE_LOG(LogTRGame, Error, TEXT("PersistentDataComponent:ClientUnlockLevelTemplateForPlayer - incoming guid %s does not match player guid %s"), *PlayerGuid.ToString(EGuidFormats::Digits), *TRPlayerState->PlayerGuid.ToString(EGuidFormats::Digits));
				}
			}
		}
	}
}


bool UTRPersistentDataComponent::ClientUnlockLevelTemplateForPlayer_Validate(const FLevelTemplate& LevelTemplate, const FGuid PlayerGuid)
{
	return true;
}


void UTRPersistentDataComponent::ServerSetLevelTemplateForPlay_Implementation(const FName LevelId)
{
	ATRGameModeLobby* GameMode = Cast<ATRGameModeLobby>(UGameplayStatics::GetGameMode(GetOwner()));
	if (GameMode) {
		GameMode->SetSelectedLevelTemplate(LevelId);
	}
	//UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
	//if (GameInst) {
	//	GameInst->SetSelectedLevelTemplate(LevelTemplate);		
	//}
	//else {
	//	UE_LOG(LogTRGame, Error, TEXT("ServerSetLevelTemplateForPlay - Could not get game instance."));
	//}
}

bool UTRPersistentDataComponent::ServerSetLevelTemplateForPlay_Validate(const FName LevelId)
{
	return true;
}


void UTRPersistentDataComponent::ClientSetSelectedLevelTemplate_Implementation(const FLevelTemplate& LevelTemplate)
{
	UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
	if (GameInst) {
		GameInst->SetSelectedLevelTemplate(LevelTemplate);
	}
	else {
		UE_LOG(LogTRGame, Error, TEXT("ClientSetLevelTemplateForPlay - Could not get game instance."));
	}
}

bool UTRPersistentDataComponent::ClientSetSelectedLevelTemplate_Validate(const FLevelTemplate& LevelTemplate)
{
	return true;
}


FString UTRPersistentDataComponent::GetLocalPlayerSaveFilename(const FGuid& PlayerGuid)
{
	if (PlayerGuid.IsValid()) {
		return PlayerGuid.ToString(EGuidFormats::Digits) + LocalPlayerFilenameSuffix;
	}
	return FString();
}


FString UTRPersistentDataComponent::GetRemotePlayerSaveFilename(const FGuid& PlayerGuid)
{
	if (PlayerGuid.IsValid()) {
		return PlayerGuid.ToString(EGuidFormats::Digits) + RemotePlayerFilenameSuffix;
	}
	return FString();
}


FString UTRPersistentDataComponent::GetPlayerSaveFilenameForGuid(const FGuid PlayerProfileGuid, const bool bRemote)
{
	if (bRemote) {
		return GetRemotePlayerSaveFilename(PlayerProfileGuid);		
	}
	else {
		return GetLocalPlayerSaveFilename(PlayerProfileGuid);
	}
}


FString UTRPersistentDataComponent::GetPlayerSaveFilename()
{ 
	ATRPlayerControllerBase* TRPlayerController = Cast<ATRPlayerControllerBase>(GetOwner());
	if (TRPlayerController)
	{
		ATRPlayerState* TRPlayerState = Cast<ATRPlayerState>(TRPlayerController->PlayerState);
		if (TRPlayerState) 
		{
			if (TRPlayerState->PlayerGuid.IsValid()) {
				return GetPlayerSaveFilenameForGuid(TRPlayerState->PlayerGuid, !TRPlayerController->IsLocalController());
			}
			UE_LOG(LogTRGame, Error, TEXT("GetPlayerSaveFilename - player guid is not valid."));
		}
	}
	UE_LOG(LogTRGame, Error, TEXT("GetPlayerSaveFilename - Could not determine save filename."));
	return FString();
}


TArray<FString> UTRPersistentDataComponent::GetAllSaveProfileFilenames()
{
	bool bEnableDebug = false;

	//////////////////////////////////////////////////////////////////////////////
	class FFindSavesVisitor : public IPlatformFile::FDirectoryVisitor
	{
	public:
		FFindSavesVisitor() {}
		
		bool bEnableDebug = false;

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
		{
			if (!bIsDirectory)
			{
				if (FilenameOrDirectory) {
					UE_CLOG(bEnableDebug, LogTRGame, Log, TEXT("GetAllSaveProfileNames - potential file: %s."), FilenameOrDirectory);
				}
				else
				{
					UE_LOG(LogTRGame, Error, TEXT("TRPersistentDataComponent GetAllSaveProfileFilenames encountered null FilenameOrDirectory."));
					return false;
				}
				FString FullFilePath(FilenameOrDirectory);
				if (FPaths::GetExtension(FullFilePath) == TEXT("sav"))
				{
					FString CleanFilename = FPaths::GetBaseFilename(FullFilePath);
					if (CleanFilename.EndsWith(*UTRPersistentDataComponent::LocalPlayerFilenameSuffix))
					{
						SavesFound.Add(CleanFilename);
						UE_CLOG(bEnableDebug, LogTRGame, Log, TEXT("GetAllSaveProfileNames - found file: %s."), *CleanFilename);
					}
				}
			}
			return true;
		}
		TArray<FString> SavesFound;
	};
	//////////////////////////////////////////////////////////////////////////////

	const FString SavesFolder = FPaths::ProjectSavedDir() + TEXT("SaveGames");
	UE_CLOG(bEnableDebug, LogTRGame, Log, TEXT("GetAllSaveProfileNames - checking dir: %s."), *SavesFolder)
	if (!SavesFolder.IsEmpty())
	{
		FFindSavesVisitor Visitor;
		Visitor.bEnableDebug = bEnableDebug;
		FPlatformFileManager::Get().GetPlatformFile().IterateDirectory(*SavesFolder, Visitor);
		return Visitor.SavesFound;
	}
	return TArray<FString>();
}

TArray<FPlayerSaveData> UTRPersistentDataComponent::GetAllSaveProfileData()
{
	TArray<FPlayerSaveData> AllFoundData;
	TArray<FString> AllFilenames = GetAllSaveProfileFilenames();
	for (FString TmpFilename : AllFilenames)
	{
		if (UGameplayStatics::DoesSaveGameExist(TmpFilename, 0))
		{
			UPlayerSave* SaveGame = Cast<UPlayerSave>(UGameplayStatics::LoadGameFromSlot(TmpFilename, 0));
			if (SaveGame) {
				AllFoundData.Add(SaveGame->PlayerSaveData);
			}
		}
	}
	return AllFoundData;
}


bool UTRPersistentDataComponent::ProfileExistsForRemotePlayer(FGuid PlayerGuid)
{
	if (!PlayerGuid.IsValid())
	{
		UE_LOG(LogTRGame, Warning, TEXT("PersistentDataComponent ProfileExistsForRemotePlayer invalid player guid"));
		return false;
	}
	return UGameplayStatics::DoesSaveGameExist(GetRemotePlayerSaveFilename(PlayerGuid), 0);
}


void UTRPersistentDataComponent::SavePlayerDataInternal()
{
	ATRPlayerControllerBase* TRPlayerController = Cast<ATRPlayerControllerBase>(GetOwner());
	if (TRPlayerController == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("SavePlayerDataInternal - Could not get player controller."));
		return;
	}
	ATRPlayerState* TRPlayerState = Cast<ATRPlayerState>(TRPlayerController->PlayerState);
	if (TRPlayerState == nullptr) 
	{
		UE_LOG(LogTRGame, Error, TEXT("SavePlayerDataInternal - Could not get player state."));
		return;
	}
		
	// Create a new save each time
	UPlayerSave* SaveGame = Cast<UPlayerSave>(UGameplayStatics::CreateSaveGameObject(UPlayerSave::StaticClass()));
	// Get the data from our owning controller.
	TRPlayerController->GetPlayerSaveData(SaveGame->PlayerSaveData);
	//UE_LOG(LogTRGame, Log, TEXT("ServerSavePlayerData - Saving player data. Guid: %s"), *SaveGame->PlayerSaveData.PlayerGuid.ToString(EGuidFormats::Digits));
	// Begin debug inventory logging
	/*if (SaveGame->PlayerSaveData.GoodsInventory.Num() == 0) {
		UE_LOG(LogTRGame, Log, TEXT("    GoodsInventory is empty."));
	}
	else
	{
		UE_LOG(LogTRGame, Log, TEXT("    Goods Inventory: "));
		for (FGoodsQuantity InvElem : SaveGame->PlayerSaveData.GoodsInventory) {
			UE_LOG(LogTRGame, Log, TEXT("     %s: %f"), *InvElem.Name.ToString(), InvElem.Quantity);
		}
	}*/
	// End debug inventory logging 
	if (UGameplayStatics::SaveGameToSlot(SaveGame, GetPlayerSaveFilename(), 0)) {
		UE_LOG(LogTRGame, Log, TEXT("SavePlayerDataInternal - Player data for Guid %s saved to: %s"), *SaveGame->PlayerSaveData.PlayerGuid.ToString(EGuidFormats::Digits), *GetPlayerSaveFilename());
	}
	else {
		UE_LOG(LogTRGame, Error, TEXT("PersistentDataComponent SavePlayerDataInternal : Error saving player data: %s"), *GetPlayerSaveFilename());
	} 
}


void UTRPersistentDataComponent::ServerSavePlayerData_Implementation()
{
	SavePlayerDataInternal();
}

bool UTRPersistentDataComponent::ServerSavePlayerData_Validate()
{
	return true;
}


void UTRPersistentDataComponent::ServerLoadPlayerData_Implementation()
{
	ATRPlayerControllerBase* TRPlayerController = Cast<ATRPlayerControllerBase>(GetOwner());
	if (TRPlayerController == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("TRPersistentDataComponent::ServerLoadPlayerData could not get player controller."));
		OnPlayerDataLoaded.Broadcast(); // notify anyway
		return;
	}
	ATRPlayerState* TRPlayerState = Cast<ATRPlayerState>(TRPlayerController->PlayerState);
	if (TRPlayerState == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("TRPersistentDataComponent::ServerLoadPlayerData could not get player state."));
		OnPlayerDataLoaded.Broadcast(); // notify anyway
		return;
	}
	if (TRPlayerState->PlayerGuid.IsValid()) {
		ServerLoadPlayerDataByGuid(TRPlayerState->PlayerGuid);
	}
	else {
		UE_LOG(LogTRGame, Error, TEXT("TRPersistentDataComponent::ServerLoadPlayerData called when PlayerState.PlayerGuid is not valid."));
	}
}


bool UTRPersistentDataComponent::ServerLoadPlayerData_Validate()
{
	return true;
}


void UTRPersistentDataComponent::ServerLoadPlayerDataByGuid_Implementation(const FGuid ForcePlayerGuid)
{
	ATRPlayerControllerBase* TRPlayerController = Cast<ATRPlayerControllerBase>(GetOwner());
	if (TRPlayerController == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("ServerLoadPlayerData - Could not get player controller."));
		OnPlayerDataLoaded.Broadcast(); // notify anyway
		return;
	}
	ATRPlayerState* TRPlayerState = Cast<ATRPlayerState>(TRPlayerController->PlayerState);
	if (TRPlayerState == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("ServerLoadPlayerData - Could not get player state."));
		OnPlayerDataLoaded.Broadcast(); // notify anyway
		return;
	}
	if (ForcePlayerGuid.IsValid())
	{
		if (TRPlayerState->PlayerGuid.IsValid() && ForcePlayerGuid != TRPlayerState->PlayerGuid) {
			UE_LOG(LogTRGame, Error, TEXT("ServerLoadPlayerDataByGuid - Incoming GUID %s does not match existing GUID %s"), *ForcePlayerGuid.ToString(EGuidFormats::Digits), *TRPlayerState->PlayerGuid.ToString(EGuidFormats::Digits));
		}
		TRPlayerState->PlayerGuid = ForcePlayerGuid;
	}
	FString PlayerSaveFilename = GetPlayerSaveFilename();
	if (PlayerSaveFilename.IsEmpty())
	{
		UE_LOG(LogTRGame, Error, TEXT("ServerLoadPlayerData - Save filename invalid - no player guid set."));
		OnPlayerDataLoaded.Broadcast(); // notify anyway
		return;
	}
	if (UGameplayStatics::DoesSaveGameExist(PlayerSaveFilename, 0))
	{
		UPlayerSave* SaveGame = Cast<UPlayerSave>(UGameplayStatics::LoadGameFromSlot(PlayerSaveFilename, 0));
		if (SaveGame)
		{
			if (ForcePlayerGuid.IsValid() && SaveGame->PlayerSaveData.PlayerGuid.IsValid() && SaveGame->PlayerSaveData.PlayerGuid != ForcePlayerGuid) 
			{
				UE_LOG(LogTRGame, Error, TEXT("Player guid in file: %s does not match player guid %s."), *SaveGame->PlayerSaveData.PlayerGuid.ToString(EGuidFormats::Digits), *ForcePlayerGuid.ToString(EGuidFormats::Digits));
			}
			else
			{
				//UE_LOG(LogTRGame, Log, TEXT("PersistentDataComponent - Loading player data. Guid: %s"), *SaveGame->PlayerSaveData.PlayerGuid.ToString(EGuidFormats::Digits));
				// Begin debug inventory logging
				/* if (SaveGame->PlayerSaveData.GoodsInventory.Num() == 0)
				{
					UE_LOG(LogTRGame, Log, TEXT("    GoodsInventory is empty."));
				}
				else
				{
					UE_LOG(LogTRGame, Log, TEXT("    Goods Inventory:"));
					for (FGoodsQuantity InvElem : SaveGame->PlayerSaveData.GoodsInventory)
					{
						UE_LOG(LogTRGame, Log, TEXT("    %s: %f"), *InvElem.Name.ToString(), InvElem.Quantity);
					}
				} */
				// End debug inventory logging
				// Update data here on server
				TRPlayerController->UpdateFromPlayerSaveData(SaveGame->PlayerSaveData);
				if (TRPlayerController->IsLocalController()) 
				{	
					// Update game instance with host's local info
					UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
					if (GameInst)
					{
						GameInst->ClientLocalProfileName = TRPlayerState->ProfileName;
						GameInst->ClientLocalPlayerGuid = TRPlayerState->PlayerGuid;
					}
				}
				else
				{
					// Call client to update data
					ClientEchoLoadPlayerData(SaveGame->PlayerSaveData);
				}
				UE_LOG(LogTRGame, Log, TEXT("ServerLoadPlayerData - loaded player data for guid %s from: %s"), *SaveGame->PlayerSaveData.PlayerGuid.ToString(EGuidFormats::Digits), *PlayerSaveFilename);
			}						
		}
		else {
			UE_LOG(LogTRGame, Error, TEXT("ServerLoadPlayerData - error loading save file: %s"), *PlayerSaveFilename);
		}
	}
	else 
	{
		UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("ServerLoadPlayerData - No save file found: %s. Using current data."), *PlayerSaveFilename);
		// No existing file, setup guids and init data.
		if (!TRPlayerState->PlayerGuid.IsValid()) 
		{ 
			if (ForcePlayerGuid.IsValid()) { 
				TRPlayerState->PlayerGuid = ForcePlayerGuid; 
			}
			else 
			{ 				
				TRPlayerState->PlayerGuid = FGuid::NewGuid(); 
				UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("ServerLoadPlayerData - Generated new player GUID %s"), *TRPlayerState->PlayerGuid.ToString(EGuidFormats::Digits));
			}
		}					
		if (TRPlayerController->IsLocalController())
		{
			// Update game instance with our local info
			UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
			if (GameInst)
			{
				GameInst->ClientLocalProfileName = TRPlayerState->ProfileName;
				GameInst->ClientLocalPlayerGuid = TRPlayerState->PlayerGuid;
			}
		}
		else
		{
			// Get data from live entities
			FPlayerSaveData NewSaveData;
			TRPlayerController->GetPlayerSaveData(NewSaveData);
			// Call client to update data
			ClientEchoLoadPlayerData(NewSaveData);
		}
	}
	OnPlayerDataLoaded.Broadcast(); // Notify no matter what
}


bool UTRPersistentDataComponent::ServerLoadPlayerDataByGuid_Validate(const FGuid ForcePlayerGuid)
{
	return true;
}


void UTRPersistentDataComponent::ClientEchoLoadPlayerData_Implementation(const FPlayerSaveData PlayerSaveData)
{
	// Get parent PlayerController
	ATRPlayerControllerBase* TRPlayerController = Cast<ATRPlayerControllerBase>(GetOwner());
	if (TRPlayerController == nullptr) 
	{
		UE_LOG(LogTRGame, Error, TEXT("PersistentDataComponent ClientEchoLoadPlayerData - Could not get player controller."));
		OnPlayerDataLoaded.Broadcast(); // Notify anyway
		return;
	}
	UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("PersistentDataComponent ClientEchoLoadPlayerData - updating player data with GUID: %s."), *PlayerSaveData.PlayerGuid.ToString());
	// Controler UpdateFromPlayerSaveData - calls PlayerState.UpdateFromPlayerSaveData
	TRPlayerController->UpdateFromPlayerSaveData(PlayerSaveData);		
	// Save the incoming data locally if we are not the sever. (should always be true)
	if (GetOwnerRole() < ROLE_Authority) 
	{
		// We are on the client, so update our GameInsance info
		UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
		if (GameInst)
		{
			GameInst->ClientLocalProfileName = PlayerSaveData.ProfileName;
			GameInst->ClientLocalPlayerGuid = PlayerSaveData.PlayerGuid;
		}
		SavePlayerDataInternal();
	}
	// Call notification
	OnPlayerDataLoaded.Broadcast();
}


bool UTRPersistentDataComponent::ClientEchoLoadPlayerData_Validate(const FPlayerSaveData PlayerSaveData)
{
	return true;
}


void UTRPersistentDataComponent::ClientRequestPlayerDataFromClient_Implementation()
{
	UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("ClientRequestPlayerDataFromClient - starting"));
	ATRPlayerControllerBase* TRPlayerController = Cast<ATRPlayerControllerBase>(GetOwner());
	if (TRPlayerController == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("ClientRequestPlayerDataFromClient - Could not get player controller."));
		return;
	}
	ATRPlayerState* TRPlayerState = Cast<ATRPlayerState>(TRPlayerController->PlayerState);
	if (TRPlayerState == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("ClientRequestPlayerDataFromClient - Could not get player state."));
		return;
	}
	if (!TRPlayerState->PlayerGuid.IsValid())
	{
		// May be a new controller instance. Try getting guid from GI
		UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
		if (GameInst)
		{
			TRPlayerState->ProfileName = GameInst->ClientLocalProfileName;
			TRPlayerState->PlayerGuid = GameInst->ClientLocalPlayerGuid;
		}
	}
	FString PlayerSaveFilename = GetPlayerSaveFilename();
	if (PlayerSaveFilename.IsEmpty())
	{
		UE_LOG(LogTRGame, Error, TEXT("ClientRequestPlayerDataFromClient - player save file name is empty."));
		return;
	}
	// Gather our level template data to send.
	// TODO: will need to do this differently when/if player has a lot of level templates unlocked.
	TArray<FLevelTemplateContextStruct> LTCStructs;
	UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
	if (GameInst)
	{
		GameInst->LoadLevelTemplatesData();
		TArray<ULevelTemplateContext*> LTCs = GameInst->GetLevelTemplatesForPlayer(TRPlayerState->PlayerGuid);
		UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("ClientRequestPlayerDataFromClient - loaded %d level template contexts"), LTCs.Num());
		for (ULevelTemplateContext* LTC : LTCs)
		{
			if (LTC)
			{
				FLevelTemplateContextStruct LTCStruct = LTC->ToStruct();
				// Strip out PLRs
				LTCStruct.PlayerRecords.Empty();
				// Add in only the PLR for this player and only if it has been unlocked
				FPlayerLevelRecord PLR = GameInst->GetLevelTemplatePlayerRecord(LTCStruct.LevelTemplate.LevelId, TRPlayerState->PlayerGuid);
				if (PLR.LevelId.IsValid() && PLR.Unlocked)
				{
					UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("ClientRequestPlayerDataFromClient - adding level template %s"), *LTCStruct.LevelTemplate.LevelId.ToString());
					LTCStruct.PlayerRecords.Add(PLR);
					LTCStructs.Add(LTCStruct);
				}
			}
		}
	}
	UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("ClientRequestPlayerDataFromClient - sending %d level template contexts"), LTCStructs.Num());
	// Get data from local save file if it exists
	if (UGameplayStatics::DoesSaveGameExist(PlayerSaveFilename, 0))
	{					
		UPlayerSave* SaveGame = Cast<UPlayerSave>(UGameplayStatics::LoadGameFromSlot(PlayerSaveFilename, 0));
		if (SaveGame)
		{	
			// Send the data to server.
			ServerPlayerDataFromClient(SaveGame->PlayerSaveData, LTCStructs);
			//TRPlayerController->UpdateFromPlayerSaveData(SaveGame->PlayerSaveData);
			UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("ClientRequestPlayerDataFromClient - loaded player data for guid %s from: %s"), *SaveGame->PlayerSaveData.PlayerGuid.ToString(EGuidFormats::Digits), *PlayerSaveFilename);
		}
		else {
			UE_LOG(LogTRGame, Error, TEXT("ClientRequestPlayerDataFromClient - error loading save file: %s"), *PlayerSaveFilename);
		}
	}
	else
	{
		UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("ClientRequestPlayerDataFromClient - No client-side save file with name %s, getting data from entities."), *PlayerSaveFilename);
		// No existing file, setup guids and init data.
		FPlayerSaveData NewSaveData;
		if (!TRPlayerState->PlayerGuid.IsValid())
		{
			TRPlayerState->PlayerGuid = FGuid::NewGuid();
			UE_LOG(LogTRGame, Log, TEXT("ClientRequestPlayerDataFromClient - no save file found, setting new player guid: %s."), *TRPlayerState->PlayerGuid.ToString(EGuidFormats::Digits));
		}
		// Get data from local entities and send to server.
		TRPlayerController->GetPlayerSaveData(NewSaveData);
		ServerPlayerDataFromClient(NewSaveData, LTCStructs);					
	}		
}


bool UTRPersistentDataComponent::ClientRequestPlayerDataFromClient_Validate()
{
	return true;
}


void UTRPersistentDataComponent::ServerPlayerDataFromClient_Implementation(const FPlayerSaveData PlayerSaveData, const TArray<FLevelTemplateContextStruct>& LevelTemplateContexts)
{
	UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("ServerPlayerDataFromClient - received data from client for player guid %s"), *PlayerSaveData.PlayerGuid.ToString(EGuidFormats::Digits));
	ATRPlayerControllerBase* TRPlayerController = Cast<ATRPlayerControllerBase>(GetOwner());
	if (TRPlayerController == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("PersistentDataComponent ServerPlayerDataFromClient - Could not get player controller."));
		OnPlayerDataLoaded.Broadcast(); // Notify anyway
		return;
	}
	ATRPlayerState* TRPlayerState = Cast<ATRPlayerState>(TRPlayerController->PlayerState);
	if (TRPlayerState == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("PersistentDataComponent ServerPlayerDataFromClient - Could not get player state."));
		OnPlayerDataLoaded.Broadcast(); // Notify anyway
		return;
	}
	if (PlayerSaveData.PlayerGuid.IsValid()) 
	{
		if (TRPlayerState->PlayerGuid.IsValid() && PlayerSaveData.PlayerGuid != TRPlayerState->PlayerGuid) {
			UE_LOG(LogTRGame, Warning, TEXT("PersistentDataComponent - ServerPlayerDataFromClient - recieved player data with GUID %s different from existing player GUID %s"), *PlayerSaveData.PlayerGuid.ToString(EGuidFormats::Digits), *TRPlayerState->PlayerGuid.ToString(EGuidFormats::Digits));
		}
		TRPlayerState->PlayerGuid = PlayerSaveData.PlayerGuid;
	}
	// Update data here on server
	TRPlayerController->UpdateFromPlayerSaveData(PlayerSaveData);
	// If this is the server-local player (should never happen)
	if (TRPlayerController->IsLocalController())
	{
		UE_LOG(LogTRGame, Warning, TEXT("ServerPlayerDataFromClient called for local server player controller."));
		// Update game instance with host's local info
		UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
		if (GameInst)
		{
			GameInst->ClientLocalProfileName = TRPlayerState->ProfileName;
			GameInst->ClientLocalPlayerGuid = TRPlayerState->PlayerGuid;
		}
	}
	// Update level templates
	UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
	if (GameInst)
	{
		int32 LTCsAdded = 0;
		GameInst->LoadLevelTemplatesData();
		UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("ServerPlayerDataFromClient - Initial template contexts %d"), GameInst->LevelTemplatesMap.Num());
		for (FLevelTemplateContextStruct LTCStruct : LevelTemplateContexts)
		{
			if (LTCStruct.PlayerRecords.Num() > 0)
			{
				if (LTCStruct.PlayerRecords[0].PlayerGuid == TRPlayerState->PlayerGuid && LTCStruct.PlayerRecords[0].Unlocked)
				{
					UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("ServerPlayerDataFromClient - Adding level template %s"), *LTCStruct.LevelTemplate.LevelId.ToString());
					GameInst->AddLevelTemplate(LTCStruct.LevelTemplate);
					GameInst->UnlockLevelTemplateForPlayer(LTCStruct.LevelTemplate.LevelId, TRPlayerState->PlayerGuid);
					LTCsAdded++;
				}
			}
		}
		// If we added LTCs save the data.
		if (LTCsAdded > 0) {
			GameInst->SaveLevelTemplatesData(); // this also calls SavePlayerRecordsData
		}
		// If we are in the lobby, notify lobby GM. (so it can relay to other players)
		ATRGameModeLobby* LobbyGameMode = Cast<ATRGameModeLobby>(GetWorld()->GetAuthGameMode());
		if (LobbyGameMode) {
			UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("ServerPlayerDataFromClient - Notifying Lobby Game mode of new level templates"));
			LobbyGameMode->LevelTemplatesChanged();
		}
	}
	// Save the data we just got.
	SavePlayerDataInternal();
	// Call standard load of data to trigger normal notications etc.
	// ServerLoadPlayerData();
	// Call notification about new player data loaded
	OnPlayerDataLoaded.Broadcast();
}


bool UTRPersistentDataComponent::ServerPlayerDataFromClient_Validate(const FPlayerSaveData PlayerSaveData, const TArray<FLevelTemplateContextStruct>& LevelTemplateContexts)
{
	return true;
}


void UTRPersistentDataComponent::ServerDeletePlayerProfile_Implementation(const FGuid PlayerGuid, const bool bRemote)
{
	FString PlayerSaveFilename = GetPlayerSaveFilenameForGuid(PlayerGuid, bRemote);
	if (PlayerSaveFilename.IsEmpty())
	{
		UE_LOG(LogTRGame, Error, TEXT("ServerDeletePlayerProfile - Save filename invalid - no player guid invalid."));
		return;
	}
	if (UGameplayStatics::DoesSaveGameExist(PlayerSaveFilename, 0))	{
		UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("Deleting %s profile for player GUID %s"), bRemote ? *FString(TEXT("Remote")) : *FString(TEXT("Local")), *PlayerGuid.ToString(EGuidFormats::Digits));
		UGameplayStatics::DeleteGameInSlot(PlayerSaveFilename, 0);
	}
	else {
		UE_LOG(LogTRGame, Warning, TEXT("ServerDeletePlayerProfile - No profile exists with filename %s"), *PlayerSaveFilename);
	}
}

bool UTRPersistentDataComponent::ServerDeletePlayerProfile_Validate(const FGuid PlayerGuid, const bool bRemote)
{
	return true;
}


void UTRPersistentDataComponent::ServerRetrieveGoodsMarketData_Implementation()
{
	TArray<FGoodsPurchaseItem> MarketGoods;
	ATRGameModeLobby* GameMode = Cast<ATRGameModeLobby>(UGameplayStatics::GetGameMode(GetOwner()));
	ATRPlayerControllerBase* TRPlayerController = Cast<ATRPlayerControllerBase>(GetOwner());
	if (GameMode)
	{
		if (GameMode->GetMarketDataForPlayer(TRPlayerController, MarketGoods) > 0)
		{
			if (TRPlayerController->IsLocalController()) {
				OnGoodsMarketDataRetrieved.Broadcast(MarketGoods);
			}
			else {
				ClientEchoGoodsMarketData(MarketGoods);
			}
		}
	}
}

bool UTRPersistentDataComponent::ServerRetrieveGoodsMarketData_Validate()
{
	return true;
}


void UTRPersistentDataComponent::ClientEchoGoodsMarketData_Implementation(const TArray<FGoodsPurchaseItem>& GoodsMarketData)
{
	OnGoodsMarketDataRetrieved.Broadcast(GoodsMarketData);
}

bool UTRPersistentDataComponent::ClientEchoGoodsMarketData_Validate(const TArray<FGoodsPurchaseItem>& GoodsMarketData)
{
	return true;
}


void UTRPersistentDataComponent::ServerRetrieveToolsMarketData_Implementation()
{
	TArray<FToolPurchaseItem> ToolsMarketItems;
	ATRGameModeLobby* GameMode = Cast<ATRGameModeLobby>(UGameplayStatics::GetGameMode(GetOwner()));
	ATRPlayerControllerBase* TRPlayerController = Cast<ATRPlayerControllerBase>(GetOwner());
	if (GameMode)
	{
		if (GameMode->GetToolMarketDataForPlayer(TRPlayerController, ToolsMarketItems) > 0)
		{
			if (TRPlayerController->IsLocalController()) {
				OnToolsMarketDataRetrieved.Broadcast(ToolsMarketItems);
			}
			else {
				ClientEchoToolsMarketData(ToolsMarketItems);
			}
		}
	}
}

bool UTRPersistentDataComponent::ServerRetrieveToolsMarketData_Validate()
{
	return true;
}


void UTRPersistentDataComponent::ClientEchoToolsMarketData_Implementation(const TArray<FToolPurchaseItem>& ToolsMarketData)
{
	OnToolsMarketDataRetrieved.Broadcast(ToolsMarketData);
}

bool UTRPersistentDataComponent::ClientEchoToolsMarketData_Validate(const TArray<FToolPurchaseItem>& ToolsMarketData)
{
	return true;
}


void UTRPersistentDataComponent::ServerRetrieveNextPlayerLevelUpData_Implementation()
{
	ATRGameModeLobby* GameMode = Cast<ATRGameModeLobby>(UGameplayStatics::GetGameMode(GetOwner()));
	ATRPlayerControllerBase* TRPlayerController = Cast<ATRPlayerControllerBase>(GetOwner());
	if (GameMode)
	{
		FPlayerLevelUpData LevelUpData;
		bool bHasNextLevel = GameMode->GetLevelUpDataForPlayer(TRPlayerController, LevelUpData);
		if (TRPlayerController->IsLocalController()) {
			OnNextPlayerLevelUpDataRetrieved.Broadcast(bHasNextLevel, LevelUpData);
		}
		else {
			ClientEchoNextPlayerLevelUpData(bHasNextLevel, LevelUpData);
		}
	}
}

bool UTRPersistentDataComponent::ServerRetrieveNextPlayerLevelUpData_Validate()
{
	return true;
}


void UTRPersistentDataComponent::ClientEchoNextPlayerLevelUpData_Implementation(const bool bHasNextLevel, const FPlayerLevelUpData& LevelUpData)
{
	OnNextPlayerLevelUpDataRetrieved.Broadcast(bHasNextLevel, LevelUpData);
}

bool UTRPersistentDataComponent::ClientEchoNextPlayerLevelUpData_Validate(const bool bHasNextLevel, const FPlayerLevelUpData& LevelUpData)
{
	return true;
}