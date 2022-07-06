// Fill out your copyright notice in the Description page of Project Settings.


#include "GridForgeBase.h"
#include "RoomTemplate.h"
#include "RoomFunctionLibrary.h"
#include "TrEnums.h"
#include "TRMath.h"
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

	OnGenerateGridProgressDelegate.ExecuteIfBound(FProgressItem(TEXT("Generate Grid Template"), TEXT("Generating grid template"), 0.0f, 1.0f));
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
	OnGenerateGridProgressDelegate.ExecuteIfBound(FProgressItem(TEXT("Generate Grid Template"), TEXT("Generating grid template finished"), 1.0f, 1.0f));
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
			NewCell->Group = 1;
			UGridTemplateCellRow* NewRow = NewObject<UGridTemplateCellRow>(this);
			NewRow->RowCells.Add(GridCoordsToCellNumber(Coords), NewCell);
			BlockingGroups.Add(1, NewRow);
		}
	}
	else
	{
		float WidthX = static_cast<float>(WorkingRoomGridTemplate->GridExtentMaxX - WorkingRoomGridTemplate->GridExtentMinX) + 1.0f;
		float WidthY = static_cast<float>(WorkingRoomGridTemplate->GridExtentMaxY - WorkingRoomGridTemplate->GridExtentMinY) + 1.0f;
		int32 BlackoutCount = 0;
		bool bStopBlackoutSearch = false;
		FProgressItem Progress = FProgressItem(TEXT("Generate Blackout Cells"), TEXT("Generating blackout cells"), 0, 0);
		if (WidthX > 3 && WidthY > 3)
		{
			BlackoutCount = static_cast<int32>(FMath::RoundHalfToZero(FMath::Sqrt(WidthX * WidthY)) - RandStream.RandRange(0, 1));
			if (WidthX > 5 && WidthY > 5)
			{
				BlackoutCount += RandStream.RandRange(0, (int32)FMath::RoundHalfToZero((WidthX * WidthY) / 10.0f));
			}
		}
		BlackoutCount = (int32)((float)BlackoutCount * BlackoutDensityFactor);
		DebugLog(FString::Printf(TEXT("Generating blackout cells: %d"), BlackoutCount));
		Progress.OfTotalProgress = BlackoutCount;
		if (BlackoutCount > 0)
		{
			FVector2D BlackoutCoords;
			int32 PickFails = 0;
			OnGenerateGridProgressDelegate.ExecuteIfBound(Progress);
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
					//bool bBlockedNS, bBlockedEW;
					int32 GroupNumber;
					// Check our neighbors
					GetBlockedCellNeighbors(BlackoutCoords, Neighbors);
					if (AllGroupCellsAdjacent(BlackoutCoords, Neighbors) && GetBlockingCellGroupNumber(BlackoutCoords, Neighbors, GroupNumber))
					{
						BlackoutCoords = FVector2D((int32)BlackoutCoords.X, (int32)BlackoutCoords.Y);
						BlackoutCells.Add(BlackoutCoords);
						DebugLog(FString::Printf(TEXT("Added blackout cell X:%f Y:%f in group: %d"), BlackoutCoords.X, BlackoutCoords.Y, GroupNumber));
						NewCell = GetOrCreateCell(BlackoutCoords);
						NewCell->CellState = ETRGridCellState::Blocked;
						NewCell->Group = GroupNumber;
						BlockingGroups[GroupNumber]->RowCells.Add(GridCoordsToCellNumber(BlackoutCoords), NewCell);
						--BlackoutCount;
						PickFails = 0;
						OnGenerateGridProgressDelegate.ExecuteIfBound(Progress.Update(TEXT("Blackout cell generated")));
					}
					else
					{
						PickFails++;
						continue; // pick again
					}
				}
				else
				{
					PickFails++;
				}
			}
			OnGenerateGridProgressDelegate.ExecuteIfBound(Progress.Complete(TEXT("Blackout cells generated")));
		}		
