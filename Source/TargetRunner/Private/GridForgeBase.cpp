// Fill out your copyright notice in the Description page of Project Settings.


#include "GridForgeBase.h"
#include "RoomTemplate.h"
#include "TrEnums.h"
#include "..\Public\GridForgeBase.h"

UGridForgeBase::UGridForgeBase()
{
	BlackoutCellCenterBias = 0.75f;
	BlackoutCellXYBias = 0.5f;
	BlackoutDensityFactor = 1.0f;
	BlackoutSearchDistance = 2;
}

void UGridForgeBase::GenerateGridTemplate(UPARAM(ref) FRandomStream& RandStream, FRoomGridTemplate& TemplateGrid, bool& Successful)
{
	SetRoomGridTemplate(TemplateGrid);
	
	// Don't call PickStartAndEndCells() because we are choosing them manually in this base class.
	TemplateGrid.StartCells.Empty();
	TemplateGrid.StartCells.Add(FVector2D(0, 0));
	TemplateGrid.EndCells.Empty();
	TemplateGrid.EndCells.Add(FVector2D(1, 0));

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
	GetRoomRow(TemplateGrid, 0)->RowRooms.Add(0, Room);
	Room.NorthWall = ETRWallState::Blocked;
	Room.EastWall = ETRWallState::Blocked;
	Room.SouthWall = ETRWallState::Empty;
	Room.WestWall = ETRWallState::Blocked;
	GetRoomRow(TemplateGrid, 1)->RowRooms.Add(0, Room);

	Successful = true;
}


// Default implementation does nothing but return true.
void UGridForgeBase::GenerateGridTemplateCells(UPARAM(ref) FRandomStream& RandStream, bool& Successful)
{
	Successful = true;
}


bool UGridForgeBase::GetRoomRef( FRoomGridTemplate& RoomGridTemplate, const FVector2D& Coords, FRoomTemplate& Room)
{
	if (!RoomGridTemplate.Grid.Contains(Coords.X)) { return false; }
	if (!RoomGridTemplate.Grid[Coords.X].RowRooms.Contains(Coords.Y)) { return false; }
	Room = *RoomGridTemplate.Grid.Find(Coords.X)->RowRooms.Find(Coords.Y);
	return true;
}


void UGridForgeBase::SetRoomGridTemplate(FRoomGridTemplate& GridTemplate)
{
	WorkingRoomGridTemplate = &GridTemplate;
}


void UGridForgeBase::GenerateBlackoutCells(FRandomStream& RandStream)
{	
	DebugLog(TEXT("GenerateBlackoutCells"));
	UGridTemplateCell* NewCell = nullptr;
	if (BlackoutCells.Num() > 0)
	{
		DebugLog(TEXT("Using pre-set blackout cells."));
		// If they are already set (ex: came from grid manager's override blackout cells), just create the cells.
		for (FVector2D Coords : BlackoutCells)
		{
			NewCell = GetOrCreateCell(Coords);
			NewCell->CellState = ETRGridCellState::Blocked;
		}
	}
	else
	{
		float WidthX = static_cast<float>(WorkingRoomGridTemplate->GridExtentMaxX - WorkingRoomGridTemplate->GridExtentMinX) + 1.0f;
		float WidthY = static_cast<float>(WorkingRoomGridTemplate->GridExtentMaxY - WorkingRoomGridTemplate->GridExtentMinY) + 1.0f;
		int32 BlackoutCount = 0;
		bool bStopBlackoutSearch = false;
		if (WidthX > 3 && WidthY > 3)
		{
			BlackoutCount = static_cast<int32>(FMath::RoundHalfToZero(FMath::Sqrt(WidthX * WidthY)) + RandStream.RandRange(-1, 0));
			if (WidthX > 5 && WidthY > 5)
			{
				BlackoutCount += RandStream.RandRange(0, (int32)FMath::RoundHalfToZero((WidthX * WidthY) / 10.0f));
			}
		}
		BlackoutCount = (int32)((float)BlackoutCount * BlackoutDensityFactor);
		DebugLog(FString::Printf(TEXT("Generating blackout cells: %d"), BlackoutCount));
		if (BlackoutCount > 0)
		{
			FVector2D BlackoutCoords;
			int32 PickFails = 0;
			while (BlackoutCount > 0 && !bStopBlackoutSearch && (PickFails <= BlackoutCount * 3))
			{
				DebugLog(FString::Printf(TEXT("      Trying pick. Remaining %d. Fails: %d"), BlackoutCount, PickFails));
				if (PickBlackoutCoords(RandStream, BlackoutCoords))
				{
					DebugLog(FString::Printf(TEXT("      Looking at cell. X:%d Y:%d"), (int32)BlackoutCoords.X, (int32)BlackoutCoords.Y));
					if (BlackoutCells.Contains(BlackoutCoords)) 
					{ 
						PickFails++;
						continue; 
					}
					// Get neighbors and check rules to prevent blockages.
					TMap<ETRDirection, UGridTemplateCell*> Neighbors;
					TArray<FVector2D> IgnoredCells;
					UGridTemplateCell* CellOne = nullptr;
					UGridTemplateCell* CellTwo = nullptr;
					bool bOpposedNS, bOpposedEW;
					// Check our neighbors
					if (HasOpposingBlockedNeighbors(BlackoutCoords, BlackoutSearchDistance, IgnoredCells, Neighbors, bOpposedNS, bOpposedEW))
					{
						PickFails++;
						continue; // pick again
						//if (bOpposedNS)
						//{
						//}
						//if (bOpposedEW)
						//{
						//}
					}
					else
					{
						BlackoutCoords = FVector2D((int32)BlackoutCoords.X, (int32)BlackoutCoords.Y);
						BlackoutCells.Add(BlackoutCoords);
						DebugLog(FString::Printf(TEXT("Added blackout cell X:%f Y:%f"), BlackoutCoords.X, BlackoutCoords.Y));
						NewCell = GetOrCreateCell(BlackoutCoords);
						NewCell->CellState = ETRGridCellState::Blocked;
						--BlackoutCount;
						PickFails = 0;
					}
				}
				else
				{
					PickFails++;
				}
			}
		}
		DebugLog(FString::Printf(TEXT("Blackout cells generated: %d"), BlackoutCells.Num()));
		for (FVector2D dbgBlackout : BlackoutCells)
		{
			DebugLog(FString::Printf(TEXT("  X:%f Y:%f"), dbgBlackout.X, dbgBlackout.Y));
		}
	}
}


