// Fill out your copyright notice in the Description page of Project Settings.

#include "RoomPlatformGridMgr.h"

#include "TargetRunner.h"
#include "Kismet/GameplayStatics.h"
#include "..\Public\RoomPlatformGridMgr.h"

// Sets default values
ARoomPlatformGridMgr::ARoomPlatformGridMgr()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ARoomPlatformGridMgr::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARoomPlatformGridMgr, BlackoutCells);
}

// Called when the game starts or when spawned
void ARoomPlatformGridMgr::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARoomPlatformGridMgr::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void ARoomPlatformGridMgr::GenerateGridImpl()
{
	// Destroy old grid, if any
	DestroyGrid();

	UE_LOG(LogTRGame, Log, TEXT("%s GenerateGrid - Generating grid."), *this->GetName());

	bool bSuccessful;
	//GridForgeClass = UGridForgeBase::StaticClass();
	
	UGridForgeBase* GridForge = NewObject<UGridForgeBase>(this, GridForgeClass);
	if (GridForge)
	{
		GridForge->BlackoutCells = BlackoutCells;
	}
	else
	{
		UE_LOG(LogTRGame, Error, TEXT("%s GenerateGrid - Could not construct GridForge."), *this->GetName());
		return;
	}
	// TODO Seed this correctly - currently uses values added in editor.
	//GridRandStream.GenerateNewSeed();
	GridRandStream.Reset();
	UE_LOG(LogTRGame, Log, TEXT("GridRandStream seed: %d"), GridRandStream.GetInitialSeed());
	
	RoomGridTemplate.GridCellWorldSize = GridCellWorldSize;
	RoomGridTemplate.RoomCellSubdivision = RoomCellSubdivision;
	if (RoomGridTemplate.RoomCellSubdivision == 0) { RoomGridTemplate.RoomCellSubdivision = 1; }
	RoomGridTemplate.GridExtentMinX = GridExtentMinX;
	RoomGridTemplate.GridExtentMaxX = GridExtentMaxX;
	RoomGridTemplate.GridExtentMinY = GridExtentMinY;
	RoomGridTemplate.GridExtentMaxY = GridExtentMaxY;
	if (bGenerateStartEnd)
	{
		RoomGridTemplate.StartCells.Empty();
		RoomGridTemplate.EndCells.Empty();
	}
	else
	{
		RoomGridTemplate.StartCells.Add(StartGridCoords);
		RoomGridTemplate.EndCells.Add(ExitGridCoords);
	}
	GridForge->GenerateGridTemplate(GridRandStream, RoomGridTemplate, bSuccessful);

	if (bSuccessful)
	{
		UE_LOG(LogTRGame, Log, TEXT("%s GenerateGrid - Generating grid successful."), *this->GetName());

		StartGridCoords = RoomGridTemplate.StartCells[0];
		ExitGridCoords = RoomGridTemplate.EndCells[0];
		
		MovePlayerStarts();
		if (bSpawnGridAfterGenerate)
		{
			TArray<int32> RowNums;
			TArray<int32> ColNums;
			RoomGridTemplate.Grid.GenerateKeyArray(RowNums);
			for (int32 Row : RowNums)
			{
				ColNums.Empty(RoomGridTemplate.Grid.Find(Row)->RowRooms.Num());
				RoomGridTemplate.Grid.Find(Row)->RowRooms.GenerateKeyArray(ColNums);
				for (int32 Col : ColNums)
				{
					FRoomTemplate* Room = RoomGridTemplate.Grid.Find(Row)->RowRooms.Find(Col);
					if (Room != nullptr)
					{
						SpawnRoom(FVector2D(Row, Col));
					}
				}
			}

		}
	} 
	else
	{
		UE_LOG(LogTRGame, Warning, TEXT("Grid generation failed with grid forge: %s"), *GetNameSafe(GridForge));
	}
	GridForge->EmptyGridTemplateCells();

	if (GetLocalRole() == ROLE_Authority)
	{
		// Update the clients' room grid template
		ClientUpdateRoomGridTemplate(RoomGridTemplate);
	}
}

void ARoomPlatformGridMgr::DestroyGridImpl()
{
	TArray<int32> RowNums;
	TArray<int32> PlatformNums;
	PlatformGridMap.GenerateKeyArray(RowNums);

	UE_LOG(LogTRGame, Warning, TEXT("Destroying %d rows."), RowNums.Num());
	for (int32 Row : RowNums)
	{
		PlatformGridMap.Find(Row)->RowPlatforms.GenerateKeyArray(PlatformNums);
		for (int32 Col : PlatformNums)
		{
			APlatformBase* Platform = PlatformGridMap.Find(Row)->RowPlatforms[Col];
			if (IsValid(Platform))
			{
				UE_LOG(LogTRGame, Warning, TEXT("Destroying room X:%d Y:%d."), Platform->GridX, Platform->GridY);
				Platform->Destroy();
			}
		}
		PlatformGridMap.Find(Row)->RowPlatforms.Empty();
	}
	PlatformGridMap.Empty();

	RowNums.Empty(RoomGridTemplate.Grid.Num());
	RoomGridTemplate.Grid.GenerateKeyArray(RowNums);
	for (int32 Row : RowNums)
	{
		RoomGridTemplate.Grid.Find(Row)->RowRooms.Empty();
	}
	RoomGridTemplate.Grid.Empty();
	RoomGridTemplate.StartCells.Empty();
	RoomGridTemplate.EndCells.Empty();

	if (GetLocalRole() == ROLE_Authority)
	{
		// Update the clients' room grid template
		ClientUpdateRoomGridTemplate(RoomGridTemplate);
	}
}

