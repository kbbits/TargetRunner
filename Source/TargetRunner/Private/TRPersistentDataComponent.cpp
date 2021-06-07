// Fill out your copyright notice in the Description page of Project Settings.


#include "TRPersistentDataComponent.h"
#include "..\Public\TRPersistentDataComponent.h"
#include "TRGameModeLobby.h"
#include "TRPlayerControllerBase.h"
#include "TRPlayerState.h"
#include "PlayerSave.h"
#include "Kismet/GameplayStatics.h"
#include "Paths.h"
#include "PlatformFile.h"
#include "PlatformFilemanager.h"
#include "PlatformFeatures.h"
#include "GameFramework/SaveGame.h"
#include "TRGameInstance.h"

const FString UTRPersistentDataComponent::PlayerFilenameSuffix = FString(TEXT("_player"));

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
					TArray<ULevelTemplateContext*> TmpLTCArray;
					GameInst->LevelTemplatesMap.GenerateValueArray(TmpLTCArray);
					LevelTemplatesPage.Empty();
					LevelTemplatesPage.Append(ULevelTemplateContext::ToStructArray(TmpLTCArray));
					LevelTemplatesRepTrigger++;
					UE_LOG(LogTRGame, Log, TEXT("UTRPersistentDataComponent - level template %s unlocked for player %s"), *LevelId.ToString(), *TRPlayerState->PlayerGuid.ToString(EGuidFormats::Digits));
					for (FLevelTemplateContextStruct TmpLTCS : LevelTemplatesPage)
					{
						UE_LOG(LogTRGame, Log, TEXT("UTRPersistentDataComponent - template now has %s player records: %d"), *TmpLTCS.LevelTemplate.LevelId.ToString(), TmpLTCS.PlayerRecords.Num());
					}

					// Manually call rep_notify on server
					if (GetOwnerRole() == ROLE_Authority) { OnRep_LevelTemplatesPageLoaded(); }
				}
				else
				{
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


void UTRPersistentDataComponent::ServerSetLevelTemplateForPlay_Implementation(const FLevelTemplate& LevelTemplate)
{
	UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
	if (GameInst) {
		GameInst->SetSelectedLevelTemplate(LevelTemplate);		
	}
	else {
		UE_LOG(LogTRGame, Error, TEXT("ServerSetLevelTemplateForPlay - Could not get game instance."));
	}
}

bool UTRPersistentDataComponent::ServerSetLevelTemplateForPlay_Validate(const FLevelTemplate& LevelTemplate)
{
	return true;
}


FString UTRPersistentDataComponent::GetPlayerSaveFilename()
{ 
	ATRPlayerControllerBase* TRPlayerController = Cast<ATRPlayerControllerBase>(GetOwner());
	if (TRPlayerController)
	{
		ATRPlayerState* TRPlayerState = Cast<ATRPlayerState>(TRPlayerController->PlayerState);
		if (TRPlayerState) {
			if (TRPlayerState->PlayerGuid.IsValid()) {
				return TRPlayerState->PlayerGuid.ToString(EGuidFormats::Digits) + PlayerFilenameSuffix;
			}
			else {
				UE_LOG(LogTRGame, Error, TEXT("GetPlayerSaveFilename - player guid is not valid."));
			}
		}
	}
	UE_LOG(LogTRGame, Error, TEXT("GetPlayerSaveFilename - Could not determine save filename."));
	return FString();
}


TArray<FString> UTRPersistentDataComponent::GetAllSaveProfileFilenames()
{
	//////////////////////////////////////////////////////////////////////////////
	class FFindSavesVisitor : public IPlatformFile::FDirectoryVisitor
	{
	public:
		FFindSavesVisitor() {}

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
		{
			if (!bIsDirectory)
			{
				if (FilenameOrDirectory)
				{
					UE_LOG(LogTRGame, Log, TEXT("GetAllSaveProfileNames - potential file: %s."), FilenameOrDirectory);
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
					if (CleanFilename.EndsWith(*UTRPersistentDataComponent::PlayerFilenameSuffix))
					{
						//CleanFilename = CleanFilename.Replace(TEXT(".sav"), TEXT(""));
						//CleanFilename = CleanFilename.Replace(*PLAYER_FILENAME_PREFIX, TEXT(""));
						SavesFound.Add(CleanFilename);
						UE_LOG(LogTRGame, Log, TEXT("GetAllSaveProfileNames - found file: %s."), *CleanFilename);
					}
				}
			}
			return true;
		}
		TArray<FString> SavesFound;
	};
	//////////////////////////////////////////////////////////////////////////////

	const FString SavesFolder = FPaths::ProjectSavedDir() + TEXT("SaveGames");
	UE_LOG(LogTRGame, Log, TEXT("GetAllSaveProfileNames - checking dir: %s."), *SavesFolder)
	if (!SavesFolder.IsEmpty())
	{
		FFindSavesVisitor Visitor;
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
			if (SaveGame)
			{
				AllFoundData.Add(SaveGame->PlayerSaveData);
			}
		}
	}
	return AllFoundData;
}