FRoomTemplate* UGridForgeBase::GetOrCreateRoom(FRoomGridTemplate& RoomGridTemplate, const FVector2D Coords, bool& bFound)
{
	bFound = false;
	FRoomGridRow* Row = GetRoomRow(RoomGridTemplate, Coords.X);
	if (Row == nullptr) { return false; }
	if (!Row->RowRooms.Contains(Coords.Y))
	{
		FRoomTemplate NewRoom;
		Row->RowRooms.Add(Coords.Y, NewRoom);
	}
	FRoomTemplate* FoundRoom = Row->RowRooms.Find(Coords.Y);
	if (FoundRoom != nullptr) { bFound = true; }
	return FoundRoom;
}


FRoomTemplate* UGridForgeBase::GetRoom(FRoomGridTemplate& RoomGridTemplate, const FVector2D& Coords, bool& bFound)
{
	bFound = false;
	if (!RoomGridTemplate.Grid.Contains(Coords.X)) { return nullptr; }
	if (!RoomGridTemplate.Grid[Coords.X].RowRooms.Contains(Coords.Y)) { return nullptr; }
	FRoomTemplate* Room = RoomGridTemplate.Grid.Find(Coords.X)->RowRooms.Find(Coords.Y);
	if (Room != nullptr) { bFound = true; }
	return Room;
}


FRoomGridRow* UGridForgeBase::GetRoomRow(FRoomGridTemplate& RoomGridTemplate, const int32 RowNum)
{
	if (!RoomGridTemplate.Grid.Contains(RowNum))
	{
		FRoomGridRow NewRow;
		RoomGridTemplate.Grid.Add(RowNum, NewRow);
	}
	return RoomGridTemplate.Grid.Find(RowNum);
}


FRoomTemplate* UGridForgeBase::GetRoomNeighbor(FRoomGridTemplate& RoomGridTemplate, const FVector2D& Coords, const ETRDirection Direction)
{
	FRoomTemplate* Room = nullptr;
	bool bFound = false;
	switch (Direction)
	{
	case ETRDirection::North:
		Room = GetRoom(RoomGridTemplate, Coords + FVector2D(1.0f, 0.0f), bFound);
		break;
	case ETRDirection::East:
		Room = GetRoom(RoomGridTemplate, Coords + FVector2D(0.0f, 1.0f), bFound);
		break;
	case ETRDirection::South:
		Room = GetRoom(RoomGridTemplate, Coords + FVector2D(-1.0f, 0.0f), bFound);
		break;
	case ETRDirection::West:
		Room = GetRoom(RoomGridTemplate, Coords + FVector2D(0.0f, -1.0f), bFound);
	}
	if (!bFound) { return nullptr; }
	return Room;
}


ETRWallState UGridForgeBase::GetWallStateFromNeighbor(const ETRWallState NeighborState, const bool bConnectedToNeighbor)
{
	ETRWallState RoomWallState;
	if (bConnectedToNeighbor)
	{
		RoomWallState = NeighborState == ETRWallState::Empty ? ETRWallState::Door : ETRWallState::Empty;
	}
	else
	{
		RoomWallState = NeighborState == ETRWallState::Empty ? ETRWallState::Blocked : ETRWallState::Empty;
	}
	return RoomWallState;
}


void UGridForgeBase::TranslateCellGridToRoomGrid(UPARAM(ref)FRandomStream& RandStream)
{
	FRoomGridTemplate& RoomGridTemplate = *WorkingRoomGridTemplate;
	TArray<UGridTemplateCellRow*> Rows;
	TArray<UGridTemplateCell*> CellsToTranslate;
	FVector2D CurCoords;
	UGridTemplateCell* CurCell;
	FRoomTemplate CurRoom;
	bool bFound;

	CurCoords = RoomGridTemplate.StartCells[0];
	CurCell = GetCell(CurCoords, bFound);

	if (CurCell == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("GridForgeBase - TranslatCellGridToRoomGrid found null start cell X:%d Y:%d"), (int32)CurCoords.X, (int32)CurCoords.Y);
		return;
	}
	
	// Add the start cell to our list of cells to translate
	CellsToTranslate.Add(CurCell);
	while (CellsToTranslate.Num() > 0)
	{
		CurCell = CellsToTranslate[0];
		CellsToTranslate.RemoveAt(0);
		TranslateCellToRoom(RandStream, CurCell, RoomGridTemplate);

		// Traverse our tree.
		// Add the connected cells to our list of cells to translate. But only if they haven't been translated already (bOnPath=true) and they aren't in the list already
		for (FVector2D ConnectedCoords : CurCell->ConnectedCells)
		{
			UGridTemplateCell* ConnectedCell = GetCell(ConnectedCoords, bFound);
			if (bFound && ConnectedCell != nullptr && !ConnectedCell->bOnPath && !CellsToTranslate.Contains(ConnectedCell))
			{
				CellsToTranslate.Add(ConnectedCell);
				DebugLog(FString::Printf(TEXT("GridForgeBase - Added cell to translate list X:%d Y:%d"), ConnectedCell->X, ConnectedCell->Y));
			}
		}
	}
	// Rooms for blackout cells
	for (FVector2D BlackoutCoords : BlackoutCells)
	{
		CurCell = GetCell(BlackoutCoords, bFound);
		if (CurCell != nullptr) { TranslateCellToRoom(RandStream, CurCell, RoomGridTemplate); }
	}
}


