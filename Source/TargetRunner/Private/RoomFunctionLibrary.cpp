// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomFunctionLibrary.h"
#include "RoomGridTemplate.h"
#include "RoomExitInfo.h"
#include "TRMath.h"
// for intellisense
#include "..\Public\RoomFunctionLibrary.h"

const TArray<ETRDirection> URoomFunctionLibrary::OrthogonalDirections = { ETRDirection::North, ETRDirection::East, ETRDirection::South, ETRDirection::West };

int32 URoomFunctionLibrary::GetRoomTemplateCount(const FRoomGridTemplate& RoomGridTemplate, const bool bIncludeBlackout)
{
	TArray<int32> RowNums;
	TArray<int32> ColNums;
	int32 Count = 0;
	RoomGridTemplate.Grid.GenerateKeyArray(RowNums);
	for (int32 Row : RowNums)
	{
		ColNums.Empty(RoomGridTemplate.Grid.Find(Row)->RowRooms.Num());
		RoomGridTemplate.Grid.Find(Row)->RowRooms.GenerateKeyArray(ColNums);
		for (int32 Col : ColNums)
		{
			const FRoomTemplate* Room = RoomGridTemplate.Grid.Find(Row)->RowRooms.Find(Col);
			if (Room != nullptr)
			{
				if (bIncludeBlackout || !Room->bIsBlackout) {
					Count++;
				}
			}
		}
	}
	return Count;
}

int32 URoomFunctionLibrary::GetAllRoomTemplateCoords(const FRoomGridTemplate& RoomGridTemplate, TArray<FVector2D>& RoomCoords, const bool bIncludeBlackout)
{
	TArray<int32> RowNums;
	TArray<int32> ColNums;
	RoomGridTemplate.Grid.GenerateKeyArray(RowNums);
	for (int32 RowNum : RowNums)
	{
		const FRoomGridRow* Row = RoomGridTemplate.Grid.Find(RowNum);
		ColNums.Empty(Row->RowRooms.Num());
		Row->RowRooms.GenerateKeyArray(ColNums);
		for (int32 ColNum : ColNums)
		{
			const FRoomTemplate* Room = RoomGridTemplate.Grid.Find(RowNum)->RowRooms.Find(ColNum);
			if (Room != nullptr)
			{
				if (bIncludeBlackout || !Room->bIsBlackout) {
					RoomCoords.Add(FVector2D(RowNum, ColNum));
				}
			}
		}
	}
	return RoomCoords.Num();
}


bool URoomFunctionLibrary::GetRoom(FRoomGridTemplate RoomGridTemplate, const FIntPoint& RoomCoords, FRoomTemplate& Room)
{
	FRoomTemplate* FoundRoom = GetRoom(RoomGridTemplate, RoomCoords);
	if (FoundRoom)
	{
		Room = *FoundRoom;
		return true;
	}
	return false;
}


FRoomTemplate* URoomFunctionLibrary::GetRoom(FRoomGridTemplate& RoomGridTemplate, const FIntPoint& RoomCoords)
{
	if (!RoomGridTemplate.Grid.Contains(RoomCoords.X)) { return nullptr; }
	if (!RoomGridTemplate.Grid[RoomCoords.X].RowRooms.Contains(RoomCoords.Y)) { return nullptr; }
	FRoomTemplate* Room = RoomGridTemplate.Grid.Find(RoomCoords.X)->RowRooms.Find(RoomCoords.Y);	
	return Room;
}


