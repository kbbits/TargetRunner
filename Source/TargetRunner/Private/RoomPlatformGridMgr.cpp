// Fill out your copyright notice in the Description page of Project Settings.

#include "RoomPlatformGridMgr.h"
#include "..\Public\RoomPlatformGridMgr.h"
#include "TRMath.h"
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


void ARoomPlatformGridMgr::OnGridForgeProgress(const FProgressItem ProgressItem)
{
	DebugLog(FString::Printf(TEXT("RoomPlatformGridMgr::OnGridForgeProgress: Code: %s,  %s, %.0f, %.0f"), *ProgressItem.Code.ToString(), *ProgressItem.Message, ProgressItem.CurrentProgress, ProgressItem.OfTotalProgress));
	OnGenerateProgress.Broadcast(ProgressItem);
}


// Called every frame
void ARoomPlatformGridMgr::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}


ARoomPlatformBase* ARoomPlatformGridMgr::GetRoomInGrid(const FVector2D& RoomCoords)
{
	bool bFound;
	return Cast<ARoomPlatformBase>(GetPlatformInGrid(RoomCoords, bFound));
}


ARoomPlatformBase* ARoomPlatformGridMgr::GetRoomNeighbor(const FVector2D& RoomCoords, const ETRDirection DirectionToNeighbor)
{
	return Cast<ARoomPlatformBase>(GetPlatformNeighbor(RoomCoords, DirectionToNeighbor));
}


void ARoomPlatformGridMgr::GenerateGridImpl()
{
	bool bSuccessful = false;
	// Destroy old grid, if any
	DestroyGrid();
	DebugLog(FString::Printf(TEXT("%s RoomPlatformGridMgr::GenerateGrid - Generating grid. Extents: MinX:%d MinY:%d  MaxX:%d MaxY:%d"), *this->GetName(), GridExtentMinX, GridExtentMinY, GridExtentMaxX, GridExtentMaxY));
	FRandomStream* GridStreamFound = nullptr;
	FRandomStream* ResourceStreamFound = nullptr;
	// Grab the game mode.
	ATR_GameMode* GameMode = Cast<ATR_GameMode>(GetWorld()->GetAuthGameMode());
	if (IsValid(GameMode))
	{
		// Get our random streams from game mode
		GridStreamFound = &GameMode->GetGridStream();
		ResourceStreamFound = &GameMode->GetResourceDropperStream();
	}
	else 
	{
		UE_LOG(LogTRGame, Error, TEXT("%s RoomPlatformGridMgr::GenerateGrid - Could not get GameMode. Using default rand stream."), *this->GetName());
		// Use our default streams
		GridStreamFound = &DefaultGridRandStream;
		GridStreamFound->Reset();
		ResourceStreamFound = &DefaultResourceDropperStream;
		ResourceStreamFound->Reset();
	}
	
	FRandomStream& GridRandStream = *GridStreamFound;
	FRandomStream& ResourceDropperStream = *ResourceStreamFound;
	DebugLog(FString::Printf(TEXT("RoomPlatformGridMgr::GenerateGridImpl - GridRandStream seed: %d"), GridRandStream.GetInitialSeed()));
	// Create the grid forge
	UGridForgeBase* GridForge = NewObject<UGridForgeBase>(this, GridForgeClass);
	if (IsValid(GridForge))
	{ 
		GridForge->OnGenerateGridProgressDelegate.BindUObject(this, &ARoomPlatformGridMgr::OnGridForgeProgress);
	}
	else
	{
		UE_LOG(LogTRGame, Error, TEXT("%s RoomPlatformGridMgr::GenerateGrid - Could not construct GridForge."), *this->GetName());
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
		// Use the manually specified blackout cells.
		GridForge->BlackoutCells = OverrideBlackoutCells;
	}
	else
	{
		GridForge->BlackoutCells.Empty();
	}
	RoomGridTemplate.bDoorsAtWallCenter = false;
	
	// Fill the RoomGridTemplate's grid
	GridForge->GenerateGridTemplate(GridRandStream, RoomGridTemplate, bSuccessful);

	if (bSuccessful)
	{
		DebugLog(FString::Printf(TEXT("%s RoomPlatformGridMgr::GenerateGrid - Generating grid successful."), *GetNameSafe(this)));
		StartGridCoords = RoomGridTemplate.StartCells[0];
		ExitGridCoords = RoomGridTemplate.EndCells[0];
		// Allocate resources to room templates
		UResourceDropperBase* ResourceDropper = NewObject<UResourceDropperBase>(this, ResourceDropperClass);
		if (ResourceDropper == nullptr) {
			UE_LOG(LogTRGame, Error, TEXT("%s RoomPlatformGridMgr::GenerateGrid - Could not construct ResourceDropper."), *GetNameSafe(this));
			return;
		}
		// Drop the resources
		ResourceDropper->DistributeResources(ResourceDropperStream, ResourcesToDistribute, RoomGridTemplate);
		// Drop special actors
		ResourceDropper->DistributeSpecials(ResourceDropperStream, SpecialsToDistribute, RoomGridTemplate);
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
		//ClientUpdateRoomGridTemplate(RoomGridTemplate);
	}
	GridForge->OnGenerateGridProgressDelegate.Unbind();
}