void UGridForgeBase::TranslateCellToRoom(UPARAM(ref)FRandomStream& RandStream, UGridTemplateCell* Cell, FRoomGridTemplate& RoomGridTemplate)
{
	FRoomTemplate* Room;
	FRoomTemplate* TmpRoom;
	FVector2D RoomCoords(Cell->X, Cell->Y);
	ETRWallState NeighborWallState;	
	bool bConnected;
	bool bFound;

	DebugLog(FString::Printf(TEXT("GridForgeBase - TranslateCellToRoom X:%d Y:%d"), Cell->X, Cell->Y));

	if (Cell->bOnPath)
	{
		DebugLog(TEXT("GridForgeBase - Cell already on path, so room is already created. Returning."));
		return;
	}
	
	Room = GetOrCreateRoom(RoomGridTemplate, RoomCoords, bFound);
	if (bFound && Room != nullptr)
	{
		if (BlackoutCells.Contains(RoomCoords))
		{
			Room->NorthWall = ETRWallState::Empty;
			Room->EastWall = ETRWallState::Empty;
			Room->SouthWall = ETRWallState::Empty;
			Room->WestWall = ETRWallState::Empty;
			Room->bIsBlackout = true;
		}
		else
		{
			Room->bIsBlackout = false;
			// Determine wall state
			// North	
			TmpRoom = GetRoomNeighbor(RoomGridTemplate, RoomCoords, ETRDirection::North);
			bConnected = Cell->ConnectedCells.Contains(RoomCoords + FVector2D(1, 0));
			NeighborWallState = ETRWallState::Empty;
			if (TmpRoom != nullptr) { NeighborWallState = TmpRoom->SouthWall; }
			else { DebugLog(TEXT("No north neighbor found.")); }
			Room->NorthWall = GetWallStateFromNeighbor(NeighborWallState, bConnected);

			// East
			TmpRoom = GetRoomNeighbor(RoomGridTemplate, RoomCoords, ETRDirection::East);
			bConnected = Cell->ConnectedCells.Contains(RoomCoords + FVector2D(0, 1));
			NeighborWallState = ETRWallState::Empty;
			if (TmpRoom != nullptr) { NeighborWallState = TmpRoom->WestWall; }
			else { DebugLog(TEXT("No east neighbor found.")); }
			Room->EastWall = GetWallStateFromNeighbor(NeighborWallState, bConnected);

			// South 
			TmpRoom = GetRoomNeighbor(RoomGridTemplate, RoomCoords, ETRDirection::South);
			bConnected = Cell->ConnectedCells.Contains(RoomCoords + FVector2D(-1, 0));
			NeighborWallState = ETRWallState::Empty;
			if (TmpRoom != nullptr) { NeighborWallState = TmpRoom->NorthWall; }
			else { DebugLog(TEXT("No south neighbor found.")); }
			Room->SouthWall = GetWallStateFromNeighbor(NeighborWallState, bConnected);

			// West
			TmpRoom = GetRoomNeighbor(RoomGridTemplate, RoomCoords, ETRDirection::West);
			bConnected = Cell->ConnectedCells.Contains(RoomCoords + FVector2D(0, -1));
			NeighborWallState = ETRWallState::Empty;
			if (TmpRoom != nullptr) { NeighborWallState = TmpRoom->EastWall; }
			else { DebugLog(TEXT("No west neighbor found.")); }
			Room->WestWall = GetWallStateFromNeighbor(NeighborWallState, bConnected);
		}
		// Mark the cell as on the path, so we don't create a room for it again.
		Cell->bOnPath = true;
		DebugLog(FString::Printf(TEXT("Cell X:%d Y:%d translated to %s "), Cell->X, Cell->Y, *RoomToString(*Room)));
	}
}


// Returns nullptr if no such cell exists already.
UGridTemplateCell* UGridForgeBase::GetCell(const FVector2D& Coords, bool& bFound)
{
	int32 X = static_cast<int32>(Coords.X);
	int32 Y = static_cast<int32>(Coords.Y);
	if (GridTemplateCells.Contains(X))
	{
		if (GridTemplateCells[X]->RowCells.Contains(Y))
		{
			bFound = true;
			UGridTemplateCellRow* Row = GridTemplateCells[X];
			return Row->RowCells[Y];
		}
	}
	bFound = false;
	return nullptr;
}

// Adds a placeholder blocked cell for neighbors outside grid extents.
void UGridForgeBase::GetCellNeighbors(const FVector2D& Coords, TMap<ETRDirection, UGridTemplateCell*>& NeighborCells)
{
	UGridTemplateCell* Cell = nullptr;
	bool bFound;
	NeighborCells.Empty(8);
	Cell = GetCell(Coords + FVector2D(1, 0), bFound);
	if (Cell != nullptr) { NeighborCells.Add(ETRDirection::North, Cell); }
	else 
	{
		if (!IsInGrid(Coords + FVector2D(1, 0))) 
		{
			Cell = NewObject<UGridTemplateCell>(this);
			Cell->CellState = ETRGridCellState::Blocked;
			Cell->X = (int32)Coords.X + 1;
			Cell->Y = (int32)Coords.Y;
		}
		NeighborCells.Add(ETRDirection::North, Cell);
	}
	
	Cell = GetCell(Coords + FVector2D(0, 1), bFound);
	if (Cell != nullptr) { NeighborCells.Add(ETRDirection::East, Cell); }
	else 
	{
		if (!IsInGrid(Coords + FVector2D(0, 1)))
		{
			Cell = NewObject<UGridTemplateCell>(this);
			Cell->CellState = ETRGridCellState::Blocked;
			Cell->X = (int32)Coords.X;
			Cell->Y = (int32)Coords.Y + 1;
		}
		NeighborCells.Add(ETRDirection::East, Cell);
	}

	Cell = GetCell(Coords + FVector2D(-1, 0), bFound);
	if (Cell != nullptr) { NeighborCells.Add(ETRDirection::South, Cell); }
	else 
	{
		if (!IsInGrid(Coords + FVector2D(-1, 0)))
		{
			Cell = NewObject<UGridTemplateCell>(this);
			Cell->CellState = ETRGridCellState::Blocked;
			Cell->X = (int32)Coords.X - 1;
			Cell->Y = (int32)Coords.Y;
		}
		NeighborCells.Add(ETRDirection::South, Cell);
	}
	
	Cell = GetCell(Coords + FVector2D(0, -1), bFound);
	if (Cell != nullptr) { NeighborCells.Add(ETRDirection::West, Cell); }
	else 
	{
		if (!IsInGrid(Coords + FVector2D(0, -1)))
		{
			Cell = NewObject<UGridTemplateCell>(this);
			Cell->CellState = ETRGridCellState::Blocked;
			Cell->X = (int32)Coords.X;
			Cell->Y = (int32)Coords.Y - 1;
		}
		NeighborCells.Add(ETRDirection::West, Cell);
	}

	// Diagonals
	Cell = GetCell(Coords + FVector2D(1, -1), bFound);
	if (Cell != nullptr) { NeighborCells.Add(ETRDirection::NorthWest, Cell); }
	else
	{
		if (!IsInGrid(Coords + FVector2D(1, -1)))
		{
			Cell = NewObject<UGridTemplateCell>(this);
			Cell->CellState = ETRGridCellState::Blocked;
			Cell->X = (int32)Coords.X + 1;
			Cell->Y = (int32)Coords.Y - 1;
		}
		NeighborCells.Add(ETRDirection::NorthWest, Cell);
	}

	Cell = GetCell(Coords + FVector2D(1, 1), bFound);
	if (Cell != nullptr) { NeighborCells.Add(ETRDirection::NorthEast, Cell); }
	else
	{
		if (!IsInGrid(Coords + FVector2D(1, 1)))
		{
			Cell = NewObject<UGridTemplateCell>(this);
			Cell->CellState = ETRGridCellState::Blocked;
			Cell->X = (int32)Coords.X + 1;
			Cell->Y = (int32)Coords.Y + 1;
		}
		NeighborCells.Add(ETRDirection::NorthEast, Cell);
	}

	Cell = GetCell(Coords + FVector2D(-1, 1), bFound);
	if (Cell != nullptr) { NeighborCells.Add(ETRDirection::SouthEast, Cell); }
	else
	{
		if (!IsInGrid(Coords + FVector2D(-1, 1)))
		{
			Cell = NewObject<UGridTemplateCell>(this);
			Cell->CellState = ETRGridCellState::Blocked;
			Cell->X = (int32)Coords.X - 1;
			Cell->Y = (int32)Coords.Y + 1;
		}
		NeighborCells.Add(ETRDirection::SouthEast, Cell);
	}

	Cell = GetCell(Coords + FVector2D(-1, -1), bFound);
	if (Cell != nullptr) { NeighborCells.Add(ETRDirection::SouthWest, Cell); }
	else
	{
		if (!IsInGrid(Coords + FVector2D(-1, -1)))
		{
			Cell = NewObject<UGridTemplateCell>(this);
			Cell->CellState = ETRGridCellState::Blocked;
			Cell->X = (int32)Coords.X - 1;
			Cell->Y = (int32)Coords.Y - 1;
		}
		NeighborCells.Add(ETRDirection::SouthWest, Cell);
	}
}

