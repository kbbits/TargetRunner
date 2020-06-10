#pragma once


#include "Engine/DataTable.h"
#include "TrEnums.h"
#include "ResourceQuantity.h"
#include "RoomTemplate.generated.h"

USTRUCT(BlueprintType)
struct FRoomTemplate : public FTableRowBase
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

	// Is this room an "inside" room (bIsInterior=true) or an "outside" room (bIsInterior=false). Default = false = exterior rooms.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsInterior;

	// Total quantity of Resources contained in this room. Room generation logic may split these into different numbers of nodes, etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FResourceQuantity> Resources;
		
};