int32 URoomFunctionLibrary::GetRoomTemplateGridAsArrays(const FRoomGridTemplate& RoomGridTemplate, TArray<FVector2D>& RoomCoords, TArray<FRoomTemplate>& RoomTemplates, const bool bIncludeBlackout)
{
	TArray<int32> RowNums;
	TArray<int32> ColNums;
	RoomGridTemplate.Grid.GenerateKeyArray(RowNums);
	for (int32 RowNum : RowNums)
	{
		const FRoomGridRow* Row = RoomGridTemplate.Grid.Find(RowNum);
		ColNums.Empty(Row->RowRooms.Num());
		Row->RowRooms.GenerateKeyArray(ColNums);
		for (int32 ColNum : ColNums)
		{
			const FRoomTemplate* Room = RoomGridTemplate.Grid.Find(RowNum)->RowRooms.Find(ColNum);
			if (Room != nullptr)
			{
				if (bIncludeBlackout || !Room->bIsBlackout) {
					RoomCoords.Add(FVector2D(RowNum, ColNum));
					RoomTemplates.Add(*Room);
				}
			}
		}
	}
	return RoomCoords.Num();
}

FRoomExitInfo URoomFunctionLibrary::GetRoomExitInfo(const FRoomGridTemplate& RoomGridTemplate, const FIntPoint& RoomCoords)
{
	FRoomGridTemplate RGT = const_cast<FRoomGridTemplate&>(RoomGridTemplate);
	FRoomExitInfo ExitInfo;
	int32 ExitCount = 0;
	ETRDirection LastDoorOrientation = ETRDirection::North;
	ETRDirection LastNonDoorOrientation = ETRDirection::North;
	ETRDirection PreviousLastDoorOrientation = ETRDirection::North;
	ETRWallState WallState;
	FRoomTemplate* NeighborRoom = nullptr;
	FRoomTemplate* Room = GetRoom(RGT, RoomCoords);
	if (Room)
	{
		for (ETRDirection Direction : OrthogonalDirections)
		{
			WallState = GetWallState(*Room, Direction);
			if (WallState == ETRWallState::Empty)
			{
				// If our wall state is empty, get the state from the neighbor room in that direction.
				NeighborRoom = GetRoom(RGT, RoomCoords + UTRMath::DirectionToOffset(Direction));
				if (NeighborRoom) {
					// The the opposite wall's state in this neighbor room. i.e. the wall that borders our target room.
					WallState = GetWallState(*NeighborRoom, UTRMath::OppositeDirection(Direction));
				}
			}
			if (WallState == ETRWallState::Door)
			{
				ExitCount++;
				PreviousLastDoorOrientation = LastDoorOrientation;
				LastDoorOrientation = Direction;
			}
			else
			{
				LastNonDoorOrientation = Direction;
			}
		}
		// Most common
		if (ExitCount == 1) 
		{
			ExitInfo.ExitLayout = ETRRoomExitLayout::One;
			ExitInfo.Orientation = LastDoorOrientation;
		}
		// Next most common
		else if (ExitCount == 2) 
		{
			if (PreviousLastDoorOrientation == UTRMath::OppositeDirection(LastDoorOrientation)) {
				ExitInfo.ExitLayout = ETRRoomExitLayout::TwoOpposite;
				ExitInfo.Orientation = PreviousLastDoorOrientation;
			}
			else {
				ExitInfo.ExitLayout = ETRRoomExitLayout::TwoAdjacent;
				ExitInfo.Orientation = LastDoorOrientation == ETRDirection::West &&  PreviousLastDoorOrientation == ETRDirection::North ? ETRDirection::West : PreviousLastDoorOrientation;
			}			
		}
		else if (ExitCount == 3) 
		{
			ExitInfo.ExitLayout = ETRRoomExitLayout::Three;
			ExitInfo.Orientation = UTRMath::OppositeDirection(LastNonDoorOrientation);
		}
		else
		{
			ExitInfo.ExitLayout = ETRRoomExitLayout::Four;
			ExitInfo.Orientation = ETRDirection::North;
		}
	}
	return ExitInfo;
}


ETRWallState URoomFunctionLibrary::GetWallState(const FRoomTemplate& Room, const ETRDirection Direction)
{
	switch(Direction)
	{
	case ETRDirection::North :
		return Room.NorthWall;
	case ETRDirection::East :
		return Room.EastWall;
	case ETRDirection::South :
		return Room.SouthWall;
	case ETRDirection::West :
		return Room.WestWall;
	}
	return ETRWallState::Unknown;
}