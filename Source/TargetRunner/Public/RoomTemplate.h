#pragma once


//#include "Engine/DataTable.h"
#include "TrEnums.h"
#include "ResourceQuantity.h"
#include "RoomExitInfo.h"
#include "RoomTemplate.generated.h"

USTRUCT(BlueprintType)
struct FRoomTemplate //: public FTableRowBase
{
	GENERATED_BODY()

public:
	// States of each of the four walls. 
	// Currently, walls between neighboring rooms only exist in one of the rooms (the first room generated).
	// The corresponding wall section of the other neighbor is set to Empty. 
	// During wall spawning, only one wall section is spawned for each wall division -- instead of one for each room.
	// TODO: Move this Blocked/Empty neighbor logic higher -- to the wall spawning code.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ETRWallState NorthWall;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ETRWallState EastWall;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ETRWallState SouthWall;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ETRWallState WestWall;

	// The detailed array of wall segment states. One entry for each room cell subdivision per wall.
	// i.e. RoomCellSubdivision * 4 entries
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
		TArray<ETRWallState> WallTemplate;

	//Exit info for the room
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
		FRoomExitInfo ExitInfo;

	// Group ID within the grid
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Group;

	// If this is a "blackout" room. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsBlackout;

	// Is this room an "inside" room (bIsInterior=true) or an "outside" room (bIsInterior=false). Default = false = exterior rooms.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsInterior;

	// Distance (in room grid units) of this room to the start room.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 DistanceToStart = -1;

	// Distance (in room grid units) of this room to the end room.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 DistanceToEnd = -1;

	// Distance (in room grid units) of this room to the closest room on the shortest path from start to end.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 DistanceToShortestPath = -1;

	// Total quantity of Resources contained in this room. Room generation logic may split these into different numbers of nodes, etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (TitleProperty = "ResourceType"))
		TArray<FResourceQuantity> Resources;
		
	// The special actors in this room
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<AActor>> SpecialActors;

	// Seed to use when initializing the RandStream for this room.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		int32 RoomRandSeed;
};