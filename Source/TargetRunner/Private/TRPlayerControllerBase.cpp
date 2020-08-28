// Fill out your copyright notice in the Description page of Project Settings.


#include "TRPlayerControllerBase.h"
#include "TRPlayerState.h"
#include "TargetRunner.h"
#include "Kismet/GameplayStatics.h"
#include "..\Public\TRPlayerControllerBase.h"

ATRPlayerControllerBase::ATRPlayerControllerBase()
	: Super()
{
	MaxEquippedWeapons = 2;
	CurrentTool = nullptr;
	PersistentDataComponent = CreateDefaultSubobject<UTRPersistentDataComponent>(TEXT("PersistentDataComponent"));
	if (PersistentDataComponent)
	{
		AddOwnedComponent(PersistentDataComponent);
		PersistentDataComponent->SetIsReplicated(true);
	}
}


void ATRPlayerControllerBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATRPlayerControllerBase, CurrentTool);
	DOREPLIFETIME(ATRPlayerControllerBase, MaxEquippedWeapons);
}


void ATRPlayerControllerBase::OnRep_CurrentTool()
{
	OnCurrentToolChanged.Broadcast(CurrentTool);
}

void ATRPlayerControllerBase::ServerAddToolToInventory_Implementation(TSubclassOf<UToolBase> ToolClass)
{
	UToolBase* TmpTool = NewObject<UToolBase>(this, ToolClass);
	if (TmpTool)
	{
		FToolData TmpToolData;
		TmpTool->ItemGuid = FGuid::NewGuid();
		TmpTool->ToToolData(TmpToolData);
		ToolInventory.Add(TmpTool->ItemGuid, TmpToolData);
		OnToolInventoryAdded.Broadcast(TmpToolData);
	}
	UE_LOG(LogTRGame, Log, TEXT("TRPlayerControllerBase - ServerAddToolToInventory added tool guid %s."), *TmpTool->ItemGuid.ToString());
	if (!IsLocalController())
	{
		UE_LOG(LogTRGame, Log, TEXT("TRPlayerControllerBase - ServerAddToolToInventory calling client."));
		ClientAddToolToInventory(ToolClass, TmpTool->ItemGuid);
	}
}

bool ATRPlayerControllerBase::ServerAddToolToInventory_Validate(TSubclassOf<UToolBase> ToolClass)
{
	return true;
}


void ATRPlayerControllerBase::ClientAddToolToInventory_Implementation(TSubclassOf<UToolBase> ToolClass, const FGuid AddedGuid)
{
	UToolBase* TmpTool = NewObject<UToolBase>(this, ToolClass);
	if (TmpTool)
	{
		FToolData TmpToolData;
		TmpTool->ItemGuid = AddedGuid;
		TmpTool->ToToolData(TmpToolData);
		ToolInventory.Add(TmpTool->ItemGuid, TmpToolData);
		OnToolInventoryAdded.Broadcast(TmpToolData);
	}
	UE_LOG(LogTRGame, Log, TEXT("TRPlayerControllerBase - ClientAddToolToInventory added tool guid %s."), *TmpTool->ItemGuid.ToString());
}

bool ATRPlayerControllerBase::ClientAddToolToInventory_Validate(TSubclassOf<UToolBase> ToolClass, const FGuid AddedGuid)
{
	return true;
}


void ATRPlayerControllerBase::GetMarketTools_Implementation(TArray<TSubclassOf<UToolBase>>& AvailableMarketToolClasses)
{
	AvailableMarketToolClasses.Empty(MarketToolClasses.Num());
	AvailableMarketToolClasses.Append(MarketToolClasses);
}


void ATRPlayerControllerBase::SetMarketTools_Implementation(const TArray<TSubclassOf<UToolBase>>& AvailableMarketToolClasses)
{
	MarketToolClasses.Empty(AvailableMarketToolClasses.Num());
	MarketToolClasses.Append(AvailableMarketToolClasses);
}