void UGridForgeBase::GetCellNeighbors(const UGridTemplateCell& Cell, TMap<ETRDirection, UGridTemplateCell*>& NeighborCells)
{
	GetCellNeighbors(FVector2D(Cell.X, Cell.Y), NeighborCells);
}

bool UGridForgeBase::HasOpposingBlockedNeighbors(const FVector2D& Coords, const int32 Distance, const TArray<FVector2D>& IgnoredCells, TMap<ETRDirection, UGridTemplateCell*>& Neighbors, bool& bOpposedNS, bool& bOpposedEW)
{
	
	FString LogIndent;
	for (int32 i = 0; i < 10 - Distance; i++) { LogIndent += FString(TEXT(" ")); }
	//DebugLog(FString::Printf(TEXT("%sHasOpposingBlockedNeighbors checking X:%d Y:%d"), *LogIndent, (int32)Coords.X, (int32)Coords.Y));
	// If outside the grid, we're blocking at least one direction NS or EW
	if (!IsInGrid(Coords))
	{
		bOpposedNS = int32(Coords.Y) < WorkingRoomGridTemplate->GridExtentMinY || int32(Coords.Y) > WorkingRoomGridTemplate->GridExtentMaxY ||
			WorkingRoomGridTemplate->GridExtentMinX - (int32)Coords.X > 1 || (int32)Coords.X - WorkingRoomGridTemplate->GridExtentMaxX > 1;
		bOpposedEW = int32(Coords.X) < WorkingRoomGridTemplate->GridExtentMinX || int32(Coords.X) > WorkingRoomGridTemplate->GridExtentMaxX ||
			WorkingRoomGridTemplate->GridExtentMinY - (int32)Coords.Y > 1 || (int32)Coords.Y - WorkingRoomGridTemplate->GridExtentMaxY > 1;
		//DebugLog(FString::Printf(TEXT("%sOut of grid is blocked for X:%d Y:%d - NS:%s EW: %s"), *LogIndent, (int32)Coords.X, (int32)Coords.Y, (bOpposedNS ? TEXT("true") : TEXT("false")), (bOpposedEW ? TEXT("true") : TEXT("false"))));
		return true;
	}

	bOpposedNS = false;
	bOpposedEW = false;
	TArray<UGridTemplateCell*> GroupOne;
	TArray<UGridTemplateCell*> GroupTwo;
	
	GetCellNeighbors(Coords, Neighbors);
	if (Neighbors[ETRDirection::NorthWest] != nullptr && Neighbors[ETRDirection::NorthWest]->CellState == ETRGridCellState::Blocked) {
		GroupOne.Add(Neighbors[ETRDirection::NorthWest]); }
	if (Neighbors[ETRDirection::North] != nullptr && Neighbors[ETRDirection::North]->CellState == ETRGridCellState::Blocked) { 
		GroupOne.Add(Neighbors[ETRDirection::North]); }
	if (Neighbors[ETRDirection::NorthEast] != nullptr && Neighbors[ETRDirection::NorthEast]->CellState == ETRGridCellState::Blocked) {
		GroupOne.Add(Neighbors[ETRDirection::NorthEast]); }
	if (Neighbors[ETRDirection::SouthWest] != nullptr && Neighbors[ETRDirection::SouthWest]->CellState == ETRGridCellState::Blocked) {
		GroupTwo.Add(Neighbors[ETRDirection::SouthWest]); }
	if (Neighbors[ETRDirection::South] != nullptr && Neighbors[ETRDirection::South]->CellState == ETRGridCellState::Blocked) {
		GroupTwo.Add(Neighbors[ETRDirection::South]); }
	if (Neighbors[ETRDirection::SouthEast] != nullptr && Neighbors[ETRDirection::SouthEast]->CellState == ETRGridCellState::Blocked) {
		GroupTwo.Add(Neighbors[ETRDirection::SouthEast]); }
	if (GroupOne.Num() > 0 && GroupTwo.Num() > 0)
	{
		if (Distance < 1)
		{
			bOpposedNS = true;
		}
		else
		{
			TMap<ETRDirection, UGridTemplateCell*> NeighborsNeighbors;
			TArray<FVector2D> NeighborIgnoredCells;
			bool bNOpposedNS;
			bool bNOpposedEW;
			bool bFoundUnblockedOne = false;
			bool bFoundUnblockedTwo = false;
			NeighborIgnoredCells.Append(IgnoredCells);
			NeighborIgnoredCells.Add(Coords);
			for (UGridTemplateCell* NeighborCell : GroupOne)
			{
				if (!IgnoredCells.Contains(CellToCoords(NeighborCell)))
				{
					NeighborIgnoredCells.Add(CellToCoords(NeighborCell));
					if (!HasOpposingBlockedNeighbors(CellToCoords(NeighborCell), Distance - 1, NeighborIgnoredCells, NeighborsNeighbors, bNOpposedNS, bNOpposedEW))
					{
						bFoundUnblockedOne = true;
						break;
					}
				}
			}
			if (bFoundUnblockedOne)
			{
				for (UGridTemplateCell* NeighborCell : GroupTwo)
				{
					if (!IgnoredCells.Contains(CellToCoords(NeighborCell)))
					{
						NeighborIgnoredCells.Add(CellToCoords(NeighborCell));
						if (!HasOpposingBlockedNeighbors(CellToCoords(NeighborCell), Distance - 1, NeighborIgnoredCells, NeighborsNeighbors, bNOpposedNS, bNOpposedEW))
						{
							bFoundUnblockedTwo = true;
							break;
						}
					}
				}
			}
			bOpposedNS = !(bFoundUnblockedOne && bFoundUnblockedTwo);
		}
	}

	if (Neighbors[ETRDirection::SouthWest] != nullptr && Neighbors[ETRDirection::SouthWest]->CellState == ETRGridCellState::Blocked) { 
		GroupOne.Add(Neighbors[ETRDirection::SouthWest]); }
	if (Neighbors[ETRDirection::West] != nullptr && Neighbors[ETRDirection::West]->CellState == ETRGridCellState::Blocked) {
		GroupOne.Add(Neighbors[ETRDirection::West]); }
	if (Neighbors[ETRDirection::NorthWest] != nullptr && Neighbors[ETRDirection::NorthWest]->CellState == ETRGridCellState::Blocked) {
		GroupOne.Add(Neighbors[ETRDirection::NorthWest]); }
	if (Neighbors[ETRDirection::NorthEast] != nullptr && Neighbors[ETRDirection::NorthEast]->CellState == ETRGridCellState::Blocked) {
		GroupTwo.Add(Neighbors[ETRDirection::NorthEast]); }
	if (Neighbors[ETRDirection::East] != nullptr && Neighbors[ETRDirection::East]->CellState == ETRGridCellState::Blocked) {
		GroupTwo.Add(Neighbors[ETRDirection::East]); }
	if (Neighbors[ETRDirection::SouthEast] != nullptr && Neighbors[ETRDirection::SouthEast]->CellState == ETRGridCellState::Blocked) {
		GroupTwo.Add(Neighbors[ETRDirection::SouthEast]); }
	if (GroupOne.Num() > 0 && GroupTwo.Num() > 0)
	{
		if (Distance < 1)
		{
			bOpposedEW = true;
		}
		else
		{
			TMap<ETRDirection, UGridTemplateCell*> NeighborsNeighbors;
			TArray<FVector2D> NeighborIgnoredCells;
			bool bNOpposedNS;
			bool bNOpposedEW;
			bool bFoundUnblockedOne = false;
			bool bFoundUnblockedTwo = false;
			NeighborIgnoredCells.Append(IgnoredCells);
			NeighborIgnoredCells.Add(Coords);
			for (UGridTemplateCell* NeighborCell : GroupOne)
			{
				if (!IgnoredCells.Contains(CellToCoords(NeighborCell)))
				{
					NeighborIgnoredCells.Add(CellToCoords(NeighborCell));
					if (!HasOpposingBlockedNeighbors(CellToCoords(NeighborCell), Distance - 1, NeighborIgnoredCells, NeighborsNeighbors, bNOpposedNS, bNOpposedEW))
					{
						bFoundUnblockedOne = true;
						break;
					}
				}
			}
			if (bFoundUnblockedOne)
			{
				for (UGridTemplateCell* NeighborCell : GroupTwo)
				{
					if (!IgnoredCells.Contains(CellToCoords(NeighborCell)))
					{
						NeighborIgnoredCells.Add(CellToCoords(NeighborCell));
						if (!HasOpposingBlockedNeighbors(CellToCoords(NeighborCell), Distance - 1, NeighborIgnoredCells, NeighborsNeighbors, bNOpposedNS, bNOpposedEW))
						{
							bFoundUnblockedTwo = true;
							break;
						}
					}
				}
			}
			bOpposedEW = !(bFoundUnblockedOne && bFoundUnblockedTwo);
		}
	}

	//DebugLog(FString::Printf(TEXT("%sHasOpposingBlockedNeighbors for X:%d Y:%d - NS:%s EW: %s"), *LogIndent, (int32)Coords.X, (int32)Coords.Y, (bOpposedNS ? TEXT("true") : TEXT("false")), (bOpposedEW ? TEXT("true") : TEXT("false"))));
	return (bOpposedNS || bOpposedEW);
}


