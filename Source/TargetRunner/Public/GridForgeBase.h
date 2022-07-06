#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Delegates/Delegate.h"
#include "TargetRunner.h"
#include "RoomGridTemplate.h"
#include "GridTemplateCell.h"
#include "GridTemplateCellRow.h"
#include "ProgressItem.h"
#include "GridForgeBase.generated.h"

    DECLARE_DELEGATE_OneParam(FOnGenerateGridProgress, const FProgressItem);

/**
 * A factory class to generate a GridTemplate map representing a two dimensional grid of cells.
 * Various attributes of this class describe limitations and characteristics of the grid to generate.
 * The cell grid represents the lower-level abstraction of a "maze" grid. The grid generated is later used by 
 * other classes for higher-level map generation. (ex: see RoomPlatformGridMgr)
 * Subclasses of this base class are intented to generate cell grids with differing characteristics. (see GridForgePrim)
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

    // Delegate event notification for grid generation progress.
        FOnGenerateGridProgress OnGenerateGridProgressDelegate;

protected:

    FRoomGridTemplate* WorkingRoomGridTemplate;

    // This contains the next group number to use. You must increment after using. (no wrapper functions now)
    int32 NextGroupNumber = 1;

    // Tracks which cell groups have members that are adjacent to a wall.
    // If it is in this list, the group is touching a wall somewhere.
    UPROPERTY(BlueprintReadOnly)
        TArray<int32> AnchoredCellGroups;

    // Key is group number, the Row array contains blocking cells in that group.
    UPROPERTY(BlueprintReadOnly)
        TMap<int32, UGridTemplateCellRow*> BlockingGroups;

#if WITH_EDITOR
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
        bool bEnableClassDebugLog;
#else
    bool bEnableClassDebugLog = false;
#endif

    const int32 WALL_GROUP_ID = -1;

    // Some helper arrays for iterating
    const TArray<ETRDirection> ClockwiseArray = { ETRDirection::North, ETRDirection::NorthEast, ETRDirection::East, ETRDirection::SouthEast,
                                                  ETRDirection::South, ETRDirection::SouthWest, ETRDirection::West, ETRDirection::NorthWest };
    const TArray<ETRDirection> OrthogonalDirections = { ETRDirection::North, ETRDirection::East, ETRDirection::South, ETRDirection::West };

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

    FRoomTemplate* GetOrCreateRoom(FRoomGridTemplate& RoomGridTemplate, const FVector2D& Coords, bool& bFound);

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
        void GetCellNeighbors(const FVector2D& Coords, TMap<ETRDirection, UGridTemplateCell*>& NeighborCells, const bool bIncludeDiagonal = true);

    // Adds a placeholder blocked cell for neighbors outside grid extents.
    void GetCellNeighbors(const UGridTemplateCell& Cell, TMap<ETRDirection, UGridTemplateCell*>& NeighborCells, const bool bIncludeDiagonal = true);

    // Only include entries in map for neighbors that exist and are blocked.
    UFUNCTION(BlueprintCallable)
        void GetBlockedCellNeighbors(const FVector2D& Coords, TMap<ETRDirection, UGridTemplateCell*>& NeighborCells);

    // NO LONGER USED
    // if it retuns true (can place) then the new blocking cell group number is populated.
    bool CanPlaceBlockingCell(const FVector2D& Coords, TMap<ETRDirection, UGridTemplateCell*>& FoundBlockingNeighbors);

    // Determines if a joining blocker can be placed. i.e. a "joining blocker" is a blocking cell between two or more other blocking cells.
    // If BlockingNeighbors is empty, this will call GetBlockingNeighbors().
    // If this returns true then the appropriate cell group number will be popuated > 0.
    // If this returns false then this is an invalid place for a new blocked cell and GroupNumber will = 0.
    bool GetBlockingCellGroupNumber(const FVector2D& Coords, TMap<ETRDirection, UGridTemplateCell*>& BlockingNeighbors, int32& GroupNumber);

    bool AllGroupCellsAdjacent(const FVector2D& Coords, const TMap<ETRDirection, UGridTemplateCell*>& BlockingNeighbors);

    UFUNCTION(BlueprintCallable)
        UGridTemplateCell* GetOrCreateCellXY(const int32 X, const int32 Y);

    UFUNCTION(BlueprintCallable)
        UGridTemplateCell* GetOrCreateCell(const FVector2D& Coords);

    UFUNCTION(BlueprintCallable)
        UGridTemplateCell* GetOrCreateCellNeighbor(const int32 X, const int32 Y, const ETRDirection Direction);
    
    UFUNCTION(BlueprintCallable)
        void GetOrCreateCellNeighbors(const int32 X, const int32 Y, TArray<UGridTemplateCell*>& NeighborCells, const bool bIncludeBlocked = true, const bool bIncludeDiagonal = true);

    UFUNCTION(BlueprintCallable)
        void GetUnflaggedCellNeighbors(const int32 X, const int32 Y, TArray<UGridTemplateCell*>& NeighborCells, const bool bIncludeBlocked = false, const bool bIncludeDiagonal = true);

    // If there are no start or end cells selected, select one of each and set it in the RoomTemplateGrid.
    void PickStartAndEndCells(UPARAM(ref) FRandomStream& RandStream);

    // Picks a coordinate for a blackout cell.
    // Returns true if a coordinate could be selected, false otherwise.
    bool PickBlackoutCoords(FRandomStream& RandStream, FVector2D& BlackoutCoords);

    UFUNCTION(BlueprintCallable)
        const bool IsInGrid(const FVector2D& Coords);
        
    // Each grid cell can also be identified by a number. The number of a given cell depends on the extents of the grid. 
    // Cells are numbered starting at GridExtentMinX, GridExtentMinY, proceeding along the +Y axis, then up the +X axis.
    // Resulting in cell 0 being at [GridExtentMinX, GridExtentMinY] and the highest cell number at [GridExtentMaxX, GridExtentMaxY].
    const int32 GridCoordsToCellNumber(const FVector2D& Coords);

    
    // Debug
    FString RoomToString(const FRoomTemplate& Room);
#if WITH_EDITOR
    FORCEINLINE void DebugLog(const FString& LogString) { if (bEnableClassDebugLog) { UE_LOG(LogTRGame, Log, TEXT("%s"), *LogString); } };
#else
    FORCEINLINE void DebugLog(const FString& LogString) { };
#endif
};