void UTRPersistentDataComponent::ServerSavePlayerData_Implementation()
{
	ATRPlayerControllerBase* TRPlayerController = Cast<ATRPlayerControllerBase>(GetOwner());
	if (TRPlayerController)
	{
		ATRPlayerState* TRPlayerState = Cast<ATRPlayerState>(TRPlayerController->PlayerState);
		if (TRPlayerState)
		{
			UPlayerSave* SaveGame = Cast<UPlayerSave>(UGameplayStatics::LoadGameFromSlot(GetPlayerSaveFilename(), 0));
			if (SaveGame == nullptr || !SaveGame->PlayerSaveData.PlayerGuid.IsValid())
			{
				SaveGame = Cast<UPlayerSave>(UGameplayStatics::CreateSaveGameObject(UPlayerSave::StaticClass()));
			}
			TRPlayerController->GetPlayerSaveData(SaveGame->PlayerSaveData);
			
			UE_LOG(LogTRGame, Log, TEXT("ServerSavePlayerData - Saving player data. Guid: %s"), *SaveGame->PlayerSaveData.PlayerGuid.ToString(EGuidFormats::Digits));
			FString InvBuff;
			if (SaveGame->PlayerSaveData.GoodsInventory.Num() == 0)
			{
				UE_LOG(LogTRGame, Log, TEXT("    GoodsInventory is empty."));
			}
			// TODO remove this debug logging
			for (FTRNamedFloat InvElem : SaveGame->PlayerSaveData.GoodsInventory)
			{
				InvBuff.Append(FString::Printf(TEXT("    %s: %f    \r\n"), *InvElem.Name.ToString(), InvElem.Quantity));
			}
			UE_LOG(LogTRGame, Log, TEXT("    Goods Inventory   \r\n  %s"), *InvBuff);

			UGameplayStatics::SaveGameToSlot(SaveGame, GetPlayerSaveFilename(), 0);
			UE_LOG(LogTRGame, Log, TEXT("ServerSavePlayerData - Player data saved to: %s"), *GetPlayerSaveFilename());
		}
		else {
			UE_LOG(LogTRGame, Error, TEXT("ServerSavePlayerData - Could not get player state."));
		}
	}
	else {
		UE_LOG(LogTRGame, Error, TEXT("ServerSavePlayerData - Could not get player controller."));
	}
}

bool UTRPersistentDataComponent::ServerSavePlayerData_Validate()
{
	return true;
}