void ARoomPlatformGridMgr::DestroyGridImpl()
{
	//TArray<int32> RowNums;
	//TArray<int32> PlatformNums;
	//PlatformGridMap.GenerateKeyArray(RowNums);

	//DebugLog(FString::Printf(TEXT("Destroying %d rows."), RowNums.Num()));
	//for (int32 Row : RowNums)
	//{
	//	PlatformGridMap.Find(Row)->RowPlatforms.GenerateKeyArray(PlatformNums);
	//	for (int32 Col : PlatformNums)
	//	{
	//		APlatformBase* Platform = PlatformGridMap.Find(Row)->RowPlatforms[Col];
	//		if (IsValid(Platform))
	//		{
	//			DebugLog(FString::Printf(TEXT("Destroying room X:%d Y:%d."), Platform->GridX, Platform->GridY));
	//			Platform->Destroy();
	//		}
	//	}
	//	PlatformGridMap.Find(Row)->RowPlatforms.Empty();
	//}
	//PlatformGridMap.Empty();

	APlatformGridMgr::DestroyGridImpl();
	TArray<int32> RowNums;
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
		//ClientUpdateRoomGridTemplate(RoomGridTemplate);
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
	// Now spawn all room contents
	TArray<FVector2D> AllRoomCoords;
	bool bFound;
	URoomFunctionLibrary::GetAllRoomTemplateCoords(RoomGridTemplate, AllRoomCoords, true);
	for (FVector2D RoomCoord : AllRoomCoords)
	{
		ARoomPlatformBase* RoomPlatform = Cast<ARoomPlatformBase>(GetPlatformInGrid(RoomCoord, bFound));
		if (RoomPlatform != nullptr)
		{
			RoomPlatform->SpawnContents(); // calls SpawnResources & SpawnSpecials
		}
	}
	if (GetLocalRole() == ROLE_Authority)
	{
		// Have the clients update their grid maps
		//ClientFillGridFromExistingPlatforms();
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
		OldRoom = nullptr;
	}

	ARoomPlatformBase* NewRoom;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = this;
	ATR_GameMode* GameMode = Cast<ATR_GameMode>(GetWorld()->GetAuthGameMode());

	DebugLog(FString::Printf(TEXT("%s SpawnRoom - Spanwing room actor for X:%d Y:%d."), *this->GetName(), (int32)GridCoords.X, (int32)GridCoords.Y));

	// Create appropriate wall state arrays representing the used wall types, sized according to RoomCellSubdivision.
	int32 DoorSection;
	if (RoomGridTemplate.bDoorsAtWallCenter)
	{
		// Put the door in the center of the wall
		DoorSection = (RoomGridTemplate.RoomCellSubdivision / 2);
	}
	else
	{
		// Put the door in a random wall section
		if (IsValid(GameMode))
		{			
			DoorSection = GameMode->GetGridStream().RandRange(0, RoomCellSubdivision - 1);
		}
		else 
		{
			DoorSection = DefaultGridRandStream.RandRange(0, RoomCellSubdivision - 1);
		}
	}
	TArray<ETRWallState> DoorWall;
	for (int i = 0; i < RoomGridTemplate.RoomCellSubdivision; i++)
	{
		if (i == DoorSection) { DoorWall.Add(ETRWallState::Door); }
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
		NewRoom->bRoomTemplateSet = true;		
		if (IsValid(GameMode))
		{
			// Get our random stream from game mode
			NewRoom->PlatformRandStream.Initialize(GameMode->GetGridStream().RandRange(1, INT_MAX -1));
		}
		else
		{
			// Use our default streams
			NewRoom->PlatformRandStream.Initialize(DefaultGridRandStream.RandRange(1, INT_MAX - 1));
		}
		// TODO: Seed platform rand stream
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
			if (WallState == ETRWallState::Door && !RoomGridTemplate.bDoorsAtWallCenter)
			{
				// Put the door in a new random wall section
				if (IsValid(GameMode))
				{
					DoorSection = GameMode->GetGridStream().RandRange(0, RoomCellSubdivision - 1);
				}
				else
				{
					DoorSection = DefaultGridRandStream.RandRange(0, RoomCellSubdivision - 1);
				}
				for (int i = 0; i < RoomGridTemplate.RoomCellSubdivision; i++)
				{
					if (i == DoorSection) { DoorWall[i] = ETRWallState::Door; }
					else { DoorWall[i] = ETRWallState::Blocked; }
				}
			}
		}
		int32 FloorStateArraySize = (RoomCellSubdivision * RoomCellSubdivision);
		// Initialize the floor state array values.
		for (int32 i = 0; i < FloorStateArraySize; i++)
		{
			// Default the state to open.
			NewRoom->FloorTemplate.Add(ETRFloorState::Open);
		}
		AddPlatformToGridMap(NewRoom);
		NewRoom->GenerateRoom();
	}
}