#if WITH_EDITOR
		if (bEnableClassDebugLog) {
			DebugLog(FString::Printf(TEXT("Blackout cells generated: %d"), BlackoutCells.Num()));
			for (FVector2D dbgBlackout : BlackoutCells)
			{
				DebugLog(FString::Printf(TEXT("  X:%f Y:%f"), dbgBlackout.X, dbgBlackout.Y));
			}
		}
#endif
	}
}


FRoomTemplate* UGridForgeBase::GetOrCreateRoom(FRoomGridTemplate& RoomGridTemplate, const FVector2D& Coords, bool& bFound)
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
	// Get exit info for each room
	TArray<FVector2D> AllRoomCoords;
	URoomFunctionLibrary::GetAllRoomTemplateCoords(RoomGridTemplate, AllRoomCoords, true);
	for (FVector2D RoomCoords : AllRoomCoords)
	{
		FRoomTemplate* TmpRoom = URoomFunctionLibrary::GetRoom(RoomGridTemplate, RoomCoords.IntPoint());
		if (TmpRoom)
		{
			TmpRoom->ExitInfo = URoomFunctionLibrary::GetRoomExitInfo(RoomGridTemplate, RoomCoords.IntPoint());
		}
	}
}


void UGridForgeBase::TranslateCellToRoom(UPARAM(ref)FRandomStream& RandStream, UGridTemplateCell* Cell, FRoomGridTemplate& RoomGridTemplate)
{
	FRoomTemplate* Room;
	FRoomTemplate* TmpRoom;
	TMap<ETRDirection, UGridTemplateCell*> Neighbors;
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
			Room->Group = Cell->Group;
			Room->bIsBlackout = true;
			Room->bIsInterior = false;
		}
		else
		{
			Room->Group = Cell->Group;
			Room->bIsBlackout = false;
			Room->DistanceToStart = Cell->DistanceToStart;
			Room->DistanceToEnd = Cell->DistanceToEnd;
			Room->DistanceToShortestPath = Cell->DistanceToShortestPath;
			// Determine wall state
			// North	
			TmpRoom = GetRoomNeighbor(RoomGridTemplate, RoomCoords, ETRDirection::North);
			bConnected = Cell->ConnectedCells.Contains(RoomCoords + UTRMath::DirectionToOffset(ETRDirection::North));
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

			if (Cell->DistanceToStart == 0 || Cell->DistanceToEnd == 0)
			{
				Room->bIsInterior = false;
			}
			else
			{
				//GetCellNeighbors(*Cell, Neighbors, true);
				//if (Neighbors.Num() <= 5)
				//{
				//	Room->bIsInterior = false;
				//}
				Room->bIsInterior = RandStream.FRandRange(0.f, (Cell->DistanceToShortestPath * 0.5) + 1.0) <= 0.75f;
			}			
		}
		
		// Mark the cell as on the path, so we don't create a room for it again.
		Cell->bOnPath = true;
		DebugLog(FString::Printf(TEXT("Cell X:%d Y:%d translated to %s "), Cell->X, Cell->Y, *RoomToString(*Room)));
	}
}