void ARoomPlatformGridMgr::SpawnRoom_Implementation(FVector2D GridCoords)
{
	// First remove any existing platform.
	bool bSuccess;
	APlatformBase* OldRoom = RemovePlatformFromGrid(GridCoords, bSuccess);
	if (IsValid(OldRoom))
	{
		UE_LOG(LogTRGame, Warning, TEXT("Destroying old room X:%d Y:%d."), OldRoom->GridX, OldRoom->GridY);
		OldRoom->Destroy();
	}

	ARoomPlatformBase* NewRoom;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = this;

	UE_LOG(LogTRGame, Log, TEXT("%s SpawnRoom - Spanwing room actor for X:%d Y:%d."), *this->GetName(), (int32)GridCoords.X, (int32)GridCoords.Y);

	// Create appropriate wall state arrays representing the used wall types, sized according to RoomCellSubdivision.
	int32 CenterSubdivision = (RoomGridTemplate.RoomCellSubdivision / 2);
	TArray<ETRWallState> DoorWall;
	for (int i = 0; i < RoomGridTemplate.RoomCellSubdivision; i++)
	{
		if (i == CenterSubdivision) { DoorWall.Add(ETRWallState::Door); }
		else { DoorWall.Add(ETRWallState::Blocked); }
	}
	TArray<ETRWallState> SolidWall;
	for (int i = 0; i < RoomGridTemplate.RoomCellSubdivision; i++)
	{
		SolidWall.Add(ETRWallState::Blocked);
	}
	TArray<ETRWallState> EmptyWall;
	for (int i = 0; i < RoomGridTemplate.RoomCellSubdivision; i++)
	{
		EmptyWall.Add(ETRWallState::Empty);
	}

	FRoomGridRow* GridRow = RoomGridTemplate.Grid.Find(GridCoords.X);
	if (GridRow == nullptr) { UE_LOG(LogTRGame, Error, TEXT("%s - Could not find grid row %d"), *this->GetName(), (int32)GridCoords.X); }
	FRoomTemplate* RoomTemplate = GridRow->RowRooms.Find(GridCoords.Y);
	if (RoomTemplate == nullptr) { UE_LOG(LogTRGame, Error, TEXT("%s - Could not find room template at X:%d Y:%d"), *this->GetName(), (int32)GridCoords.X, (int32)GridCoords.Y); }

	NewRoom = GetWorld()->SpawnActor<ARoomPlatformBase>(RoomClass, GetGridCellWorldTransform(GridCoords), SpawnParams);
	NewRoom->MyGridManager = this;
	NewRoom->GridX = GridCoords.X;
	NewRoom->GridY = GridCoords.Y;
	// Create wall template, sized so each of the four walls gets [RoomCellSubdivision] entries.
	NewRoom->WallTemplate.Empty(4 * RoomGridTemplate.RoomCellSubdivision);
	TArray<ETRWallState> Walls;
	Walls.Add(RoomTemplate->NorthWall);
	Walls.Add(RoomTemplate->EastWall);
	Walls.Add(RoomTemplate->SouthWall);
	Walls.Add(RoomTemplate->WestWall);
	// Append wall segments for each wall. 
	for (ETRWallState WallState : Walls)
	{
		NewRoom->WallTemplate.Append(WallState == ETRWallState::Blocked ? SolidWall : (WallState == ETRWallState::Door ? DoorWall : EmptyWall));
	}
	AddPlatformToGridMap(NewRoom);
	NewRoom->GenerateRoom();
}

void ARoomPlatformGridMgr::ClientUpdateRoomGridTemplate_Implementation(const FRoomGridTemplate& UpdatedTemplate)
{
	// Multicast replication but only needed on clients.
	if (GetLocalRole() < ROLE_Authority)
	{
		RoomGridTemplate = UpdatedTemplate;
	}
}

TArray<FRoomTemplate*> ARoomPlatformGridMgr::GetAllRoomTemplates()
{
	TArray<FRoomTemplate*> AllRooms;
	TArray<int32> RowNums;
	TArray<int32> ColNums;
	RoomGridTemplate.Grid.GenerateKeyArray(RowNums);

	for (int32 Row : RowNums)
	{
		ColNums.Empty(RoomGridTemplate.Grid.Find(Row)->RowRooms.Num());
		RoomGridTemplate.Grid.Find(Row)->RowRooms.GenerateKeyArray(ColNums);
		for (int32 Col : ColNums)
		{
			FRoomTemplate* Room = RoomGridTemplate.Grid.Find(Row)->RowRooms.Find(Col);
			if (Room != nullptr)
			{
				AllRooms.Add(Room);
			}
		}
	}

	return AllRooms;
}
