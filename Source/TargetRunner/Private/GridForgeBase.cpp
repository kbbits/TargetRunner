// Fill out your copyright notice in the Description page of Project Settings.


#include "GridForgeBase.h"
#include "RoomTemplate.h"
#include "TrEnums.h"
#include "..\Public\GridForgeBase.h"

void UGridForgeBase::GenerateGridTemplate(UPARAM(ref) FRandomStream& RandStream, FRoomGridTemplate& TemplateGrid, bool& Successful)
{
	SetupFromRoomGridTemplate(TemplateGrid);
	
	// Don't call PickStartAndEndCells() because we are choosing them manually.
	TemplateGrid.StartCells.Empty();
	TemplateGrid.StartCells.Add(FVector2D(0, 0));
	TemplateGrid.EndCells.Empty();
	TemplateGrid.EndCells.Add(FVector2D(1, 0));

	Successful = false;

	TArray<ETRWallState> DoorWall; 
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
	EmptyWall.Add(ETRWallState::Empty);

	// Generate each cell in the grid
	FRoomTemplate Room;
	Room.NorthWall = ETRWallState::Door;
	Room.EastWall = ETRWallState::Blocked;
	Room.SouthWall = ETRWallState::Blocked;
	Room.WestWall = ETRWallState::Blocked;
	Room.WallTemplate.Append(DoorWall);
	Room.WallTemplate.Append(SolidWall);
	Room.WallTemplate.Append(SolidWall);
	Room.WallTemplate.Append(SolidWall);
	GetRoomRow(TemplateGrid, 0)->RowRooms.Add(0, Room);
	Room.NorthWall = ETRWallState::Door;
	Room.EastWall = ETRWallState::Blocked;
	Room.SouthWall = ETRWallState::Blocked;
	Room.WestWall = ETRWallState::Blocked;
	Room.WallTemplate.Empty();
	Room.WallTemplate.Append(SolidWall);
	Room.WallTemplate.Append(SolidWall);
	Room.WallTemplate.Append(EmptyWall);
	Room.WallTemplate.Append(SolidWall);
	GetRoomRow(TemplateGrid, 1)->RowRooms.Add(0, Room);

	Successful = true;
}


// Default implementation does nothing but return true.
void UGridForgeBase::GenerateGridTemplateCells(UPARAM(ref) FRandomStream& RandStream, const FRoomGridTemplate& RoomGridTemplate, bool& Successful)
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