void ARoomPlatformGridMgr::SetRoomGridTemplateData_Implementation(const FRoomGridTemplate& UpdatedTemplate, const TArray<FVector2D>& RoomCoords, const TArray<FRoomTemplate>& RoomTemplates)
{
	RoomGridTemplate = UpdatedTemplate;
	int32 CoordX;
	int32 CoordY;
	for (int i = 0; i < RoomCoords.Num(); i++)
	{
		CoordX = (int32)RoomCoords[i].X;
		CoordY = (int32)RoomCoords[i].Y;
		if (!RoomGridTemplate.Grid.Contains(CoordX))
		{
			FRoomGridRow NewRow;
			RoomGridTemplate.Grid.Add(CoordX, NewRow);
		}
		RoomGridTemplate.Grid.Find(CoordX)->RowRooms.Add(CoordY, RoomTemplates[i]);
	}
	// Multicast replication but only needed on clients.
	//if (GetLocalRole() < ROLE_Authority)
	//{
	//	RoomGridTemplate = UpdatedTemplate;
	//}
}


void ARoomPlatformGridMgr::WakeNeighborsImpl(FVector2D AroundGridCoords)
{
	const TArray<ETRDirection> OrthogonalDirections = { ETRDirection::North, ETRDirection::East, ETRDirection::South, ETRDirection::West };
	FVector2D OffsetCoords;
	ARoomPlatformBase* CurRoom = nullptr;
	ARoomPlatformBase* NeighborRoom = nullptr;
	ARoomPlatformBase* AroundRoom = GetRoomInGrid(AroundGridCoords);

	if (AroundRoom && !AroundRoom->bStasisWokeNeighbors)
	{
		// Orthogonal directions only
		for (ETRDirection Direction : OrthogonalDirections)
		{
			OffsetCoords = AroundGridCoords;
			for (int32 i = 1; i <= StasisWakeRange; i++)
			{
				CurRoom = GetRoomInGrid(OffsetCoords);
				if (CurRoom)
				{
					// Connected neighbors only
					NeighborRoom = CurRoom->GetConnectedNeighbor(Direction);
					if (NeighborRoom)
					{
						NeighborRoom->StasisWakeActors();
					}
				}
				// Move OffsetCoords to get next room over, even if there was no room here.
				OffsetCoords += UTRMath::DirectionToOffsetVector(Direction);
			}
		}
		AroundRoom->bStasisWokeNeighbors = true;
	}
}