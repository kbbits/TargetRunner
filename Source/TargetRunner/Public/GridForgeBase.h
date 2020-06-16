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
UCLASS(Blueprintable)
class TARGETRUNNER_API UGridForgeBase : public UObject
{
    GENERATED_BODY()

public:
    UGridForgeBase();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TMap<int32, UGridTemplateCellRow*> GridTemplateCells;

    // These cells are not available to be part of the maze grid. Set these before generating grid.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<FVector2D> BlackoutCells;

    // Applied as a multiplier to the number of blackout cells generated.
    // Default is 1.0.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float BlackoutDensityFactor;

    // 0 - 1. Higher bias increases chance blackout cells will be towards center.
    // Default = 0.75
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float BlackoutCellCenterBias;

    // Higher bias increases chance of blackout cells on X edges, lower along Y edges.
    // 0-1  Default = 0.5
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float BlackoutCellXYBias;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 BlackoutSearchDistance;

protected:

    FRoomGridTemplate* WorkingRoomGridTemplate;

#if WITH_EDITOR
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
        bool bEnableClassDebugLog = false;
#endif

public:

    UFUNCTION(BlueprintCallable)
        virtual void GenerateGridTemplate(UPARAM(ref) FRandomStream& RandStream, FRoomGridTemplate& NewRoomGridTemplate, bool& Successful);

    // Generates the underlying cell maze using info from the RoomGridTemplate. 
    UFUNCTION(BlueprintCallable)
        virtual void GenerateGridTemplateCells(UPARAM(ref) FRandomStream& RandStream, bool& Successful);

    UFUNCTION(BlueprintCallable)
        bool GetRoomRef(FRoomGridTemplate& RoomGridTemplate, const FVector2D& Coords, FRoomTemplate& RoomTemplate);

    UFUNCTION(BlueprintCallable)
        void EmptyGridTemplateCells();

protected:

    void SetRoomGridTemplate(FRoomGridTemplate& RoomGridTemplate);

    // Base class implementation of picking blackout cells.
    virtual void GenerateBlackoutCells(FRandomStream& RandStream);

    FRoomTemplate* GetOrCreateRoom(FRoomGridTemplate& RoomGridTemplate, const FVector2D Coords, bool& bFound);

    FRoomTemplate* GetRoom(FRoomGridTemplate& RoomGridTemplate, const FVector2D& Coords, bool& bFound);

    // Returns the grid row. Creates the row if none existed.
    FRoomGridRow* GetRoomRow(FRoomGridTemplate& TemplateGrid, const int32 RowNum);

    FRoomTemplate* GetRoomNeighbor(FRoomGridTemplate& RoomGridTemplate, const FVector2D& Coords, const ETRDirection Direction);

    // Walls only "belong" to one room. If the neighbor has already determined the wall state (Blocked or Door), then we leave this one empty.
    // Change logic in here to change which rooms "own" the filled (i.e. non Empty) wall states.
    ETRWallState GetWallStateFromNeighbor(const ETRWallState NeighborState, const bool bConnectedToNeighbor);

    // Take our grid of cells and translate it to fill out the room grid in the RoomGridTemplate.
    void TranslateCellGridToRoomGrid(UPARAM(ref) FRandomStream& RandStream);

    // Take a given grid cell and translte it to a RoomTemplate, then put that into the room grid in the RoomGridTemplate.
    void TranslateCellToRoom(UPARAM(ref) FRandomStream& RandStream, UGridTemplateCell* Cell, FRoomGridTemplate& RoomGridTemplate);

    // ======== Grid cells related - for generating the base cell maze data. ============

    UFUNCTION(BlueprintCallable)
        UGridTemplateCell* GetCell(const FVector2D& Coords, bool& bFound);

    // Adds a placeholder blocked cell for neighbors outside grid extents.
    UFUNCTION(BlueprintCallable)
        void GetCellNeighbors(const FVector2D& Coords, TMap<ETRDirection, UGridTemplateCell*>& NeighborCells);

    // Adds a placeholder blocked cell for neighbors outside grid extents.
    void GetCellNeighbors(const UGridTemplateCell& Cell, TMap<ETRDirection, UGridTemplateCell*>& NeighborCells);

    bool HasOpposingBlockedNeighbors(const FVector2D& Coords, const int32 Distance, const TArray<FVector2D>& IgnoredCells, TMap<ETRDirection, UGridTemplateCell*>& Neighbors, bool& bOpposedNS, bool& bOpposedEW);

    UFUNCTION(BlueprintCallable)
        UGridTemplateCell* GetOrCreateCellXY(const int32 X, const int32 Y);

    UFUNCTION(BlueprintCallable)
        UGridTemplateCell* GetOrCreateCell(const FVector2D& Coords);

    UFUNCTION(BlueprintCallable)
        UGridTemplateCell* GetOrCreateCellNeighbor(const int32 X, const int32 Y, const ETRDirection Direction);
    
    UFUNCTION(BlueprintCallable)
        void GetOrCreateCellNeighbors(const int32 X, const int32 Y, TArray<UGridTemplateCell*>& NeighborCells);

    UFUNCTION(BlueprintCallable)
        void GetUnflaggedCellNeighbors(const int32 X, const int32 Y, TArray<UGridTemplateCell*>& NeighborCells, const bool bIncludeBlocked = false);

    // If there are no start or end cells selected, select one of each and set it in the RoomTemplateGrid.
    void PickStartAndEndCells(UPARAM(ref) FRandomStream& RandStream);

    // Picks a coordinate for a blackout cell.
    // Returns true if a coordinate could be selected, false otherwise.
    bool PickBlackoutCoords(FRandomStream& RandStream, FVector2D& BlackoutCoords);

    bool IsInGrid(const FVector2D Coords);

    FVector2D CellToCoords(const UGridTemplateCell* Cell);

    // Each grid cell can also be identified by a number. The number of a given cell depends on the extents of the grid. 
    // Cells are numbered starting at GridExtentMinX, GridExtentMinY, proceeding along the +Y axis, then up the +X axis.
    // Resulting in cell 0 being at [GridExtentMinX, GridExtentMinY] and the highest cell number at [GridExtentMaxX, GridExtentMaxY].
    int32 GridCoordsToCellNumber(const FVector2D Coords);

    //FVector2D CellNumberToGridCoords(const int32 CellNumber);
    
    // Debug
    FString RoomToString(const FRoomTemplate& Room);
#if WITH_EDITOR
    FORCEINLINE void DebugLog(const FString& LogString) { if (bEnableClassDebugLog) { UE_LOG(LogTRGame, Log, TEXT("%s"), *LogString); } };
#else
    FORCEINLINE void DebugLog(const FString& LogString) { };
#endif
};