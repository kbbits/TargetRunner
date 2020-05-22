// Fill out your copyright notice in the Description page of Project Settings.


#include "GridForgeBase.h"
#include "RoomTemplate.h"
#include "TrEnums.h"
#include "..\Public\GridForgeBase.h"

void UGridForgeBase::GenerateGridTemplate(FRoomGridTemplate& TemplateGrid, bool& Successful)
{
	Successful = false;

	TArray<ETRWallState> DoorWall; 
	DoorWall.Add(ETRWallState::WallState_Blocked);
	DoorWall.Add(ETRWallState::WallState_Open);
	DoorWall.Add(ETRWallState::WallState_Blocked);

	TArray<ETRWallState> SolidWall;
	SolidWall.Add(ETRWallState::WallState_Blocked);
	SolidWall.Add(ETRWallState::WallState_Blocked);
	SolidWall.Add(ETRWallState::WallState_Blocked);

	// Generate each cell in the grid
	FRoomTemplate Room;
	Room.WallTemplate.Append(DoorWall);
	Room.WallTemplate.Append(SolidWall);
	Room.WallTemplate.Append(SolidWall);
	Room.WallTemplate.Append(SolidWall);
	GetGridRow(TemplateGrid, 0).RowRooms.Add(0, Room);
	Room.WallTemplate.Empty();
	Room.WallTemplate.Append(SolidWall);
	Room.WallTemplate.Append(SolidWall);
	Room.WallTemplate.Append(DoorWall);
	Room.WallTemplate.Append(SolidWall);
	GetGridRow(TemplateGrid, 1).RowRooms.Add(0, Room);

	Successful = true;
}


FRoomGridRow& UGridForgeBase::GetGridRow(FRoomGridTemplate& TemplateGrid, int32 RowNum)
{
	if (!TemplateGrid.Grid.Contains(RowNum))
	{
		FRoomGridRow NewRow;
		TemplateGrid.Grid.Add(RowNum, NewRow);		
	}
	return TemplateGrid.Grid[RowNum];
}