UGridTemplateCell* UGridForgeBase::GetOrCreateCellXY(const int32 X, const int32 Y)
{
	// We need our working grid 
	if (WorkingRoomGridTemplate == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("%s - GetOrCreateCellXY - WorkingRoomGridTemplate is null."));
		return nullptr;
	}
	// If outside grid, return nullptr
	if (X > WorkingRoomGridTemplate->GridExtentMaxX || X < WorkingRoomGridTemplate->GridExtentMinX || Y > WorkingRoomGridTemplate->GridExtentMaxY || Y < WorkingRoomGridTemplate->GridExtentMinY)
	{
		return nullptr;
	}
	// If the grid doesn't have this row yet, create the row.
	if (!GridTemplateCells.Contains(X))
	{
		UGridTemplateCellRow* NewRow = NewObject<UGridTemplateCellRow>(this);
		GridTemplateCells.Add(X, NewRow);
	}
	// If the cell isn't in the row yet, insert a new default cell.
	UGridTemplateCellRow* CurRow = GridTemplateCells[X];
	if (!CurRow->RowCells.Contains(Y))
	{
		UGridTemplateCell* NewCell = NewObject<UGridTemplateCell>(this);
		NewCell->X = X;
		NewCell->Y = Y;
		if (BlackoutCells.Contains(FVector2D(X, Y)))
		{
			NewCell->CellState = ETRGridCellState::Blocked;
			DebugLog(FString::Printf(TEXT("  Coords in blackout list. Cell will be blocked.")));
		}
		else
		{
			NewCell->CellState = ETRGridCellState::Open;
		}
		CurRow->RowCells.Add(Y, NewCell);
		FString StateName = UEnum::GetValueAsString<ETRGridCellState>(NewCell->CellState);
		DebugLog(FString::Printf(TEXT("GridForgeBase - Created new cell X:%d Y:%d State:%s"), NewCell->X, NewCell->Y, *StateName));
	}
	return CurRow->RowCells[Y];
}


