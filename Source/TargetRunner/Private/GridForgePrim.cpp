// Fill out your copyright notice in the Description page of Project Settings.


#include "GridForgePrim.h"
#include "TrEnums.h"
#include "..\Public\GridForgePrim.h"

void UGridForgePrim::GenerateGridTemplate(UPARAM(ref) FRandomStream& RandStream, FRoomGridTemplate& RoomGridTemplate, bool& bSuccessful)
{
	UE_LOG(LogTRGame, Log, TEXT("%s - GenerateGridTemplate"), *this->GetName());

	SetupFromRoomGridTemplate(RoomGridTemplate);
	
	PickStartAndEndCells(RandStream, RoomGridTemplate);

	GenerateGridTemplateCells(RandStream, RoomGridTemplate, bSuccessful);
	if (bSuccessful)
	{
		TranslateCellGridToRoomGrid(RandStream, RoomGridTemplate);
	}
}

// Fill the GridTemplate using our version of the Prim algorithm.
void UGridForgePrim::GenerateGridTemplateCells(UPARAM(ref) FRandomStream& RandStream, const FRoomGridTemplate& RoomGridTemplate, bool& bSuccessful)
{
	bSuccessful = false;
	
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
	EmptyGridTemplateCells();
		
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
			CurCell = GetOrCreateCell(CurCoords);
			if (CurCell == nullptr) { continue; }
			ConnectedCellCount++;
			UE_LOG(LogTRGame, Log, TEXT("GridForgePrim - New active cell X:%d Y:%d Flagged:%s"), CurCell->X, CurCell->Y, CurCell->bFlagged ? TEXT("True") : TEXT("False"));
		}
				
		GetUnflaggedCellNeighbors(CurCoords.X, CurCoords.Y, NeighborCells);

		if (NeighborCells.Num() == 0)
		{
			ActiveCoords.Remove(CurCoords);
			UE_LOG(LogTRGame, Log, TEXT("GridForgePrim - Removing X:%d Y:%d from active."), (int32)CurCoords.X, (int32)CurCoords.Y);
		}
		else
		{
			NextCell = NeighborCells[RandStream.RandRange(0, NeighborCells.Num() - 1)];
			NextCell->bFlagged = true;
			NextCoords = FVector2D(NextCell->X, NextCell->Y);
			CurCell->ConnectedCells.Add(NextCoords);
			UE_LOG(LogTRGame, Log, TEXT("GridForgePrim - Connected current cell X:%d Y:%d to neighbor X:%d Y:%d. Total connections: %d"), CurCell->X, CurCell->Y, NextCell->X, NextCell->Y, CurCell->ConnectedCells.Num());
			ConnectedCellCount++;
			// Check if we found the end cell.
			if (EndCoords == NextCoords)
			{
				// Stop creating more cells.
				ActiveCoords.Empty();
				bSuccessful = true;
				UE_LOG(LogTRGame, Log, TEXT("GridForgePrim - Found end grid cell X:%d Y:%d"), (int32)NextCoords.X, (int32)NextCoords.Y);
			}
			else
			{
				ActiveCoords.Add(NextCoords);
			}
		}
		CurCell = nullptr;
	}
	UE_LOG(LogTRGame, Log, TEXT("GridForgePrim - Conneccted %d grid cells."), ConnectedCellCount);
}

FVector2D UGridForgePrim::PickCoord(UPARAM(ref) FRandomStream& RandStream, const TArray<FVector2D>& CoordArray)
{
	// Pick a random one - this results in output similar to Prim's algorithm.
	return CoordArray[RandStream.RandRange(0, CoordArray.Num() - 1)];
}
