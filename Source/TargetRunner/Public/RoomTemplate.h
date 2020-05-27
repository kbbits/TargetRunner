#pragma once


#include "Engine/DataTable.h"
#include "TrEnums.h"
#include "RoomTemplate.generated.h"

USTRUCT(BlueprintType)
struct FRoomTemplate : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ETRWallState NorthWall;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ETRWallState EastWall;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ETRWallState SouthWall;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ETRWallState WestWall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<ETRWallState> WallTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<ETRFloorState> FloorTemplate;
};