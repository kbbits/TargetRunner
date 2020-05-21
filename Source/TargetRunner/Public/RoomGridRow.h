#pragma once

#include "Engine/DataTable.h"
#include "PlatformBase.h"
#include "PlatformGridRow.generated.h"

USTRUCT(BlueprintType)
struct FRoomGridRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TMap<int32, APlatformBase*> RowRooms;

};
