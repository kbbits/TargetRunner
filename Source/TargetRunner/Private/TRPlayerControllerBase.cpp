// Fill out your copyright notice in the Description page of Project Settings.


#include "TRPlayerControllerBase.h"
#include "Kismet/GameplayStatics.h"

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