UGridTemplateCell* UGridForgeBase::GetOrCreateCell(const FVector2D& Coords)
{
	return GetOrCreateCellXY(Coords.X, Coords.Y);
}


UGridTemplateCell* UGridForgeBase::GetOrCreateCellNeighbor(const int32 X, const int32 Y, const ETRDirection Direction)
{
	UGridTemplateCell* Cell = nullptr;
	switch (Direction)
	{
	case ETRDirection::North :
		Cell = GetOrCreateCellXY(X + 1, Y);
		break;
	case ETRDirection::East :
		Cell = GetOrCreateCellXY(X, Y + 1);
		break;
	case ETRDirection::South :
		Cell = GetOrCreateCellXY(X - 1, Y);
		break;
	case ETRDirection::West :
		Cell = GetOrCreateCellXY(X, Y - 1);
	}
	return Cell;
}


void UGridForgeBase::GetOrCreateCellNeighbors(const int32 X, const int32 Y, TArray<UGridTemplateCell*>& NeighborCells)
{
	NeighborCells.Empty(4);
	UGridTemplateCell* Cell;
	Cell = GetOrCreateCellNeighbor(X, Y, ETRDirection::North);
	if (Cell != nullptr) { NeighborCells.Add(Cell); }
	Cell = GetOrCreateCellNeighbor(X, Y, ETRDirection::East);
	if (Cell != nullptr) { NeighborCells.Add(Cell); }
	Cell = GetOrCreateCellNeighbor(X, Y, ETRDirection::South);
	if (Cell != nullptr) { NeighborCells.Add(Cell); }
	Cell = GetOrCreateCellNeighbor(X, Y, ETRDirection::West);
	if (Cell != nullptr) { NeighborCells.Add(Cell); }
}


void UGridForgeBase::GetUnflaggedCellNeighbors(const int32 X, const int32 Y, TArray<UGridTemplateCell*>& NeighborCells, const bool bIncludeBlocked)
{
	NeighborCells.Empty(4);
	TArray<UGridTemplateCell*> TmpNeighbors;
	GetOrCreateCellNeighbors(X, Y, TmpNeighbors);
	for (UGridTemplateCell* Cell : TmpNeighbors)
	{
		if (Cell != nullptr && !Cell->bFlagged) 
		{ 
			if (bIncludeBlocked || Cell->CellState != ETRGridCellState::Blocked)
			{
				NeighborCells.Add(Cell);
			}
		}
	}
	DebugLog(FString::Printf(TEXT("GridForgeBase - GetUnflaggedCellNeighbors found %d neighbors of cell X:%d Y:%d"), NeighborCells.Num(), X, Y));
	TmpNeighbors.Empty();
}


void UGridForgeBase::EmptyGridTemplateCells()
{
	TArray<UGridTemplateCellRow*> Rows;
	GridTemplateCells.GenerateValueArray(Rows);
	for (UGridTemplateCellRow* Row : Rows)
	{
		Row->RowCells.Empty();
	}
	Rows.Empty();
	GridTemplateCells.Empty();
	BlackoutCells.Empty();
}