void ATRPlayerControllerBase::ServerEquipTool_Implementation(FGuid ToolGuid)
{
	FToolData* ToolData = ToolInventory.Find(ToolGuid);
	if (ToolData == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("TRPlayerControllerBase - ServerEquipTool tool guid %s not found in inventory."), *ToolGuid.ToString());
		return;
	}
	while (EquippedTools.Num() >= MaxEquippedWeapons)
	{
		EquippedTools.RemoveAt(0);
	}
	UToolBase* TmpTool = UToolBase::CreateToolFromToolData(*ToolData, this);
	if (TmpTool && EquippedTools.Num() <= MaxEquippedWeapons) {
		EquippedTools.Add(TmpTool);
		OnEquippedToolsChanged.Broadcast();
		UE_LOG(LogTRGame, Log, TEXT("TRPlayerControllerBase - ServerEquipTool tool %s equipped."), *ToolData->AttributeData.ItemDisplayName.ToString());
		if (!IsLocalController())
		{
			UE_LOG(LogTRGame, Log, TEXT("TRPlayerControllerBase - ServerEquipTool calling client rpc."));
			ClientEquipTool(ToolGuid);
		}
	}
}

bool ATRPlayerControllerBase::ServerEquipTool_Validate(FGuid ToolGuid)
{
	return true;
}


void ATRPlayerControllerBase::ClientEquipTool_Implementation(FGuid ToolGuid)
{
	FToolData* ToolData = ToolInventory.Find(ToolGuid);
	if (ToolData == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("TRPlayerControllerBase - ClientEquipTool tool guid %s not found in inventory."), *ToolGuid.ToString());
		return;
	}
	while (EquippedTools.Num() >= MaxEquippedWeapons)
	{
		EquippedTools.RemoveAt(0);
	}
	UToolBase* TmpTool = UToolBase::CreateToolFromToolData(*ToolData, this);
	if (TmpTool && EquippedTools.Num() <= MaxEquippedWeapons) {
		EquippedTools.Add(TmpTool);
		OnEquippedToolsChanged.Broadcast();
		UE_LOG(LogTRGame, Log, TEXT("TRPlayerControllerBase - ClientEquipTool tool %s equipped."), *ToolData->AttributeData.ItemDisplayName.ToString());
	}
}

bool ATRPlayerControllerBase::ClientEquipTool_Validate(FGuid ToolGuid)
{
	return true;
}


void ATRPlayerControllerBase::ServerUnequipTool_Implementation(FGuid ToolGuid)
{
	UToolBase* FoundTool = nullptr;
	for (UToolBase* TmpTool : EquippedTools)
	{
		if (TmpTool->ItemGuid == ToolGuid)
		{
			FoundTool = TmpTool;
			break;
		}
	}
	if (FoundTool)
	{
		EquippedTools.Remove(FoundTool);
		OnEquippedToolsChanged.Broadcast();
		if (!IsLocalController())
		{
			ClientUnequipTool(ToolGuid);
		}
	}	
}

bool ATRPlayerControllerBase::ServerUnequipTool_Validate(FGuid ToolGuid)
{
	return true;
}


void ATRPlayerControllerBase::ClientUnequipTool_Implementation(FGuid ToolGuid)
{
	UToolBase* FoundTool = nullptr;
	for (UToolBase* TmpTool : EquippedTools)
	{
		if (TmpTool->ItemGuid == ToolGuid)
		{
			FoundTool = TmpTool;
			break;
		}
	}
	if (FoundTool)
	{
		EquippedTools.Remove(FoundTool);
		OnEquippedToolsChanged.Broadcast();
	}
}

bool ATRPlayerControllerBase::ClientUnequipTool_Validate(FGuid ToolGuid)
{
	return true;
}


void ATRPlayerControllerBase::ServerSetCurrentTool_Implementation(FGuid ToolGuid)
{
	UToolBase* FoundTool = nullptr;
	for (UToolBase* TmpTool : EquippedTools)
	{
		if (TmpTool->ItemGuid == ToolGuid)
		{
			FoundTool = TmpTool;
			break;
		}
	}
	if (FoundTool)
	{
		SpawnAsCurrentTool(FoundTool);
	}
	else
	{
		UE_LOG(LogTRGame, Error, TEXT("TRPlayerController::ServerSetCurrentTool - Tool guid: %s not found in equipped tools."), *ToolGuid.ToString(EGuidFormats::Digits));
	}
}

bool ATRPlayerControllerBase::ServerSetCurrentTool_Validate(FGuid ToolGuid)
{
	return true;
}


