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
	int32 ConnectedCellCount = 0;
	TArray<FVector2D> ActiveCoords;
	TArray<UGridTemplateCell*> NeighborCells;
	UGridTemplateCell* StartCell = GetOrCreateCell(StartCoords);
	FVector2D CurCoords = StartCoords;
	UGridTemplateCell* CurCell = StartCell;
	FVector2D NextCoords;
	UGridTemplateCell* NextCell;
	
	if (StartCell == nullptr || StartCell->CellState == ETRGridCellState::Blocked)
	{
		UE_LOG(LogTRGame, Error, TEXT("GridForgePrim - Could not create an unblocked start cell for grid."));
		return;
	}
	ActiveCoords.Add(StartCoords);
	StartCell->bFlagged = true;

	while (ActiveCoords.Num() > 0)
	{
		if (CurCell == nullptr)
		{
			CurCoords = PickCoord(RandStream, ActiveCoords);
			DebugLog(FString::Printf(TEXT("GridForgePrim - %d active, picked X:%d Y:%d"), ActiveCoords.Num(), (int32)CurCoords.X, (int32)CurCoords.Y));
			CurCell = GetOrCreateCell(CurCoords);
			if (CurCell == nullptr) { continue; }
			ConnectedCellCount++;
			DebugLog(FString::Printf(TEXT("GridForgePrim - New active cell X:%d Y:%d Flagged:%s"), CurCell->X, CurCell->Y, CurCell->bFlagged ? TEXT("True") : TEXT("False")));
		}
				
		GetUnflaggedCellNeighbors(CurCoords.X, CurCoords.Y, NeighborCells, false);
		
		if (NeighborCells.Num() == 0)
		{
			ActiveCoords.Remove(CurCoords);
			NextCoords = CurCoords;
			DebugLog(FString::Printf(TEXT("GridForgePrim - Removing X:%d Y:%d from active."), (int32)CurCoords.X, (int32)CurCoords.Y));
		}
		else
		{
			NextCell = NeighborCells[RandStream.RandRange(0, NeighborCells.Num() - 1)];
			NextCell->bFlagged = true;
			NextCoords = FVector2D(NextCell->X, NextCell->Y);
			CurCell->ConnectedCells.Add(NextCoords);
			DebugLog(FString::Printf(TEXT("GridForgePrim - Connected current cell X:%d Y:%d to neighbor X:%d Y:%d. Total connections: %d"), CurCell->X, CurCell->Y, NextCell->X, NextCell->Y, CurCell->ConnectedCells.Num()));
			ConnectedCellCount++;
			ActiveCoords.Add(NextCoords);
		}
				
		// Check if we found the end cell.
		// Putting this down here handles case when start == end and grid size is only one cell.
		if (EndCoords == NextCoords)
		{
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
}

FVector2D UGridForgePrim::PickCoord(UPARAM(ref) FRandomStream& RandStream, const TArray<FVector2D>& CoordArray)
{
	// Pick a random one - this results in output similar to Prim's algorithm.
	return CoordArray[RandStream.RandRange(0, CoordArray.Num() - 1)];
}