void UGridForgeBase::PickStartAndEndCells(UPARAM(ref) FRandomStream& RandStream)
{
	FRoomGridTemplate& RoomGridTemplate = *WorkingRoomGridTemplate;
	int32 WidthX = (RoomGridTemplate.GridExtentMaxX - RoomGridTemplate.GridExtentMinX) + 1;
	int32 WidthY = (RoomGridTemplate.GridExtentMaxY - RoomGridTemplate.GridExtentMinY) + 1;
	int32 PadX = 0;
	int32 PadY = 0;
	int32 RemainingCellCount = 0;
	int32 UnavailableCellCount = 0;
	TArray<bool> PotentialCells;	
	
	// General level progress is south -> north. So start position will be along X = GridExtentMinX row + padding.
	// End cell will be along X = GridExtentMaxX row - padding.
	if (RoomGridTemplate.StartCells.Num() == 0)
	{	
		DebugLog(TEXT("GridForgePrim - Picking start cell."));
		// A percent chance to pad along grid extents.
		PadX = WidthX > 5 ? WidthX / 5 : 0;
		PadY = WidthY / 5;
		if (PadX > 0) { PadX = RandStream.FRandRange(0.0f, 1.0f) < 0.75f ? RandStream.RandRange(1, PadX) : 0; }
		if (PadY > 0) { PadY = RandStream.FRandRange(0.0f, 1.0f) < 0.75f ? RandStream.RandRange(1, PadY) : 0; }

		RemainingCellCount = 0;
		while (RemainingCellCount <= 0)
		{
			PotentialCells.Empty();
			UnavailableCellCount = 0;
			DebugLog(FString::Printf(TEXT("GridForgePrim - Trying with padding X:%d Y:%d"), PadX, PadY));
			// Create an array of cell availability.  True if cell is available, false if not.
			for (int32 Y = RoomGridTemplate.GridExtentMinY + PadY; Y <= RoomGridTemplate.GridExtentMaxY - PadY; Y++)
			{
				FVector2D CurCoord(RoomGridTemplate.GridExtentMinX + PadX, Y);
				if (BlackoutCells.Contains(CurCoord) || RoomGridTemplate.StartCells.Contains(CurCoord))
				{
					PotentialCells.Add(false);
					UnavailableCellCount++;
				}
				else
				{
					PotentialCells.Add(true);
				}
			}
			
			RemainingCellCount = (WidthY - (2 * PadY)) - UnavailableCellCount;
			// Make sure we still have some choices, after padding and blackout cells.
			if (RemainingCellCount <= 0)
			{
				// If no room left, reduce padding and try again.
				if (PadX > 0) 
				{	
					PadX--;	
				}
				else 
				{
					if (PadY == 0)
					{
						// No room at all for the cell
						UE_LOG(LogTRGame, Error, TEXT("GridForge cannot find room for start cell."));
						return;
					}
					else { PadY--; }
				}
			}
		}

		int32 PickedCellNumber = RandStream.RandRange(0, RemainingCellCount - 1);
		int32 CurCellNumber = 0;
		FVector2D StartCoords;

		DebugLog(FString::Printf(TEXT("GridForgePrim - Using padding X:%d Y:%d. UnavailableCellCount:%d RemainingCellCount:%d"), PadX, PadY, UnavailableCellCount, RemainingCellCount));
		DebugLog(FString::Printf(TEXT("GridForgePrim - Picked start cell %d"), PickedCellNumber));

		for (int32 Y = 0; Y <= WidthY - (2 * PadY); Y++)
		{
			if (PotentialCells[Y])
			{
				if (CurCellNumber == PickedCellNumber)
				{
					StartCoords.Set(static_cast<float>(RoomGridTemplate.GridExtentMinX + PadX), static_cast<float>(RoomGridTemplate.GridExtentMinY + PadY + Y));
					DebugLog(FString::Printf(TEXT("GridForgeBase - Found start cell X:%d Y:%d."), static_cast<int32>(StartCoords.X), static_cast<int32>(StartCoords.Y)));
					break;
				}
				else { CurCellNumber++; }
			}
		}

		// X direction is at lowest row + padding
		//int32 StartX = RandStream.RandRange(RoomGridTemplate.GridExtentMinX, RoomGridTemplate.GridExtentMinX + PadX);
		// Y direction can be anywhere along the row
		//int32 StartY = RandStream.RandRange(RoomGridTemplate.GridExtentMinY + PadY, RoomGridTemplate.GridExtentMaxY - PadY);
		//FVector2D StartCoords(StartX, StartY);

		RoomGridTemplate.StartCells.Add(StartCoords);
		DebugLog(FString::Printf(TEXT("GridForgeBase - using start cell X:%d Y:%d"), (int32)StartCoords.X, (int32)StartCoords.Y));
	}

	if (RoomGridTemplate.EndCells.Num() == 0)
	{
		// A percent chance to pad along grid extents.
		PadX = WidthX > 7 ? WidthX / 5 : 0;
		PadY = WidthY / 5;
		if (PadX > 0) { PadX = RandStream.RandRange(0, PadX); }
		if (PadY > 0) { PadY = RandStream.RandRange(0, PadY); }

		RemainingCellCount = 0;
		while (RemainingCellCount <= 0)
		{
			PotentialCells.Empty();
			UnavailableCellCount = 0;
			// Create an array of cell availability.  True if cell is available, false if not.
			for (int32 Y = RoomGridTemplate.GridExtentMinY + PadY; Y <= RoomGridTemplate.GridExtentMaxY - PadY; Y++)
			{
				FVector2D CurCoord(RoomGridTemplate.GridExtentMaxX - PadX, Y);
				if (BlackoutCells.Contains(CurCoord) || RoomGridTemplate.EndCells.Contains(CurCoord))
				{
					PotentialCells.Add(false);
					UnavailableCellCount++;
				}
				else
				{
					PotentialCells.Add(true);
				}
			}

			RemainingCellCount = (WidthY - (2 * PadY)) - UnavailableCellCount;
			// Make sure we still have some choices, after padding and blackout cells.
			if (RemainingCellCount <= 0)
			{
				// If no room left, reduce padding and try again.
				if (PadX > 0) { PadX--; }
				else
				{
					if (PadY == 0)
					{
						// No room at all for the cell
						UE_LOG(LogTRGame, Error, TEXT("GridForge cannot find room for end cell."));
						return;
					}
					else { PadY--; }
				}
			}
		}

		int32 PickedCellNumber = RandStream.RandRange(0, RemainingCellCount - 1);
		int32 CurCellNumber = 0;
		FVector2D EndCoords;

		for (int32 Y = 0; Y <= WidthY - (2 * PadY); Y++)
		{
			if (PotentialCells[Y])
			{
				if (CurCellNumber == PickedCellNumber)
				{
					EndCoords.Set(RoomGridTemplate.GridExtentMaxX - PadX, RoomGridTemplate.GridExtentMinY + PadY + Y);
					DebugLog(FString::Printf(TEXT("GridForge found end cell X:%d Y:%d."), (int32)EndCoords.X, (int32)EndCoords.Y));
					break;
				}
				else { CurCellNumber++; }
			}
		}

		RoomGridTemplate.EndCells.Add(EndCoords);
		DebugLog(FString::Printf(TEXT("GridForge using end cell X:%d Y:%d."), (int32)EndCoords.X, (int32)EndCoords.Y));
	}
}