// Returns nullptr if no such cell exists already.
UGridTemplateCell* UGridForgeBase::GetCell(const FVector2D& Coords, bool& bFound)
{
	FVector2D Rounded = Coords.RoundToVector();
	int32 X = static_cast<int32>(Rounded.X);
	int32 Y = static_cast<int32>(Rounded.Y);
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


// Adds a placeholder blocked cell into results for neighbors outside grid extents. (placeholder cells are not actually added to grid)
void UGridForgeBase::GetCellNeighbors(const FVector2D& Coords, TMap<ETRDirection, UGridTemplateCell*>& NeighborCells, const bool bIncludeDiagonal)
{
	bool bFound = false;
	UGridTemplateCell* Cell = nullptr;
	FVector2D NeighborCoords;
	const TArray<ETRDirection>* Directions;
	if (bIncludeDiagonal) {
		Directions = &ClockwiseArray;
		NeighborCells.Empty(8);
	}
	else {
		Directions = &OrthogonalDirections;
		NeighborCells.Empty(4);
	}
	for (ETRDirection Direction : *Directions)
	{
		NeighborCoords = Coords + UTRMath::DirectionToOffsetVector(Direction);
		Cell = GetCell(NeighborCoords, bFound);
		if (Cell != nullptr) { 
			NeighborCells.Add(Direction, Cell); 
		}
		else
		{
			if (!IsInGrid(NeighborCoords))
			{
				Cell = NewObject<UGridTemplateCell>(this);
				Cell->CellState = ETRGridCellState::Blocked;
				Cell->X = (int32)NeighborCoords.X;
				Cell->Y = (int32)NeighborCoords.Y;
				Cell->Group = -1;
				NeighborCells.Add(Direction, Cell);
			}			
		}
	}
}

void UGridForgeBase::GetCellNeighbors(const UGridTemplateCell& Cell, TMap<ETRDirection, UGridTemplateCell*>& NeighborCells, const bool bIncludeDiagonal)
{
	GetCellNeighbors(FVector2D(Cell.X, Cell.Y), NeighborCells, bIncludeDiagonal);
}


void UGridForgeBase::GetBlockedCellNeighbors(const FVector2D& Coords, TMap<ETRDirection, UGridTemplateCell*>& NeighborCells)
{
	TMap<ETRDirection, UGridTemplateCell*> AllNeighbors;
	GetCellNeighbors(Coords, AllNeighbors);
	NeighborCells.Empty();
	for (TPair<ETRDirection, UGridTemplateCell*> NeighborEntry : AllNeighbors)
	{
		if (NeighborEntry.Value != nullptr) { if (NeighborEntry.Value->CellState == ETRGridCellState::Blocked) { NeighborCells.Add(NeighborEntry.Key, NeighborEntry.Value); } }
	}
}


bool UGridForgeBase::CanPlaceBlockingCell(const FVector2D& Coords, TMap<ETRDirection, UGridTemplateCell*>& FoundBlockingNeighbors)
{
	TMap<ETRDirection, UGridTemplateCell*>& BlockingNeighbors = FoundBlockingNeighbors;
	// Get blocking neighbors in all eight directions.
	GetBlockedCellNeighbors(Coords, BlockingNeighbors);
	int32 NumBlockers = BlockingNeighbors.Num();
	// init the group number;
	//if (!GetBlockingCellGroupNumber(Coords, BlockingNeighbors, GroupNumber)) { return false; }

	// If 0 or 1 blockers, ok to place a blocker
	if (NumBlockers <= 1) { return true; }
	// If 7 or more blockers, we're in an "ally" or "pocket", so it is ok to place blocker.
	if (NumBlockers >= 7) { return true; }
	// Take a look at our immediately adjacent neighbors.
	int32 AdjacentBlockersNum = 0;
	FVector2D Offset;
	FVector2D AdjacentSumVector = FVector2D(0.0f, 0.f);
	FVector2D AbsSumVector = FVector2D(0.0f, 0.f);
	if (BlockingNeighbors.Contains(ETRDirection::North)) {
		AdjacentBlockersNum++;
		AdjacentSumVector += (BlockingNeighbors[ETRDirection::North]->GetCoords() - Coords);
	}
	if (BlockingNeighbors.Contains(ETRDirection::East)) {
		AdjacentBlockersNum++;
		AdjacentSumVector += (BlockingNeighbors[ETRDirection::East]->GetCoords() - Coords);
	}
	if (BlockingNeighbors.Contains(ETRDirection::South)) {
		AdjacentBlockersNum++;
		AdjacentSumVector += (BlockingNeighbors[ETRDirection::South]->GetCoords() - Coords);
	}
	if (BlockingNeighbors.Contains(ETRDirection::West)) {
		AdjacentBlockersNum++;
		AdjacentSumVector += (BlockingNeighbors[ETRDirection::West]->GetCoords() - Coords);
	}
	AbsSumVector = AdjacentSumVector.GetAbs();
	// If at least three of N, S, E and W are blocked, we're an ally or island and ok to place blocker.
	if (AdjacentBlockersNum >= 3) { return true; }

	// If all blockers are grid bounaries, ok to place blocker.
	int32 NumExtentNeighbors = 0;
	for (TPair<ETRDirection, UGridTemplateCell*> BlockerElem : BlockingNeighbors)
	{
		if (!IsInGrid(BlockerElem.Value->GetCoords()))
		{
			NumExtentNeighbors++;
		}
	}
	if (NumExtentNeighbors == NumBlockers) { return true; }
	
	// Num Blockers is at least 2 here, (or it would have hit success above). And...
	// If zero adjacent and total neighbors >= 2, it's a blocking join. 
	// Or, if we have only two blockers on opposite sides of each other.
	// Place only according to blocking join rules.
	if ((AdjacentBlockersNum == 0 /*&& NumBlockers >= 2*/) ||
		(NumBlockers == 2 && AdjacentSumVector.IsZero()))
	{
		return false;//GetBlockingCellGroupNumber(Coords, BlockingNeighbors, GroupNumber);
	}

	/* --- From here down AdjacentBlockersNum is 1 or 2 --- */
	
	bool bFound;
	FVector2D ProdVector = FVector2D(1.0f, 1.0f);
	FVector2D SumVector = FVector2D(0.0f, 0.f);
	for (TPair<ETRDirection, UGridTemplateCell*> BlockerElem : BlockingNeighbors)
	{
		Offset = (BlockerElem.Value->GetCoords() - Coords).RoundToVector();
		// Sum the relative offsets of the blocking neighbors
		SumVector += Offset;
		ProdVector *= Offset;
	}
	AbsSumVector = SumVector.GetAbs();
	// In a small "corner", ok to block
	if (NumBlockers == 2 && AdjacentBlockersNum == 2 && !AdjacentSumVector.IsZero()) { return true; }
	// Just two tucked next to each other, ok to add blocker
	if (NumBlockers == 2 && AdjacentBlockersNum == 1 && AbsSumVector.GetMax() == 2) { return true; }
	// If there are 3-5 total blockers: 
	if (NumBlockers >= 3 && NumBlockers <= 5)
	{
		// if the absolute value of the sum is 3 in either component, they are in a line and all bunched up. It is ok to place blocker.
		if (AbsSumVector.GetMax() == 3.0f) { return true; }
		// If there are exactly 3 blockers
		if (NumBlockers == 3)
		{
			// If the absolute value of the sum is 2 in both components, they are in a "corner", it is ok to place blocker.
			// If one is 2 and the other is 0, its a "zag" corner and ok to place.
			if ((AbsSumVector.X == 2.0f && (AbsSumVector.Y == 2.0f || AbsSumVector.Y == 0.0f)) ||
				AbsSumVector.Y == 2.0f && AbsSumVector.X == 0.0f) {
				return true;
			}
			if (AbsSumVector.GetMin() == 0.0f) {
				if (AbsSumVector.GetMax() >= 2) { return true; }
				if (AbsSumVector.GetMax() == 1.0f && ProdVector.IsZero()) { return true; }
			}
		}
		if (NumBlockers == 4 || NumBlockers == 5)
		{
			// Only two adjacent, look at the opposite diagonal - if it not blocked, then it is ok to block us.
			if (AdjacentBlockersNum == 2 && !AdjacentSumVector.IsZero())
			{
				UGridTemplateCell* DiagCell = GetCell(Coords - AdjacentSumVector, bFound);
				if (DiagCell == nullptr || (bFound && DiagCell->CellState != ETRGridCellState::Blocked)) { return true; }
			}
		}
	}
	// All valid arrangements of 0-5 blocking neighbors that do not result in a "join" should be handled above.
	// Valid arrangements of 6+ blocking neighbors are inherently handled by above.
	// Everything that falls through to here is considered a "join".
	// TODO: joining logic
	// return CanPlaceJoiningCell(Coords);
	return false;
}


bool UGridForgeBase::GetBlockingCellGroupNumber(const FVector2D& Coords, TMap<ETRDirection, UGridTemplateCell*>& BlockingNeighbors, int32& GroupNumber)
{
	GroupNumber = 0;
	if (BlockingNeighbors.Num() == 0)
	{
		GetBlockedCellNeighbors(Coords, BlockingNeighbors);
	}

	TArray<int32> NeighborGroups;
	int32 AnchoredCount = 0;
	int32 UseGroupNumber = 0;
	bool bHasOutOfGridNeighbor = false;
	for (TPair<ETRDirection, UGridTemplateCell*> NeighborElem : BlockingNeighbors)
	{
		if (NeighborElem.Value->Group > 0) // out-of-grid cells have group -1, blocking cells must have group > 0
		{
			// Start with first group 
			if (UseGroupNumber == 0) { UseGroupNumber = NeighborElem.Value->Group; }
			// For each different group:
			if (!NeighborGroups.Contains(NeighborElem.Value->Group))
			{
				NeighborGroups.Add(NeighborElem.Value->Group);
				if (AnchoredCellGroups.Contains(NeighborElem.Value->Group)) {
					AnchoredCount++; 
					if (AnchoredCount == 1) { UseGroupNumber = NeighborElem.Value->Group; }
				}
			}
		}
		else
		{
			if (!bHasOutOfGridNeighbor) { bHasOutOfGridNeighbor = !IsInGrid(NeighborElem.Value->GetCoords()); }
		}
	}
	// Can't join two or more different groups that are already anchored.
	// TODO: Add special cases where we can join two anchored groups. ex: if the two groups are orthogonnally opposite of each other and third orthogonal neighbor is a wall. (rectangle grids only)
	if (AnchoredCount > 1) { return false;	}
	if (UseGroupNumber > 0)
	{
		// If we found a blocing group use the existing group
		GroupNumber = UseGroupNumber;
		// Join the other (non-anchored) groups.
		UGridTemplateCellRow* CurGroup = nullptr;
		UGridTemplateCell* TmpCell = nullptr;
		for (int32 CurGroupNumber : NeighborGroups)
		{
			// For each "other" group:
			if (CurGroupNumber != GroupNumber)
			{
				DebugLog(FString::Printf(TEXT("GetBlockingCellGroupNumber - Joining group %d to %d."), CurGroupNumber, GroupNumber));
				CurGroup = BlockingGroups[CurGroupNumber];
				// Change each cell's group
				for (TPair<int32, UGridTemplateCell*> CurCellElem : CurGroup->RowCells)
				{
					CurCellElem.Value->Group = GroupNumber;
				}
				// Move the cells in the BlockingGroups map from old to new group.
				BlockingGroups[GroupNumber]->RowCells.Append(CurGroup->RowCells);
				CurGroup->RowCells.Empty();
				//BlockingGroups.Remove(CurGroupNumber);
				if (AnchoredCount > 0) { AnchoredCellGroups.Add(CurGroupNumber); }
			}
		}
	}
	else
	{
		// Found no group. Create a new group;
		GroupNumber = NextGroupNumber++;
		if (!BlockingGroups.Contains(GroupNumber))
		{
			UGridTemplateCellRow* NewRow = NewObject<UGridTemplateCellRow>(this);
			BlockingGroups.Add(GroupNumber, NewRow);
			DebugLog(FString::Printf(TEXT("GetBlockingCellGroupNumber - New group %d."), GroupNumber));
		}
	}
	if (bHasOutOfGridNeighbor && !AnchoredCellGroups.Contains(GroupNumber))
	{
		AnchoredCellGroups.Add(GroupNumber);
		DebugLog(FString::Printf(TEXT("GetBlockingCellGroupNumber - Anchored group %d."), GroupNumber));
	}

	return GroupNumber > 0;
}

bool UGridForgeBase::AllGroupCellsAdjacent(const FVector2D& Coords, const TMap<ETRDirection, UGridTemplateCell*>& BlockingNeighbors)
{
	if (BlockingNeighbors.Num() <= 1) { return true; }
	
	UGridTemplateCell* CurCell = nullptr;
	TArray<int32> CheckedGroups;
	int32 StartGroup = BlockingNeighbors.Contains(ETRDirection::NorthWest) ? BlockingNeighbors[ETRDirection::NorthWest]->Group : 0;
	int32 CurrentGroup = StartGroup;
	int32 LastOrthogonalGroup = BlockingNeighbors.Contains(ETRDirection::West) ? BlockingNeighbors[ETRDirection::West]->Group : 0;
	bool bFailIfChanges = false;
	bool bIsOrthogonal = true;
	// Make sure each group is contiguous
	for (ETRDirection Direction : ClockwiseArray)
	{
		bIsOrthogonal = OrthogonalDirections.Contains(Direction);
		CurCell = nullptr;
		if (BlockingNeighbors.Contains(Direction))
		{
			CurCell = BlockingNeighbors[Direction];
		}
		if (CurCell == nullptr)
		{
			// Only reset on orthogonal neighbors (directly adjacent)
			if (bIsOrthogonal) 
			{ 
				LastOrthogonalGroup = 0;
				CurrentGroup = 0; 
			}
			continue;
		}
		
		// Orthogonal neighbors block each other if not open. Handle this here.
		// If the current neighbor we are checking is orthognoal and the previous orthogonal neighbor is 
		// blocking (i.e. group > 0), then we need to compare against the previous orthogonal instead of the previous diagonal.
		if (bIsOrthogonal && LastOrthogonalGroup != 0)
		{
			CurrentGroup = LastOrthogonalGroup;
		}

		// If the current neighbor we are checking belongs to a different group than the appropriate previous neighbor,
		// make sure it is a valid arrangement.
		if (CurCell->Group != CurrentGroup)
		{				
			// If we've already seen this group or we're hitting a new anchored group and we've already seen a wall (or anchored group)
			// then we probaly have an invalid "join" of two groups. 
			if (CheckedGroups.Contains(CurCell->Group) || (CurrentGroup != -1 && AnchoredCellGroups.Contains(CurCell->Group) && CheckedGroups.Contains(-1)))
			{
				// Special case: Don't count as repeat hit if moving from anchored group directly to wall
				if (AnchoredCellGroups.Contains(CurrentGroup) && CurCell->Group == -1)
				{
					// Just change group 
					CurrentGroup = CurCell->Group;
					if (bIsOrthogonal) { LastOrthogonalGroup = CurCell->Group; }
					continue;
				}
				else
				{
					// Encountered same group twice, with something else between. So, all members of each group are not contiguous.
					DebugLog(FString::Printf(TEXT("GetBlockingCellGroupNumber - Group hit twice: %d"), CurCell->Group));
					return false;
				}
			}
			else
			{
				// Haven't seen this group yet, add it to the ones we have checked.
				CheckedGroups.Add(CurCell->Group);
				CurrentGroup = CurCell->Group;
				if (bIsOrthogonal) { LastOrthogonalGroup = CurCell->Group; }
				// Achored groups also count as visiting a wall
				if (AnchoredCellGroups.Contains(CurCell->Group)) { CheckedGroups.Add(-1); }
			}
		}
	}
	return true;
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
		FString StateName = GetEnumValueAsString<ETRGridCellState>(NewCell->CellState);
		DebugLog(FString::Printf(TEXT("GridForgeBase - Created new cell X:%d Y:%d State:%s"), NewCell->X, NewCell->Y, *StateName));
	}
	return CurRow->RowCells[Y];
}


