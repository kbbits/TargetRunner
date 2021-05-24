#pragma once


#include "Engine/DataTable.h"
#include "TrEnums.h"
#include "RoomGridRow.h"
#include "RoomGridTemplate.generated.h"

USTRUCT(BlueprintType)
struct FRoomGridTemplate : public FTableRowBase
{
	GENERATED_BODY()

public:

	// The grid of room templates. X grid coordinate as key in Grid map, Y grid coordinate as key in row map.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, NotReplicated)
		TMap<int32, FRoomGridRow> Grid;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TArray<FVector2D> StartCells;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TArray<FVector2D> EndCells;

	// The size, in world units, of each grid cell.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float GridCellWorldSize;

	// The number of subdivisions along each X & Y axis that each cell is divided.
	// Creating RoomCellSubdivision x RoomCellSubdivision total subcells in each cell.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 RoomCellSubdivision;

	// For walls with doors, this indicates whether doors will be placed a the center wall section of the room (true) or
	// doors will be placed along any of the RoomCellSubdivision wall sections. Default = true.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		bool bDoorsAtWallCenter = true;

	// Grid extents indicate the overall size of the grid. That is, the minimum and maxium valid grid coordinates.
	// Min extents are negative, max extents are positive, origin is at 0,0.
	// GridExtentMinX must be <= 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 GridExtentMinX;
	// Must be >= 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 GridExtentMaxX;
	// Must be <= 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 GridExtentMinY;
	// Must be >= 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 GridExtentMaxY;
};