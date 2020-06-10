// Fill out your copyright notice in the Description page of Project Settings.


#include "GridForgeManual.h"
#include "RoomTemplate.h"
#include "TrEnums.h"

void UGridForgeManual::GenerateGridTemplate(UPARAM(ref) FRandomStream& RandStream, FRoomGridTemplate& RoomGridTemplate, bool& Successful)
{
	SetupFromRoomGridTemplate(RoomGridTemplate);

	// Don't call PickStartAndEndCells() because we are choosing them manually.
	RoomGridTemplate.StartCells.Empty();
	RoomGridTemplate.StartCells.Add(FVector2D(0, 0));
	RoomGridTemplate.EndCells.Empty();
	RoomGridTemplate.EndCells.Add(FVector2D(1, 0));

	Successful = false;

	/*TArray<ETRWallState> DoorWall;
	DoorWall.Add(ETRWallState::Blocked);
	DoorWall.Add(ETRWallState::Open);
	DoorWall.Add(ETRWallState::Blocked);

	TArray<ETRWallState> SolidWall;
	SolidWall.Add(ETRWallState::Blocked);
	SolidWall.Add(ETRWallState::Blocked);
	SolidWall.Add(ETRWallState::Blocked);

	TArray<ETRWallState> EmptyWall;
	EmptyWall.Add(ETRWallState::Empty);
	EmptyWall.Add(ETRWallState::Empty);
	EmptyWall.Add(ETRWallState::Empty);*/

	// Generate each cell in the grid
	FRoomTemplate Room;
	Room.NorthWall = ETRWallState::Door;
	Room.EastWall = ETRWallState::Blocked;
	Room.SouthWall = ETRWallState::Blocked;
	Room.WestWall = ETRWallState::Blocked;
	GetRoomRow(RoomGridTemplate, 0)->RowRooms.Add(0, Room);
	Room.NorthWall = ETRWallState::Blocked;
	Room.EastWall = ETRWallState::Blocked;
	Room.SouthWall = ETRWallState::Empty;
	Room.WestWall = ETRWallState::Blocked;
	GetRoomRow(RoomGridTemplate, 1)->RowRooms.Add(0, Room);

	Successful = true;
}