void UGridForgeBase::SetupFromRoomGridTemplate(const FRoomGridTemplate& RoomGridTemplate)
{
	GridExtentMinX = RoomGridTemplate.GridExtentMinX;
	GridExtentMaxX = RoomGridTemplate.GridExtentMaxX;
	GridExtentMinY = RoomGridTemplate.GridExtentMinY;
	GridExtentMaxY = RoomGridTemplate.GridExtentMaxY;
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
	bool bFound;
	switch (Direction)
	{
	case ETRDirection::North:
		Room = GetRoom(RoomGridTemplate, Coords + FVector2D(1, 0), bFound);
		break;
	case ETRDirection::East:
		Room = GetRoom(RoomGridTemplate, Coords + FVector2D(0, 1), bFound);
		break;
	case ETRDirection::South:
		Room = GetRoom(RoomGridTemplate, Coords + FVector2D(-1, 0), bFound);
		break;
	case ETRDirection::West:
		Room = GetRoom(RoomGridTemplate, Coords + FVector2D(0, -1), bFound);
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

void UGridForgeBase::TranslateCellGridToRoomGrid(UPARAM(ref)FRandomStream& RandStream, FRoomGridTemplate& RoomGridTemplate)
{
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
				UE_LOG(LogTRGame, Log, TEXT("GridForgeBase - Added cell to translate list X:%d Y:%d"), ConnectedCell->X, ConnectedCell->Y);
			}
		}		
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

	UE_LOG(LogTRGame, Log, TEXT("GridForgeBase - TranslateCellToRoom X:%d Y:%d"), Cell->X, Cell->Y);

	if (Cell->bOnPath)
	{
		UE_LOG(LogTRGame, Log, TEXT("GridForgeBase - Cell already on path, so room is already created. Returning."));
		return;
	}
	
	Room = GetOrCreateRoom(RoomGridTemplate, RoomCoords, bFound);
	if (bFound && Room != nullptr)
	{
		// Determine wall state
		// North	
		TmpRoom = GetRoomNeighbor(RoomGridTemplate, RoomCoords, ETRDirection::North);
		bConnected = Cell->ConnectedCells.Contains(RoomCoords + FVector2D(1, 0));
		NeighborWallState = ETRWallState::Empty;
		if (TmpRoom != nullptr) { NeighborWallState = TmpRoom->SouthWall; }
		else { UE_LOG(LogTRGame, Warning, TEXT("No north neighbor found.")); }
		Room->NorthWall = GetWallStateFromNeighbor(NeighborWallState, bConnected);
				
		// East
		TmpRoom = GetRoomNeighbor(RoomGridTemplate, RoomCoords, ETRDirection::East);
		bConnected = Cell->ConnectedCells.Contains(RoomCoords + FVector2D(0, 1));
		NeighborWallState = ETRWallState::Empty;
		if (TmpRoom != nullptr) { NeighborWallState = TmpRoom->WestWall; }
		else { UE_LOG(LogTRGame, Warning, TEXT("No east neighbor found.")); }
		Room->EastWall = GetWallStateFromNeighbor(NeighborWallState, bConnected);

		// South 
		TmpRoom = GetRoomNeighbor(RoomGridTemplate, RoomCoords, ETRDirection::South);
		bConnected = Cell->ConnectedCells.Contains(RoomCoords + FVector2D(-1, 0));
		NeighborWallState = ETRWallState::Empty;
		if (TmpRoom != nullptr) { NeighborWallState = TmpRoom->NorthWall; }
		else { UE_LOG(LogTRGame, Warning, TEXT("No south neighbor found.")); }
		Room->SouthWall = GetWallStateFromNeighbor(NeighborWallState, bConnected);

		// West
		TmpRoom = GetRoomNeighbor(RoomGridTemplate, RoomCoords, ETRDirection::West);
		bConnected = Cell->ConnectedCells.Contains(RoomCoords + FVector2D(0, -1));
		NeighborWallState = ETRWallState::Empty;
		if (TmpRoom != nullptr) { NeighborWallState = TmpRoom->EastWall; }
		else { UE_LOG(LogTRGame, Warning, TEXT("No west neighbor found.")); }
		Room->WestWall = GetWallStateFromNeighbor(NeighborWallState, bConnected);
		
		// Mark the cell as on the path, so we don't create a room for it again.
		Cell->bOnPath = true;
		UE_LOG(LogTRGame, Log, TEXT("Cell X:%d Y:%d translated to %s "), Cell->X, Cell->Y, *RoomToString(*Room));
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
	return nullptr;
}

UGridTemplateCell* UGridForgeBase::GetOrCreateCellXY(const int32 X, const int32 Y)
{
	// If outside grid, return nullptr
	if (X > GridExtentMaxX || X < GridExtentMinX || Y > GridExtentMaxY || Y < GridExtentMinY)
	{
		//UGridTemplateCell* NewCell = NewObject<UGridTemplateCell>(this, UGridTemplateCell::StaticClass());
		//NewCell->X = X;
		//NewCell->Y = Y;
		//NewCell->CellState = ETRGridCellState::Blocked;
		//return NewCell;
		return nullptr;
	}

	// Find or create it in the grid
	// If the grid doesn't have this row yet, create the row.
	if (!GridTemplateCells.Contains(X))
	{
		UGridTemplateCellRow* NewRow = NewObject<UGridTemplateCellRow>(this);
		GridTemplateCells.Add(X, NewRow);
	}

	UGridTemplateCellRow* CurRow = GridTemplateCells[X];
	// If the cell isn't in the row yet, insert a new default cell.
	if (!CurRow->RowCells.Contains(Y))
	{
		UGridTemplateCell* NewCell = NewObject<UGridTemplateCell>(this);
		NewCell->X = X;
		NewCell->Y = Y;
		if (BlackoutCells.Contains(FVector2D(X, Y)))
		{
			NewCell->CellState = ETRGridCellState::Blocked;
		}
		else
		{
			NewCell->CellState = ETRGridCellState::Open;
		}
		CurRow->RowCells.Add(Y, NewCell);
		FString StateName = UEnum::GetValueAsString<ETRGridCellState>(NewCell->CellState);
		UE_LOG(LogTRGame, Log, TEXT("GridForgeBase - Created new cell X:%d Y:%d State:%s"), NewCell->X, NewCell->Y, *StateName);
	}
	return CurRow->RowCells[Y];
}

UGridTemplateCell* UGridForgeBase::GetOrCreateCell(const FVector2D& Coords)
{
	return GetOrCreateCellXY(Coords.X, Coords.Y);
}

UGridTemplateCell* UGridForgeBase::GetCellNeighbor(const int32 X, const int32 Y, const ETRDirection Direction)
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

void UGridForgeBase::GetCellNeighbors(const int32 X, const int32 Y, TArray<UGridTemplateCell*>& NeighborCells)
{
	NeighborCells.Empty(4);
	UGridTemplateCell* Cell;
	Cell = GetCellNeighbor(X, Y, ETRDirection::North);
	if (Cell != nullptr) { NeighborCells.Add(Cell); }
	Cell = GetCellNeighbor(X, Y, ETRDirection::East);
	if (Cell != nullptr) { NeighborCells.Add(Cell); }
	Cell = GetCellNeighbor(X, Y, ETRDirection::South);
	if (Cell != nullptr) { NeighborCells.Add(Cell); }
	Cell = GetCellNeighbor(X, Y, ETRDirection::West);
	if (Cell != nullptr) { NeighborCells.Add(Cell); }
}

void UGridForgeBase::GetUnflaggedCellNeighbors(const int32 X, const int32 Y, TArray<UGridTemplateCell*>& NeighborCells, const bool bIncludeBlocked)
{
	NeighborCells.Empty(4);
	TArray<UGridTemplateCell*> TmpNeighbors;
	GetCellNeighbors(X, Y, TmpNeighbors);
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
	UE_LOG(LogTRGame, Log, TEXT("GridForgeBase - GetUnflaggedCellNeighbors found %d neighbors of cell X:%d Y:%d"), NeighborCells.Num(), X, Y);
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
}

void UGridForgeBase::PickStartAndEndCells(UPARAM(ref) FRandomStream& RandStream, FRoomGridTemplate& RoomGridTemplate)
{
	int32 WidthX = (RoomGridTemplate.GridExtentMaxX - RoomGridTemplate.GridExtentMinX) + 1;
	int32 WidthY = (RoomGridTemplate.GridExtentMaxY - RoomGridTemplate.GridExtentMinY) + 1;
	int32 PadX = 0;
	int32 PadY = 0;
	int32 RemainingCellCount = 0;
	int32 UnavailableCellCount = 0;
	TArray<bool> PotentialCells;
	
	// We start with moving from North -> South. So start position will be along X = 0 row + padding.
	// End cell will be along X = GridExtentMaxX row - padding.
	if (RoomGridTemplate.StartCells.Num() == 0)
	{	
		UE_LOG(LogTRGame, Log, TEXT("GridForgePrim - Picking start cell."));
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
			UE_LOG(LogTRGame, Log, TEXT("GridForgePrim - Trying with padding X:%d Y:%d"), PadX, PadY);
			// Create an array of cell availability.  True if cell is available, false if not.
			for (int32 Y = RoomGridTemplate.GridExtentMinY + PadY; Y <= GridExtentMaxY - PadY; Y++)
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
				if (PadX > 0) {	PadX--;	}
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

		UE_LOG(LogTRGame, Log, TEXT("GridForgePrim - Using padding X:%d Y:%d. UnavailableCellCount:%d RemainingCellCount:%d"), PadX, PadY, UnavailableCellCount, RemainingCellCount);
		UE_LOG(LogTRGame, Log, TEXT("GridForgePrim - Picked start cell %d"), PickedCellNumber);

		for (int32 Y = 0; Y <= WidthY - (2 * PadY); Y++)
		{
			if (PotentialCells[Y])
			{
				if (CurCellNumber == PickedCellNumber)
				{
					StartCoords.Set(static_cast<float>(RoomGridTemplate.GridExtentMinX + PadX), static_cast<float>(RoomGridTemplate.GridExtentMinY + PadY + Y));
					UE_LOG(LogTRGame, Log, TEXT("GridForgeBase - Found start cell X:%d Y:%d."), static_cast<int32>(StartCoords.X), static_cast<int32>(StartCoords.Y));
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
		UE_LOG(LogTRGame, Log, TEXT("GridForgeBase - using start cell X:%d Y:%d"), (int32)StartCoords.X, (int32)StartCoords.Y);
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
			for (int32 Y = RoomGridTemplate.GridExtentMinY + PadY; Y <= GridExtentMaxY - PadY; Y++)
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
					UE_LOG(LogTRGame, Log, TEXT("GridForge found end cell X:%d Y:%d."), (int32)EndCoords.X, (int32)EndCoords.Y);
					break;
				}
				else { CurCellNumber++; }
			}
		}

		RoomGridTemplate.EndCells.Add(EndCoords);
		UE_LOG(LogTRGame, Log, TEXT("GridForge using end cell X:%d Y:%d."), (int32)EndCoords.X, (int32)EndCoords.Y);
	}
}

FString UGridForgeBase::RoomToString(const FRoomTemplate& Room)
{
	FString NorthState = UEnum::GetValueAsString<ETRWallState>(Room.NorthWall);
	FString EastState = UEnum::GetValueAsString<ETRWallState>(Room.EastWall);
	FString SouthState = UEnum::GetValueAsString<ETRWallState>(Room.SouthWall);
	FString WestState = UEnum::GetValueAsString<ETRWallState>(Room.WestWall);
	return FString::Printf(TEXT("Room N:%s E:%s S:%s W:%s"), *NorthState, *EastState, *SouthState, *WestState);
}

