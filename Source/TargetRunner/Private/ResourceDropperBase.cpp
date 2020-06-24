
#include "ResourceDropperBase.h"
#include "..\Public\ResourceDropperBase.h"
#include "RoomFunctionLibrary.h"
#include "ResourceFunctionLibrary.h"

UResourceDropperBase::UResourceDropperBase()
	: Super()
{
	MinReservedForExit = 0.15f;
	MaxReservedForExit = 0.25f;
	MinRoomPercent = 0.6f;
	MaxRoomPercent = 0.8f;
}

void UResourceDropperBase::DistributeResources(UPARAM(ref) FRandomStream& RandStream, const TArray<FResourceQuantity>& TotalLevelResources, FRoomGridTemplate& TemplateGrid)
{
	DebugLog(FString::Printf(TEXT("DistributeResources Start")));
#if WITH_EDITOR
	for (FResourceQuantity TmpResource : TotalLevelResources)
	{
		DebugLog(FString::Printf(TEXT("     %s: %f"), *TmpResource.ResourceType.Code.ToString(), TmpResource.Quantity));
	}
#endif
	float ExitPercent = RandStream.FRandRange(MinReservedForExit, MaxReservedForExit);
	TArray<FResourceQuantity> TotalExitResources;
	TArray<FResourceQuantity> TotalRoomResources;
	TArray<FResourceQuantity> PerRoomResources;
	TArray<FVector2D> AllRoomCoords;
	TArray<FVector2D> DropRoomCoords;
	FVector2D RoomCoords;
	FRoomTemplate* TmpRoom = nullptr;
	int32 RoomCount = URoomFunctionLibrary::GetAllRoomTemplateCoords(TemplateGrid, AllRoomCoords);
	int32 ResourceRoomCount = static_cast<int32>(static_cast<float>(RoomCount - 1) * RandStream.FRandRange(MinRoomPercent, MaxRoomPercent)); // subtract one for exit room
	int32 RoomsRemaining = ResourceRoomCount;
	int32 TotalPickTries = RoomsRemaining * 2;

	DebugLog(FString::Printf(TEXT("     Exit percent: %f"), ExitPercent));
	DebugLog(FString::Printf(TEXT("     All rooms: %d or %d"), RoomCount, AllRoomCoords.Num()));
	DebugLog(FString::Printf(TEXT("     Resource rooms: %d"), ResourceRoomCount));

	UResourceFunctionLibrary::MultiplyResourceQuantity(TotalLevelResources, ExitPercent, true, TotalExitResources);
	UResourceFunctionLibrary::MultiplyResourceQuantity(TotalLevelResources, 1.0f - ExitPercent, true, TotalRoomResources);
	UResourceFunctionLibrary::MultiplyResourceQuantity(TotalRoomResources, 1.0f / ResourceRoomCount, true, PerRoomResources);

#if WITH_EDITOR
	DebugLog(TEXT("Exit resources:"));
	for (FResourceQuantity TmpResource : TotalExitResources)
	{
		DebugLog(FString::Printf(TEXT("     %s: %f"), *TmpResource.ResourceType.Code.ToString(), TmpResource.Quantity));
	}
	DebugLog(TEXT("Per room resources:"));
	for (FResourceQuantity TmpResource : PerRoomResources)
	{
		DebugLog(FString::Printf(TEXT("     %s: %f"), *TmpResource.ResourceType.Code.ToString(), TmpResource.Quantity));
	}
#endif

	// Add the exit cells
	DropRoomCoords.Append(TemplateGrid.EndCells);
	// Pick the coords for normal rooms to drop resources in.
	// Randomly for now
	while (RoomsRemaining > 0 && TotalPickTries > 0)
	{
		TotalPickTries--;
		RoomCoords = AllRoomCoords[RandStream.RandRange(0, AllRoomCoords.Num() - 1)];
		// Remove it from list, we only want to try each room once.
		AllRoomCoords.Remove(RoomCoords);
		// Skip it if it picked the exit room.
		if (TemplateGrid.EndCells.Contains(RoomCoords)) {
			continue;
		}
		DropRoomCoords.Add(RoomCoords);
		RoomsRemaining--;
	}
	// Now go and set all the picked room templates' resources.
	for (FVector2D DropRoomCoord : DropRoomCoords)
	{
		if (TemplateGrid.Grid.Contains(DropRoomCoord.X)) {
			TmpRoom = TemplateGrid.Grid[DropRoomCoord.X].RowRooms.Find(DropRoomCoord.Y);
			if (TmpRoom != nullptr)
			{
				// If it is the exit room, give it the exit resources.
				if (TemplateGrid.EndCells.Contains(DropRoomCoord)) {
					TmpRoom->Resources = TotalExitResources;
				}
				else {
					TmpRoom->Resources = PerRoomResources;
				}
			}
			else
			{
				// error log - should never happen.
			}
		}
	}

	DebugLog(FString::Printf(TEXT("DistributeResources Done.")));
}

/*================ Private functions ============================*/
