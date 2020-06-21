// Fill out your copyright notice in the Description page of Project Settings.

#include "RoomPlatformGridMgr.h"
#include "..\Public\RoomPlatformGridMgr.h"

#include "TargetRunner.h"
#include "RoomFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TR_GameMode.h"

// Sets default values
ARoomPlatformGridMgr::ARoomPlatformGridMgr()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ResourceDropperClass = UResourceDropperBase::StaticClass();
}


//void ARoomPlatformGridMgr::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);	
//}


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
	bool bSuccessful = false;
	// Destroy old grid, if any
	DestroyGrid();
	DebugLog(FString::Printf(TEXT("%s GenerateGrid - Generating grid. Extents: MinX:%d MinY:%d  MaxX:%d MaxY:%d"), *this->GetName(), GridExtentMinX, GridExtentMinY, GridExtentMaxX, GridExtentMaxY));
	FRandomStream* GridStreamFound = nullptr;
	FRandomStream* ResourceStreamFound = nullptr;
	// Grab the game mode.
	ATR_GameMode* GameMode = Cast<ATR_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("%s GenerateGrid - Could not get GameMode. Using default rand stream."), *this->GetName());
		// Use our default streams
		GridStreamFound = &DefaultGridRandStream;
		GridStreamFound->Reset();
		ResourceStreamFound = &DefaultResourceDropperStream;
		ResourceStreamFound->Reset();
	}
	else 
	{
		// Get our random streams from game mode
		GridStreamFound = &GameMode->GetGridStream();
		ResourceStreamFound = &GameMode->GetResourceDropperStream();
	}
	FRandomStream& GridRandStream = *GridStreamFound;
	FRandomStream& ResourceDropperStream = *ResourceStreamFound;
	// Create the grid forge
	UGridForgeBase* GridForge = NewObject<UGridForgeBase>(this, GridForgeClass);
	if (GridForge == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("%s GenerateGrid - Could not construct GridForge."), *this->GetName());
		return;
	}
	GridForge->EmptyGridTemplateCells();
	// Populate basic properties of the template
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
	// Use override blackout cells if present
	if (OverrideBlackoutCells.Num() > 0)
	{
		// Use the manually specifid blackout cells.
		GridForge->BlackoutCells = OverrideBlackoutCells;
	}
	else
	{
		GridForge->BlackoutCells.Empty();
	}
	
	// Fill the RoomGridTemplate's grid
	GridForge->GenerateGridTemplate(GridRandStream, RoomGridTemplate, bSuccessful);

	if (bSuccessful)
	{
		DebugLog(FString::Printf(TEXT("%s GenerateGrid - Generating grid successful."), *GetNameSafe(this)));
		StartGridCoords = RoomGridTemplate.StartCells[0];
		ExitGridCoords = RoomGridTemplate.EndCells[0];
		// Allocate resources to room templates
		UResourceDropperBase* ResourceDropper = NewObject<UResourceDropperBase>(this, ResourceDropperClass);
		if (ResourceDropper == nullptr) {
			UE_LOG(LogTRGame, Error, TEXT("%s GenerateGrid - Could not construct ResourceDropper."), *GetNameSafe(this));
			return;
		}
		ResourceDropper->DistributeResources(ResourceDropperStream, ResourcesToDistribute, RoomGridTemplate);
		// Move/Create player start locations
		MovePlayerStarts();
		#if WITH_EDITOR
			if (bSpawnRoomsAfterGenerate) {	SpawnRooms();}
		#endif
	} 
	else
	{
		UE_LOG(LogTRGame, Warning, TEXT("Grid generation failed with grid forge: %s"), *GetNameSafe(GridForge));
	}
	// Clear the cell grid, just to be tidy.
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

	DebugLog(FString::Printf(TEXT("Destroying %d rows."), RowNums.Num()));
	for (int32 Row : RowNums)
	{
		PlatformGridMap.Find(Row)->RowPlatforms.GenerateKeyArray(PlatformNums);
		for (int32 Col : PlatformNums)
		{
			APlatformBase* Platform = PlatformGridMap.Find(Row)->RowPlatforms[Col];
			if (IsValid(Platform))
			{
				DebugLog(FString::Printf(TEXT("Destroying room X:%d Y:%d."), Platform->GridX, Platform->GridY));
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


void ARoomPlatformGridMgr::SpawnRooms_Implementation()
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
	if (GetLocalRole() == ROLE_Authority)
	{
		// Have the clients update their grid maps
		ClientFillGridFromExistingPlatforms();
	}
}


void ARoomPlatformGridMgr::SpawnRoom_Implementation(FVector2D GridCoords)
{
	// First remove any existing platform.
	bool bSuccess;
	APlatformBase* OldRoom = RemovePlatformFromGrid(GridCoords, bSuccess);
	if (IsValid(OldRoom))
	{
		DebugLog(FString::Printf(TEXT("Destroying old room X:%d Y:%d."), OldRoom->GridX, OldRoom->GridY));
		OldRoom->Destroy();
	}

	ARoomPlatformBase* NewRoom;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = this;

	DebugLog(FString::Printf(TEXT("%s SpawnRoom - Spanwing room actor for X:%d Y:%d."), *this->GetName(), (int32)GridCoords.X, (int32)GridCoords.Y));

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
	if (NewRoom != nullptr)
	{
		NewRoom->MyGridManager = this;
		NewRoom->RoomTemplate = *RoomTemplate;
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
}


void ARoomPlatformGridMgr::ClientUpdateRoomGridTemplate_Implementation(const FRoomGridTemplate& UpdatedTemplate)
{
	// Multicast replication but only needed on clients.
	if (GetLocalRole() < ROLE_Authority)
	{
		RoomGridTemplate = UpdatedTemplate;
	}
}