UGridTemplateCell* UGridForgeBase::GetOrCreateCell(const FVector2D& Coords)
{
	FVector2D Rounded = Coords.RoundToVector();
	return GetOrCreateCellXY((int32)Rounded.X, (int32)Rounded.Y);
}


UGridTemplateCell* UGridForgeBase::GetOrCreateCellNeighbor(const int32 X, const int32 Y, const ETRDirection Direction)
{
	UGridTemplateCell* Cell = nullptr;
	Cell = GetOrCreateCell(FVector2D(X, Y) + UTRMath::DirectionToOffset(Direction));
	return Cell;
}


void UGridForgeBase::GetOrCreateCellNeighbors(const int32 X, const int32 Y, TArray<UGridTemplateCell*>& NeighborCells, const bool bIncludeBlocked, const bool bIncludeDiagonal)
{
	NeighborCells.Empty(bIncludeDiagonal ? 8 : 4);
	UGridTemplateCell* Cell;
	const TArray<ETRDirection>* Directions;
	if (bIncludeDiagonal) {
		Directions = &ClockwiseArray;
	}
	else {
		Directions = &OrthogonalDirections;
	}
	for (ETRDirection Direction : *Directions)
	{
		Cell = GetOrCreateCellNeighbor(X, Y, Direction);
		if (Cell != nullptr && (bIncludeBlocked || Cell->CellState != ETRGridCellState::Blocked )) { 
			NeighborCells.Add(Cell); 
		}
	}
}


