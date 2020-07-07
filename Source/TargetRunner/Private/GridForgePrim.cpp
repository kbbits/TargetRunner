// Fill out your copyright notice in the Description page of Project Settings.


#include "GridForgePrim.h"
#include "TrEnums.h"
#include "..\Public\GridForgePrim.h"

void UGridForgePrim::GenerateGridTemplate(UPARAM(ref) FRandomStream& RandStream, FRoomGridTemplate& RoomGridTemplate, bool& bSuccessful)
{
	DebugLog(FString::Printf(TEXT("%s - GenerateGridTemplate"), *this->GetName()));

	EmptyGridTemplateCells();
	SetRoomGridTemplate(RoomGridTemplate);
	GenerateBlackoutCells(RandStream);
	PickStartAndEndCells(RandStream);
	GenerateGridTemplateCells(RandStream, bSuccessful);
	//if (bSuccessful)
	//{
		TranslateCellGridToRoomGrid(RandStream);
	//}
}

// Fill the GridTemplate using our version of the Prim algorithm.
void UGridForgePrim::GenerateGridTemplateCells(UPARAM(ref) FRandomStream& RandStream, bool& bSuccessful)
{
	bSuccessful = false;
	FRoomGridTemplate& RoomGridTemplate = *WorkingRoomGridTemplate;

	if (RoomGridTemplate.StartCells.Num() == 0)
	{
		UE_LOG(LogTRGame, Error, TEXT("GridForge StartCell not set."));
		return;
	}
	if (RoomGridTemplate.EndCells.Num() == 0)
	{
		UE_LOG(LogTRGame, Error, TEXT("GridForge EndCell not set."));
		return;
	}
		
	const FVector2D StartCoords = RoomGridTemplate.StartCells[0];
	const FVector2D EndCoords = RoomGridTemplate.EndCells[0];
	int32 ConnectedCellCount = 1;
	TArray<FVector2D> ActiveCoords;
	TArray<UGridTemplateCell*> NeighborCells;
	UGridTemplateCell* StartCell = GetOrCreateCell(StartCoords);
	FVector2D CurCoords = StartCoords;
	UGridTemplateCell* CurCell = StartCell;
	FVector2D NextCoords;
	UGridTemplateCell* NextCell;
	bool bFound = false;
	ActiveCoords.Empty();
	if (StartCell == nullptr || StartCell->CellState == ETRGridCellState::Blocked)
	{
		UE_LOG(LogTRGame, Error, TEXT("GridForgePrim - Could not create an unblocked start cell for grid."));
		return;
	}
	StartCell->bFlagged = true;
	StartCell->DistanceToStart = 0;
	StartCell->DistanceToShortestPath = 0;
	ActiveCoords.Add(StartCoords);	
	while (ActiveCoords.Num() > 0)
	{
		if (CurCell == nullptr)
		{
			CurCoords = PickCoord(RandStream, ActiveCoords);
			DebugLog(FString::Printf(TEXT("GridForgePrim - %d active, picked X:%d Y:%d"), ActiveCoords.Num(), (int32)CurCoords.X, (int32)CurCoords.Y));
			CurCell = GetCell(CurCoords, bFound);
			if (CurCell == nullptr) {
				UE_LOG(LogTRGame, Error, TEXT("GridForgePrim - Picked cell coords X:%d Y:%d but cell is null."), (int32)CurCoords.X, (int32)CurCoords.Y);
				ActiveCoords.Remove(CurCoords);
				continue;
			}
			DebugLog(FString::Printf(TEXT("GridForgePrim - New active cell X:%d Y:%d Flagged:%s"), CurCell->X, CurCell->Y, CurCell->bFlagged ? TEXT("True") : TEXT("False")));
		}
		// Get available neighbors
		GetUnflaggedCellNeighbors(CurCoords.X, CurCoords.Y, NeighborCells, false, false);
		//if (NeighborCells.Num() == 0)
		//{
		//	// On dead-end, give a chance to "punch through" to already visited cell.
		//	if (RandStream.FRandRange(0.0f, 1.0f) < 0.2)
		//	{
		//		GetOrCreateCellNeighbors(CurCoords.X, CurCoords.Y, NeighborCells, false, false);
		//	}
		//}
		if (NeighborCells.Num() == 0)
		{
			ActiveCoords.Remove(CurCoords);
			NextCoords = CurCoords;
			NextCell = CurCell;
			DebugLog(FString::Printf(TEXT("GridForgePrim - Removing X:%d Y:%d from active."), (int32)CurCoords.X, (int32)CurCoords.Y));
		}
		else
		{
			NextCell = NeighborCells[RandStream.RandRange(0, NeighborCells.Num() - 1)];
			NextCoords = NextCell->GetCoords();
			if (!NextCell->bFlagged) { ActiveCoords.Add(NextCoords); }
			NextCell->bFlagged = true;
			if (NextCell->DistanceToStart == -1 || NextCell->DistanceToStart > CurCell->DistanceToStart) {
				NextCell->DistanceToStart = CurCell->DistanceToStart + 1;
			}			
			if (NextCell->ConnectedFrom.Num() == 0) 
			{ 
				NextCell->ConnectedFrom.Add(CurCell->GetCoords()); 
				ConnectedCellCount++;
			}
			CurCell->ConnectedCells.Add(NextCoords);
			DebugLog(FString::Printf(TEXT("GridForgePrim - Connected current cell X:%d Y:%d to neighbor X:%d Y:%d. Total connections: %d"), CurCell->X, CurCell->Y, NextCell->X, NextCell->Y, CurCell->ConnectedCells.Num()));
		}
				
		// Check if we found the end cell.
		// Putting this down here handles case when start == end and grid size is only one cell.
		if (EndCoords == NextCoords)
		{
			NextCell->DistanceToEnd = 0;
			if (bStopWhenExitFound)
			{
				// Stop creating more cells.
				ActiveCoords.Empty();
			}
			bSuccessful = true;
			DebugLog(FString::Printf(TEXT("GridForgePrim - Found end grid cell X:%d Y:%d"), (int32)NextCoords.X, (int32)NextCoords.Y));
		}
		
		CurCell = nullptr;
	}
	DebugLog(FString::Printf(TEXT("GridForgePrim - Conneccted %d grid cells."), ConnectedCellCount));

	// Now start at end and calculate shortest path info.
	// Walk back up the tree from end to start cell. (Start cell is root of tree)
	// Using arrays and "layers" to avoid recursion.
	TMap<ETRDirection, UGridTemplateCell*> NeighborCellsMap;
	UGridTemplateCell* CurNeighbor = nullptr;
	UGridTemplateCell* CurParent = nullptr;
	UGridTemplateCell* LastChild = nullptr;
	TArray<int32> LayerWidths;
	int32 CurDistanceToEnd = 1;
	int32 CurNeighborsCount = 0;
	bool bFoundShorter = false;
	bool bBacktrack = false;
	CurCell = nullptr;
	ActiveCoords.Empty(FMath::DivideAndRoundUp(ConnectedCellCount, 2));
	CurParent = GetCell(EndCoords, bFound);
	if (CurParent == nullptr) {
		UE_LOG(LogTRGame, Error, TEXT("GridForgePrim - End cell coords X:%d Y:%d cell is null."), (int32)EndCoords.X, (int32)EndCoords.Y);
		return;
	}
	CurParent->DistanceToEnd = 0;
	CurParent->DistanceToShortestPath = 0;
	while (CurParent != nullptr)
	{
		LayerWidths.Empty(FMath::DivideAndRoundUp(ConnectedCellCount, 2));
		// Walk down each "side path" of connected cells. This does it depth first.
		if (CurParent->ConnectedCells.Num() > 0)
		{
			ActiveCoords.Append(CurParent->ConnectedCells);
			LayerWidths.Push(CurParent->ConnectedCells.Num());
			while (ActiveCoords.Num() > 0)
			{
				// Track our progress across this "layer" of connected side-path children.
				while (LayerWidths.Num() > 0 && LayerWidths.Last() == 0)
				{
					LayerWidths.Pop(false);
					CurDistanceToEnd--;
				}
				LayerWidths[LayerWidths.Num() - 1]--;
				// Check the next cell from our list.
				CurNeighbor = GetCell(ActiveCoords.Pop(), bFound);
				if (CurNeighbor)
				{
					// Skip the child neighbor we came in on.
					if (LastChild != nullptr && CurNeighbor->GetCoords() == LastChild->GetCoords()) { 
						continue; 
					}
					if (CurNeighbor->DistanceToEnd == -1 || CurNeighbor->DistanceToEnd > CurDistanceToEnd) {
						CurNeighbor->DistanceToEnd = CurDistanceToEnd;
					}
					if (CurNeighbor->DistanceToShortestPath == -1 || CurNeighbor->DistanceToShortestPath > LayerWidths.Num()) {
						CurNeighbor->DistanceToShortestPath = LayerWidths.Num();
					}
					if (CurNeighbor->ConnectedCells.Num() > 0)
					{
						CurDistanceToEnd++;
						LayerWidths.Push(CurNeighbor->ConnectedCells.Num());
						ActiveCoords.Append(CurNeighbor->ConnectedCells);
					}
				}
			}
		}
		LastChild = CurParent;
		// Walk up to next parent
		if (CurParent->ConnectedFrom.Num() > 0)
		{
			// If we had multiple parents (we don't) we would pick the one closest to the start.
			//CurParent->ConnectedFrom.Sort([](const FVector2D& A, const FVector2D& B){
			//	UGridTemplateCell* TmpCellA = GetCell(A, bFound);
			//	UGridTemplateCell* TmpCellB = GetCell(B, bFound);
			//	return TmpCellA.DistanceToStart < TmpCellB.DistanceToStart;
			//});
			CurParent = GetCell(CurParent->ConnectedFrom[0], bFound);
			CurParent->DistanceToEnd = LastChild->DistanceToEnd + 1;
			CurParent->DistanceToShortestPath = 0;			
			CurDistanceToEnd = CurParent->DistanceToEnd + 1;
			//CurDistanceToShortestPath = 1;
		}
		else
		{
			CurParent = nullptr;
		}
	}
}

FVector2D UGridForgePrim::PickCoord_Implementation(UPARAM(ref) FRandomStream& RandStream, const TArray<FVector2D>& CoordArray)
{
	// Pick a random one - this results in output similar to Prim's algorithm.
	return CoordArray[RandStream.RandRange(0, CoordArray.Num() - 1)];
}