void UTRPersistentDataComponent::ServerLoadPlayerData_Implementation(const FGuid PlayerGuid)
{
	ATRPlayerControllerBase* TRPlayerController = Cast<ATRPlayerControllerBase>(GetOwner());
	if (TRPlayerController)
	{
		ATRPlayerState* TRPlayerState = Cast<ATRPlayerState>(TRPlayerController->PlayerState);
		if (TRPlayerState)
		{
			TRPlayerState->PlayerGuid = PlayerGuid;
			FString PlayerSaveFilename = GetPlayerSaveFilename();
			if (!PlayerSaveFilename.IsEmpty())
			{
				if (UGameplayStatics::DoesSaveGameExist(PlayerSaveFilename, 0))
				{
					UPlayerSave* SaveGame = Cast<UPlayerSave>(UGameplayStatics::LoadGameFromSlot(PlayerSaveFilename, 0));
					if (SaveGame)
					{
						if (PlayerGuid.IsValid() && SaveGame->PlayerSaveData.PlayerGuid.IsValid() && SaveGame->PlayerSaveData.PlayerGuid != PlayerGuid) 
						{
							UE_LOG(LogTRGame, Warning, TEXT("Player guid in file: %s does not match player guid %s."), *SaveGame->PlayerSaveData.PlayerGuid.ToString(EGuidFormats::Digits), *PlayerGuid.ToString(EGuidFormats::Digits));
						}
						else
						{
							UE_LOG(LogTRGame, Log, TEXT("PersistentDataComponent - Loading player data. Guid: %s"), *SaveGame->PlayerSaveData.PlayerGuid.ToString(EGuidFormats::Digits));
							FString InvBuff;
							if (SaveGame->PlayerSaveData.GoodsInventory.Num() == 0)
							{
								UE_LOG(LogTRGame, Log, TEXT("    GoodsInventory is empty."));
							}
							for (FTRNamedFloat InvElem : SaveGame->PlayerSaveData.GoodsInventory)
							{
								InvBuff.Append(FString::Printf(TEXT("    %s: %f    \r\n"), *InvElem.Name.ToString(), InvElem.Quantity));
							}
							UE_LOG(LogTRGame, Log, TEXT("    Goods Inventory   \r\n  %s"), *InvBuff);
							// Update data on server
							TRPlayerController->UpdateFromPlayerSaveData(SaveGame->PlayerSaveData);
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
								UE_LOG(LogTRGame, Log, TEXT("PersistentDataComponent ServerLoadPlayerData going to echo existing to client with ClientEchoLoadPlayerData"));
								// Call client to update data
								ClientEchoLoadPlayerData(SaveGame->PlayerSaveData);
							}
							UE_LOG(LogTRGame, Log, TEXT("ServerLoadPlayerData - loaded player data from: %s"), *PlayerSaveFilename);
						}						
					}
					else {
						UE_LOG(LogTRGame, Log, TEXT("ServerLoadPlayerData - error loading save file: %s"), *PlayerSaveFilename);
					}
				}
				else 
				{
					// No existing file, setup guids and init data.
					FPlayerSaveData NewSaveData;
					if (!TRPlayerState->PlayerGuid.IsValid()) 
					{ 
						if (PlayerGuid.IsValid()) { TRPlayerState->PlayerGuid = PlayerGuid; }
						else { TRPlayerState->PlayerGuid = FGuid::NewGuid(); }
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
						UE_LOG(LogTRGame, Log, TEXT("PersistentDataComponent ServerLoadPlayerData going to echo new to client with ClientEchoLoadPlayerData"));
						// Call client to update data
						TRPlayerController->GetPlayerSaveData(NewSaveData);
						ClientEchoLoadPlayerData(NewSaveData);
					}
					UE_LOG(LogTRGame, Log, TEXT("ServerLoadPlayerData - no save file found, setting new player guid: %s."), *TRPlayerState->PlayerGuid.ToString(EGuidFormats::Digits));
				}
				OnPlayerDataLoaded.Broadcast();
			}
			else
			{
				UE_LOG(LogTRGame, Error, TEXT("ServerLoadPlayerData - no profile name set."));
			}
		} 
		else {
			UE_LOG(LogTRGame, Error, TEXT("ServerLoadPlayerData - Could not get player state."));
		}
	}
	else {
		UE_LOG(LogTRGame, Error, TEXT("ServerLoadPlayerData - Could not get player controller."));
	}
}


bool UTRPersistentDataComponent::ServerLoadPlayerData_Validate(const FGuid OptionalGuid)
{
	return true;
}


void UTRPersistentDataComponent::ClientEchoLoadPlayerData_Implementation(const FPlayerSaveData PlayerSaveData)
{
	// Get parent PlayerController
	// Controler UpdateFromPlayerSaveData - calls PlayerState.UpdateFromPlayerSaveData
	ATRPlayerControllerBase* TRPlayerController = Cast<ATRPlayerControllerBase>(GetOwner());
	if (TRPlayerController)
	{
		TRPlayerController->UpdateFromPlayerSaveData(PlayerSaveData);
		UE_LOG(LogTRGame, Log, TEXT("ClientEchoLoadPlayerData - updated player data for: %s."), *PlayerSaveData.PlayerGuid.ToString());
	}
	UTRGameInstance* GameInst = Cast<UTRGameInstance>(UGameplayStatics::GetGameInstance(GetOwner()));
	if (GameInst)
	{
		GameInst->ClientLocalProfileName = PlayerSaveData.ProfileName;
		GameInst->ClientLocalPlayerGuid = PlayerSaveData.PlayerGuid;
	}
	OnPlayerDataLoaded.Broadcast();
}


bool UTRPersistentDataComponent::ClientEchoLoadPlayerData_Validate(const FPlayerSaveData PlayerSaveData)
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
			if (TRPlayerController->IsLocalController())
			{
				OnGoodsMarketDataRetrieved.Broadcast(MarketGoods);
			}
			else
			{
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
			if (TRPlayerController->IsLocalController())
			{
				OnToolsMarketDataRetrieved.Broadcast(ToolsMarketItems);
			}
			else
			{
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
		if (TRPlayerController->IsLocalController())
		{
			OnNextPlayerLevelUpDataRetrieved.Broadcast(bHasNextLevel, LevelUpData);
		}
		else
		{
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