void UGridForgeBase::GetUnflaggedCellNeighbors(const int32 X, const int32 Y, TArray<UGridTemplateCell*>& NeighborCells, const bool bIncludeBlocked, const bool bIncludeDiagonal)
{
	NeighborCells.Empty(bIncludeDiagonal ? 8 : 4);
	TArray<UGridTemplateCell*> TmpNeighbors;
	GetOrCreateCellNeighbors(X, Y, TmpNeighbors, bIncludeBlocked, bIncludeDiagonal);
	for (UGridTemplateCell* Cell : TmpNeighbors)
	{
		if (Cell != nullptr && !Cell->bFlagged) 
		{ 
			NeighborCells.Add(Cell);
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
	for (TPair<int32, UGridTemplateCellRow*> RowElem : BlockingGroups)
	{
		RowElem.Value->RowCells.Empty();
	}
	BlockingGroups.Empty();
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
		DebugLog(TEXT("GridForgeBase - Picking start cell."));
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
			DebugLog(FString::Printf(TEXT("GridForgeBase - Trying with padding X:%d Y:%d"), PadX, PadY));
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

		DebugLog(FString::Printf(TEXT("GridForgeBase - Using padding X:%d Y:%d. UnavailableCellCount:%d RemainingCellCount:%d"), PadX, PadY, UnavailableCellCount, RemainingCellCount));
		DebugLog(FString::Printf(TEXT("GridForgeBase - Picked start cell %d"), PickedCellNumber));

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


const bool UGridForgeBase::IsInGrid(const FVector2D& Coords)
{
	int32 X = static_cast<int32>(Coords.X);
	int32 Y = static_cast<int32>(Coords.Y);
	if (WorkingRoomGridTemplate == nullptr) { return false; }
	if (X < WorkingRoomGridTemplate->GridExtentMinX || X > WorkingRoomGridTemplate->GridExtentMaxX) { return false; }
	if (Y < WorkingRoomGridTemplate->GridExtentMinY || Y > WorkingRoomGridTemplate->GridExtentMaxY) { return false; }
	return true;
}


const int32 UGridForgeBase::GridCoordsToCellNumber(const FVector2D& Coords)
{
	if (!IsInGrid(Coords)) return -1;
	if (WorkingRoomGridTemplate == nullptr) return -1;
	int32 GridWidthY = (WorkingRoomGridTemplate->GridExtentMaxY - WorkingRoomGridTemplate->GridExtentMinY) + 1;
	//      Cells in prior rows                                                                         Cells in this row before us
	return ((static_cast<int32>(Coords.X) - WorkingRoomGridTemplate->GridExtentMinX) * (GridWidthY)) + (static_cast<int32>(Coords.Y) - WorkingRoomGridTemplate->GridExtentMinY);
}


FString UGridForgeBase::RoomToString(const FRoomTemplate& Room)
{
	FString NorthState = GetEnumValueAsString<ETRWallState>(Room.NorthWall);
	FString EastState = GetEnumValueAsString<ETRWallState>(Room.EastWall);
	FString SouthState = GetEnumValueAsString<ETRWallState>(Room.SouthWall);
	FString WestState = GetEnumValueAsString<ETRWallState>(Room.WestWall);
	return FString::Printf(TEXT("Room N:%s E:%s S:%s W:%s Group:%d, IsBlackout:%s"), *NorthState, *EastState, *SouthState, *WestState, Room.Group, Room.bIsBlackout ? TEXT("true") : TEXT("false"));
}

