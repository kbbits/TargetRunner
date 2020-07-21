// Fill out your copyright notice in the Description page of Project Settings.


#include "TRPlayerControllerBase.h"
#include "TRPlayerState.h"
#include "Kismet/GameplayStatics.h"

ATRPlayerControllerBase::ATRPlayerControllerBase()
	: Super()
{
	PersistentDataComponent = CreateDefaultSubobject<UTRPersistentDataComponent>(TEXT("PersistentDataComponent"));
	if (PersistentDataComponent)
	{
		AddOwnedComponent(PersistentDataComponent);
		PersistentDataComponent->SetIsReplicated(true);
	}
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
	EquippedTools.Empty(SaveData.LastEquippedItems.Num());
	for (FGuid TmpGuid : SaveData.LastEquippedItems)
	{
		if (ToolInventory.Contains(TmpGuid))
		{
			UToolBase::FromToolData(ToolInventory[TmpGuid], this, TmpTool);
			if (TmpTool) {
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