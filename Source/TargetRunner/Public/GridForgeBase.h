#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TargetRunner.h"
#include "RoomGridTemplate.h"
#include "GridTemplateCell.h"
#include "GridTemplateCellRow.h"
#include "GridForgeBase.generated.h"

/**
 *
 */
UCLASS()
class TARGETRUNNER_API UGridForgeBase : public UObject
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable)
        virtual void GenerateGridTemplate(UPARAM(ref) FRandomStream& RandStream, FRoomGridTemplate& RoomGridTemplate, bool& Successful);

    // Generates the underlying cell maze using info from the RoomGridTemplate. 
    UFUNCTION(BlueprintCallable)
        virtual void GenerateGridTemplateCells(UPARAM(ref) FRandomStream& RandStream, const FRoomGridTemplate& RoomGridTemplate, bool& Successful);

    UFUNCTION(BlueprintCallable)
        bool GetRoomRef(FRoomGridTemplate& RoomGridTemplate, const FVector2D& Coords, FRoomTemplate& RoomTemplate);

    UFUNCTION(BlueprintCallable)
        void EmptyGridTemplateCells();

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TMap<int32, UGridTemplateCellRow*> GridTemplateCells;

    // These cells are not available to be part of the maze grid. Set these before generating grid.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<FVector2D> BlackoutCells;

    int32 GridExtentMinX;
    int32 GridExtentMaxX;
    int32 GridExtentMinY;
    int32 GridExtentMaxY;

protected:

    void SetupFromRoomGridTemplate(const FRoomGridTemplate& RoomGridTemplate);

    FRoomTemplate* GetOrCreateRoom(FRoomGridTemplate& RoomGridTemplate, const FVector2D Coords, bool& bFound);

    FRoomTemplate* GetRoom(FRoomGridTemplate& RoomGridTemplate, const FVector2D& Coords, bool& bFound);

    // Returns the grid row. Creates the row if none existed.
    FRoomGridRow* GetRoomRow(FRoomGridTemplate& TemplateGrid, const int32 RowNum);

    FRoomTemplate* GetRoomNeighbor(FRoomGridTemplate& RoomGridTemplate, const FVector2D& Coords, const ETRDirection Direction);

    // Walls only "belong" to one room. If the neighbor has already determined the wall state (Blocked or Door), then we leave this one empty.
    // Change logic in here to change which rooms "own" the filled (i.e. non Empty) wall states.
    ETRWallState GetWallStateFromNeighbor(const ETRWallState NeighborState, const bool bConnectedToNeighbor);

    // Take our grid of cells and translate it to fill out the room grid in the RoomGridTemplate.
    void TranslateCellGridToRoomGrid(UPARAM(ref) FRandomStream& RandStream, FRoomGridTemplate& RoomGridTemplate);

    // Take a given grid cell and translte it to a RoomTemplate, then put that into the room grid in the RoomGridTemplate.
    void TranslateCellToRoom(UPARAM(ref) FRandomStream& RandStream, UGridTemplateCell* Cell, FRoomGridTemplate& RoomGridTemplate);

    // Grid cells related - for generating the base cell maze data.

    UFUNCTION(BlueprintCallable)
        UGridTemplateCell* GetCell(const FVector2D& Coords, bool& bFound);

    UFUNCTION(BlueprintCallable)
        UGridTemplateCell* GetOrCreateCellXY(const int32 X, const int32 Y);

    UFUNCTION(BlueprintCallable)
        UGridTemplateCell* GetOrCreateCell(const FVector2D& Coords);

    UFUNCTION(BlueprintCallable)
        UGridTemplateCell* GetCellNeighbor(const int32 X, const int32 Y, const ETRDirection Direction);
    
    UFUNCTION(BlueprintCallable)
        void GetCellNeighbors(const int32 X, const int32 Y, TArray<UGridTemplateCell*>& NeighborCells);

    UFUNCTION(BlueprintCallable)
        void GetUnflaggedCellNeighbors(const int32 X, const int32 Y, TArray<UGridTemplateCell*>& NeighborCells, const bool bIncludeBlocked = false);

    // If there are no start or end cells selected, select one of each and set it in the RoomTemplateGrid.
    void PickStartAndEndCells(UPARAM(ref) FRandomStream& RandStream, FRoomGridTemplate& RoomGridTemplate);    
    
    // Debug
    FString RoomToString(const FRoomTemplate& Room);
};