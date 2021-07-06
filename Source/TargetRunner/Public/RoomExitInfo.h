#pragma once

#include "TrEnums.h"
#include "RoomExitInfo.generated.h"


USTRUCT(BlueprintType)
struct FRoomExitInfo
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		ETRRoomExitLayout ExitLayout;

	/**
	* Orientation of the exit layout. Ex: Orientation = East means a default (north oriented) layout should be rotated 90 degrees when spawned.
	* Must be one of the four cardinal directions (N,S,E,W).
	* Default orientation is North.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		ETRDirection Orientation;

public:

	FRoomExitInfo()
	{
		ExitLayout = ETRRoomExitLayout::None;
		Orientation = ETRDirection::North;
	}

	FORCEINLINE bool operator==(const FRoomExitInfo& Other) const
	{
		if (ExitLayout != Other.ExitLayout) return false;
		if (Orientation != Other.Orientation) return false;
		return true;
	}

	FORCEINLINE bool operator==(const FRoomExitInfo& Other)
	{
		if (ExitLayout != Other.ExitLayout) return false;
		if (Orientation != Other.Orientation) return false;
		return true;
	}

	FORCEINLINE bool operator==(FRoomExitInfo& Other)
	{
		if (ExitLayout != Other.ExitLayout) return false;
		if (Orientation != Other.Orientation) return false;
		return true;
	}

	FORCEINLINE bool operator!=(const FRoomExitInfo& Other) const
	{
		return !(*this == Other);
	}

	FORCEINLINE bool operator!=(const FRoomExitInfo& Other)
	{
		return !(*this == Other);
	}

	FORCEINLINE bool operator!=(FRoomExitInfo& Other)
	{
		return !(*this == Other);
	}
};