void ATRPlayerControllerBase::SpawnAsCurrentTool_Implementation(UToolBase* NewCurrentTool)
{
	FTransform SpawnTransform;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	if (NewCurrentTool && IsValid(NewCurrentTool))
	{
		if (CurrentTool)
		{
			CurrentTool->Destroy();
		}
		APawn* CurPawn = GetPawn();
		CurrentTool = nullptr;
		if (CurPawn) {
			SpawnParams.Owner = this;
			SpawnParams.Instigator = CurPawn;
			SpawnTransform = CurPawn->GetTransform();
			CurrentTool = GetWorld()->SpawnActorDeferred<AToolActorBase>(NewCurrentTool->ToolActorClass, SpawnTransform, SpawnParams.Owner, SpawnParams.Instigator, SpawnParams.SpawnCollisionHandlingOverride);
			CurrentTool->Tool = NewCurrentTool;
			CurrentTool->WeaponState = ETRWeaponState::Idle;
			CurrentTool->SetOwner(this);
			UGameplayStatics::FinishSpawningActor(CurrentTool, CurrentTool->GetTransform());
		}
		// Manually call rep_notify on server
		if (GetLocalRole() == ROLE_Authority) { OnRep_CurrentTool(); }
	}
}


ARoomPlatformGridMgr* ATRPlayerControllerBase::FindGridManager()
{
	ARoomPlatformGridMgr* GridManager = nullptr;
	TArray<AActor*> FoundManagers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARoomPlatformGridMgr::StaticClass(), FoundManagers);
	if (FoundManagers.Num() > 0)
	{
		GridManager = Cast<ARoomPlatformGridMgr>(FoundManagers[0]);
	}
	return GridManager;
}


void ATRPlayerControllerBase::ClientUpdateRoomGridTemplate_Implementation(const FRoomGridTemplate& UpdatedTemplate, const TArray<FVector2D>& RoomCoords, const TArray<FRoomTemplate>& RoomTemplates)
{
	ARoomPlatformGridMgr* GridManager = FindGridManager();
	if (GridManager != nullptr)
	{
		GridManager->DestroyGrid();
		GridManager->SetRoomGridTemplateData(UpdatedTemplate, RoomCoords, RoomTemplates);
		GridManager->SpawnRooms();
	}
	else
	{
		UE_LOG(LogTRGame, Warning, TEXT("ClientUpdateRoomGridTemplate - No Grid mgr found."))
	}
}


void ATRPlayerControllerBase::GetPlayerSaveData_Implementation(FPlayerSaveData& SaveData)
{
	ToolInventory.GenerateValueArray(SaveData.ToolInventory);
	SaveData.LastEquippedItems.Empty(EquippedTools.Num());
	for (UToolBase* TmpTool : EquippedTools)
	{
		SaveData.LastEquippedItems.Add(TmpTool->ItemGuid);
	}
	SaveData.AttributeData.FloatAttributes.Add(FName(TEXT("MaxEquippedWeapons")), static_cast<float>(MaxEquippedWeapons));
	ATRPlayerState* TRPlayerState = Cast<ATRPlayerState>(PlayerState);
	if (TRPlayerState)
	{
		TRPlayerState->GetPlayerSaveData(SaveData);
	}
	else
	{
		UE_LOG(LogTRGame, Warning, TEXT("GetPlayerSaveData - No player state found."))
	}
}


void ATRPlayerControllerBase::UpdateFromPlayerSaveData_Implementation(const FPlayerSaveData& SaveData)
{
	UToolBase* TmpTool = nullptr;
	ToolInventory.Empty();
	for (FToolData CurToolData : SaveData.ToolInventory)
	{
		ToolInventory.Add(CurToolData.AttributeData.ItemGuid, CurToolData);
	}
	MaxEquippedWeapons = static_cast<int32>(SaveData.AttributeData.FloatAttributes.FindRef(FName(TEXT("MaxEquippedWeapons"))));
	MaxEquippedWeapons = MaxEquippedWeapons < 2 ? 2 : MaxEquippedWeapons;
	EquippedTools.Empty(SaveData.LastEquippedItems.Num());
	for (FGuid TmpGuid : SaveData.LastEquippedItems)
	{
		if (ToolInventory.Contains(TmpGuid))
		{
			TmpTool = UToolBase::CreateToolFromToolData(ToolInventory[TmpGuid], this);
			if (TmpTool && EquippedTools.Num() <= MaxEquippedWeapons) {
				EquippedTools.Add(TmpTool);
			}
		}
	}
	ATRPlayerState* TRPlayerState = Cast<ATRPlayerState>(PlayerState);
	if (TRPlayerState)
	{
		TRPlayerState->UpdateFromPlayerSaveData(SaveData);
	}
	else
	{
		UE_LOG(LogTRGame, Warning, TEXT("UpdateFromPlayerSaveData - No player state found."))
	}	
}