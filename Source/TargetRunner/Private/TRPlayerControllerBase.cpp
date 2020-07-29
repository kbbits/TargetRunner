// Fill out your copyright notice in the Description page of Project Settings.


#include "TRPlayerControllerBase.h"
#include "TRPlayerState.h"
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

	DOREPLIFETIME(ATRPlayerControllerBase, MaxEquippedWeapons);
}


void ATRPlayerControllerBase::ServerAddToolToInventory_Implementation(TSubclassOf<UToolBase> ToolClass)
{
	AddToolToInventory(ToolClass);
	if (!IsLocalController())
	{
		ClientAddToolToInventory(ToolClass);
	}
}

bool ATRPlayerControllerBase::ServerAddToolToInventory_Validate(TSubclassOf<UToolBase> ToolClass)
{
	return true;
}


void ATRPlayerControllerBase::ClientAddToolToInventory_Implementation(TSubclassOf<UToolBase> ToolClass)
{
	if (IsLocalController())
	{
		AddToolToInventory(ToolClass);
	}
}

bool ATRPlayerControllerBase::ClientAddToolToInventory_Validate(TSubclassOf<UToolBase> ToolClass)
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


void ATRPlayerControllerBase::AddToolToInventory(TSubclassOf<UToolBase> ToolClass)
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
}


void ATRPlayerControllerBase::ServerEquipTool_Implementation(FToolData ToolData)
{
	while (EquippedTools.Num() >= MaxEquippedWeapons)
	{
		EquippedTools.RemoveAt(0);
	}
	UToolBase* TmpTool = UToolBase::CreateToolFromToolData(ToolData, this);
	if (TmpTool && EquippedTools.Num() <= MaxEquippedWeapons) {
		EquippedTools.Add(TmpTool);
		OnEquippedToolsChanged.Broadcast();
		if (!IsLocalController())
		{
			ClientEquipTool(ToolData);
		}
	}
}

bool ATRPlayerControllerBase::ServerEquipTool_Validate(FToolData ToolData)
{
	return true;
}


void ATRPlayerControllerBase::ClientEquipTool_Implementation(FToolData ToolData)
{
	if (IsLocalController())
	{
		while (EquippedTools.Num() >= MaxEquippedWeapons)
		{
			EquippedTools.RemoveAt(0);
		}
		UToolBase* TmpTool = UToolBase::CreateToolFromToolData(ToolData, this);
		if (TmpTool && EquippedTools.Num() <= MaxEquippedWeapons) {
			EquippedTools.Add(TmpTool);
			OnEquippedToolsChanged.Broadcast();
		}
	}
}

bool ATRPlayerControllerBase::ClientEquipTool_Validate(FToolData ToolData)
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
	if (IsLocalController())
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
}

bool ATRPlayerControllerBase::ClientUnequipTool_Validate(FGuid ToolGuid)
{
	return true;
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