bool UGridForgeBase::PickBlackoutCoords(FRandomStream& RandStream, FVector2D& BlackoutCoords)
{
	DebugLog(TEXT("GridForge - PickBlackoutCoords"));
	if (WorkingRoomGridTemplate == nullptr) { return false; }
	FRoomGridTemplate& Template = *WorkingRoomGridTemplate;
	float CellNumber = 0;
	float DirectionWidth = 0;
	float DirectionDepth = 0;
	float DirectionFullDepth = 0;
	float RotateDegrees = 0.0f;
	float GridMinX = static_cast<float>(Template.GridExtentMinX);
	float GridMaxX = static_cast<float>(Template.GridExtentMaxX);
	float GridMinY = static_cast<float>(Template.GridExtentMinY);
	float GridMaxY = static_cast<float>(Template.GridExtentMaxY);
	// Pick along an X or Y edge
	if (RandStream.FRandRange(0.0f, 1.0f) < BlackoutCellXYBias)
	{
		// X edge
		DirectionDepth = ((GridMaxX - GridMinX) + 1.0f) / 2.0f;
		DirectionFullDepth = (GridMaxX - GridMinX) + 1.0f;
		DirectionWidth = (GridMaxY - GridMinY) + 1.0f;
		if (RandStream.FRandRange(0.0f, 1.0f) < 0.5f) { 
			RotateDegrees = 180.0f;
			DirectionDepth = FMath::TruncToFloat(DirectionDepth); // Trunc won't include center row, if any
		}
		else { 
			RotateDegrees = 0.0f;
			DirectionDepth = FMath::CeilToFloat(DirectionDepth); // Ceil will include center row, if any
		}		
	}
	else
	{
		// Y edge
		DirectionDepth = ((GridMaxY - GridMinY) + 1.0f) / 2.0f;
		DirectionFullDepth = (GridMaxY - GridMinY) + 1.0f;
		DirectionWidth = (GridMaxX - GridMinX) + 1.0f;
		if (RandStream.FRandRange(0.0f, 1.0f) < 0.5f) {
			RotateDegrees = 90.0f;
			DirectionDepth = FMath::TruncToFloat(DirectionDepth);
		}
		else {
			RotateDegrees = 270.0f;
			DirectionDepth = FMath::CeilToFloat(DirectionDepth);
		}
	}
	// On each row but the last one, chance to move out another row.
	for (int32 CurDepth = 1; CurDepth < (int32)DirectionDepth; CurDepth++)
	{
		// Chance increases as we move out rows. More rows = higher chance to move out
		if (RandStream.FRandRange(0.0f, 1.0f) < (static_cast<float>(CurDepth) / DirectionDepth) * BlackoutCellCenterBias)
		{
			CellNumber += DirectionWidth;
		}
	}
	// Move along the last row
	if (DirectionWidth > 0) { CellNumber += RandStream.RandRange(0, DirectionWidth - 1); }

	DebugLog(FString::Printf(TEXT("    CellNumber: %d, Rotation: %d"), (int32)CellNumber, (int32)RotateDegrees));

	// Translate cell number to coords as +X, +Y offset
	FVector2D Offset = FVector2D(FMath::TruncToFloat(CellNumber / DirectionWidth), (float)((int32)CellNumber % (int32)DirectionWidth));
	DebugLog(FString::Printf(TEXT("        Offset: X:%f Y:%f"), Offset.X, Offset.Y));
	// Move the Offset so center of virtual grid is at origin
	Offset = Offset - FVector2D((DirectionFullDepth - 1.0f) / 2.0f, (DirectionWidth - 1.0f) / 2.0f);
	DebugLog(FString::Printf(TEXT("        AboutOrigin Offset: X:%f Y:%f"), Offset.X, Offset.Y));
	if (RotateDegrees != 0.0f) { Offset = Offset.GetRotated(RotateDegrees); }
	DebugLog(FString::Printf(TEXT("        Rotated Offset: X:%f Y:%f"), Offset.X, Offset.Y));
	// Translate this coord to our actual grid coord as an offset from the center of the grid
	FVector2D CellGridOriginOffset = FVector2D(
		GridMinX + (((GridMaxX - GridMinX) + 0.0f) / 2.0f),
		GridMinY + (((GridMaxY - GridMinY) + 0.0f) / 2.0f)
	);
	DebugLog(FString::Printf(TEXT("        Grid Origin: X:%f Y:%f"), CellGridOriginOffset.X, CellGridOriginOffset.Y));
	FVector2D FinalCoords = Offset + CellGridOriginOffset;
	DebugLog(FString::Printf(TEXT("        Translated Offset: X:%f Y:%f"), FinalCoords.X, FinalCoords.Y));
	FinalCoords = FinalCoords.RoundToVector();
	BlackoutCoords.Set(FinalCoords.X, FinalCoords.Y);
	DebugLog(FString::Printf(TEXT("PickBlackoutCoords picked X:%f Y:%f"), BlackoutCoords.X, BlackoutCoords.Y));
	return true;
}


bool UGridForgeBase::IsInGrid(const FVector2D Coords)
{
	int32 X = static_cast<int32>(Coords.X);
	int32 Y = static_cast<int32>(Coords.Y);
	if (WorkingRoomGridTemplate == nullptr) { return false; }
	if (X < WorkingRoomGridTemplate->GridExtentMinX || X > WorkingRoomGridTemplate->GridExtentMaxX) { return false; }
	if (Y < WorkingRoomGridTemplate->GridExtentMinY || Y > WorkingRoomGridTemplate->GridExtentMaxY) { return false; }
	return true;
}

FVector2D UGridForgeBase::CellToCoords(const UGridTemplateCell* Cell)
{
	if (Cell != nullptr) { return FVector2D(static_cast<int32>(Cell->X), static_cast<int32>(Cell->Y)); }
	return FVector2D();
}


int32 UGridForgeBase::GridCoordsToCellNumber(const FVector2D Coords)
{
	if (!IsInGrid(Coords)) return -1;
	int32 GridWidthY = (WorkingRoomGridTemplate->GridExtentMaxY - WorkingRoomGridTemplate->GridExtentMinY) + 1;
	//      Cells in prior rows                                                                         Cells in this row before us
	return ((static_cast<int32>(Coords.X) - WorkingRoomGridTemplate->GridExtentMinX) * (GridWidthY)) + (static_cast<int32>(Coords.Y) - WorkingRoomGridTemplate->GridExtentMinY);
}


//FVector2D UGridForgeBase::CellNumberToGridCoords(const int32 CellNumber)
//{
//	if (WorkingRoomGridTemplate == nullptr) { return FVector2D(); }
//	return FVector2D(
//		(float)CellNumber / (float)(WorkingRoomGridTemplate->GridExtentMaxY - WorkingRoomGridTemplate->GridExtentMinY),
//		(float)CellNumber % (float)
//	);
//}


FString UGridForgeBase::RoomToString(const FRoomTemplate& Room)
{
	FString NorthState = UEnum::GetValueAsString<ETRWallState>(Room.NorthWall);
	FString EastState = UEnum::GetValueAsString<ETRWallState>(Room.EastWall);
	FString SouthState = UEnum::GetValueAsString<ETRWallState>(Room.SouthWall);
	FString WestState = UEnum::GetValueAsString<ETRWallState>(Room.WestWall);
	return FString::Printf(TEXT("Room N:%s E:%s S:%s W:%s"), *NorthState, *EastState, *SouthState, *WestState);
}

