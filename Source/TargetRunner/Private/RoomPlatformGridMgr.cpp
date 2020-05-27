// Fill out your copyright notice in the Description page of Project Settings.

#include "RoomPlatformGridMgr.h"

#include "TargetRunner.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ARoomPlatformGridMgr::ARoomPlatformGridMgr()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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
	FRoomGridTemplate RoomGridTemplate;
	UGridForgeBase* GridForge = NewObject<UGridForgeBase>(this, GridForgeClass);
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
	if (!bGenerateStartEnd)
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

		FRoomGridRow* GridRow;
		FRoomTemplate* RoomTemplate;
		ARoomPlatformBase* NewRoom;
		FActorSpawnParameters SpawnParams;
		FVector2D CurCoords;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Owner = this;

		for (int32 row = GridExtentMinX; row <= GridExtentMaxX; row++)
		{
			if (RoomGridTemplate.Grid.Contains(row))
			{
				GridRow = RoomGridTemplate.Grid.Find(row);
				if (GridRow != nullptr)
				{
					CurCoords.X = row;
					for (int col = GridExtentMinY; col <= GridExtentMaxY; col++)
					{
						if (GridRow->RowRooms.Contains(col))
						{
							CurCoords.Y = col;
							RoomTemplate = GridRow->RowRooms.Find(col);
							if (RoomTemplate != nullptr)
							{
								UE_LOG(LogTRGame, Log, TEXT("%s GenerateGrid - Spanwing room actor for X:%d Y:%d."), *this->GetName(), (int32)CurCoords.X, (int32)CurCoords.Y);
								NewRoom = GetWorld()->SpawnActor<ARoomPlatformBase>(RoomClass, GetGridCellWorldTransform(CurCoords), SpawnParams);
								NewRoom->MyGridManager = this;
								NewRoom->GridX = row;
								NewRoom->GridY = col;
								// Create wall template
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
							else
							{
								UE_LOG(LogTRGame, Error, TEXT("%s GenerateGrid - Got null room at row X:%d Y:%d"), *this->GetName(), row, col);
							}
						}
						else
						{
							UE_LOG(LogTRGame, Log, TEXT("%s GenerateGrid - Room not found in row X:%d Y:%d"), *this->GetName(), row, col);
						}
					}
				}
			}
			else 
			{
				UE_LOG(LogTRGame, Log, TEXT("%s GenerateGrid - Row %d not found in grid."), *this->GetName(), row);
			}
		}
	} 
	else
	{
		UE_LOG(LogTRGame, Warning, TEXT("Grid generation failed with grid forge: %s"), *GetNameSafe(GridForge));
	}
	GridForge->EmptyGridTemplateCells();
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
}
