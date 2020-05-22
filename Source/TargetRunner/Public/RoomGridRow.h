#pragma once

#include "Engine/DataTable.h"
#include "RoomTemplate.h"
#include "RoomGridRow.generated.h"

USTRUCT(BlueprintType)
struct FRoomGridRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	// This grid row's room templates. Y grid coordinate as key in map.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TMap<int32, FRoomTemplate> RowRooms;

};
