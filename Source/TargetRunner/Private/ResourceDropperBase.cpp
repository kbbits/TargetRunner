
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
	UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("DistributeResources Start"));
	if (bEnableClassDebugLog)
	{
		for (FResourceQuantity TmpResource : TotalLevelResources) {
			UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("     %s: %f"), *TmpResource.ResourceType.Code.ToString(), TmpResource.Quantity);
		}
	}
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

	UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("     Exit percent: %f"), ExitPercent);
	UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("     All rooms: %d or %d"), RoomCount, AllRoomCoords.Num());
	UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("     Resource rooms: %d"), ResourceRoomCount);

	UResourceFunctionLibrary::MultiplyResourceQuantity(TotalLevelResources, ExitPercent, true, TotalExitResources);
	UResourceFunctionLibrary::MultiplyResourceQuantity(TotalLevelResources, 1.0f - ExitPercent, true, TotalRoomResources);
	UResourceFunctionLibrary::MultiplyResourceQuantity(TotalRoomResources, 1.0f / ResourceRoomCount, true, PerRoomResources);

	if (bEnableClassDebugLog)
	{
		UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("Exit resources:"));
		for (FResourceQuantity TmpResource : TotalExitResources) {
			UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("     %s: %f"), *TmpResource.ResourceType.Code.ToString(), TmpResource.Quantity);
		}
		UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("Per room resources:"));
		for (FResourceQuantity TmpResource : PerRoomResources) {
			UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("     %s: %f"), *TmpResource.ResourceType.Code.ToString(), TmpResource.Quantity);
		}
	}

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
		if (TemplateGrid.Grid.Contains(DropRoomCoord.X)) 
		{
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
			else {
				// error log ? - should never happen.
			}
		}
	}
	UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("DistributeResources Done."));
}


void UResourceDropperBase::DistributeSpecials(UPARAM(ref) FRandomStream& RandStream, const TArray<TSubclassOf<AActor>>& SpecialActorClasses, FRoomGridTemplate& TemplateGrid)
{
	UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("DistributeSpecials start."));
	TArray<FVector2D> EligibleCoords;
	FRoomTemplate* TmpRoom = nullptr;
	TArray<FRoomTemplate*> EligibleRooms;
	int32 RoomIndex;
	bool bFound;
	URoomFunctionLibrary::GetAllRoomTemplateCoords(TemplateGrid, EligibleCoords, false);
	// URoomFunctionLibrary::GetRoomTemplateGridAsArrays(TemplateGrid, EligibleCoords, EligibleRooms, false);
	if (EligibleCoords.Num() == 0)
	{
		UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("ResourceDropperBase::DistributeSpecials - No eligible rooms found."));
		return;
	}
	// Grab references to all eligible rooms
	for (FVector2D TmpCoords : EligibleCoords)
	{
		if (TemplateGrid.Grid.Contains(TmpCoords.X))
		{
			TmpRoom = TemplateGrid.Grid[TmpCoords.X].RowRooms.Find(TmpCoords.Y);
			if (TmpRoom != nullptr) {
				EligibleRooms.Add(TmpRoom);
			}
		}
	}	
	// Sort rooms from furthest-off-shortest path to closest, then by distance from start.
	EligibleRooms.Sort([](const FRoomTemplate& A, const FRoomTemplate& B) {
		if (A.DistanceToShortestPath == B.DistanceToShortestPath)
		{
			return A.DistanceToStart > B.DistanceToStart;
		}
		else 
		{
			return A.DistanceToShortestPath > B.DistanceToShortestPath;
		}
	});
	if (bEnableClassDebugLog)
	{
		for (FRoomTemplate* TmpTemplate : EligibleRooms) {
			UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("    Eligible room dist to shortest path: %d"), TmpTemplate->DistanceToShortestPath);
		}
		UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("    Distributing %d specials among %d eligible rooms."), SpecialActorClasses.Num(), EligibleRooms.Num());
	}
	RoomIndex = 0;
	for (TSubclassOf<AActor> CurSpecialActor : SpecialActorClasses)
	{
		bFound = false;
		while (RoomIndex < EligibleRooms.Num() && !bFound)
		{
			FRoomTemplate* CurRoom = EligibleRooms[RoomIndex];
			if (CurRoom && CurRoom->DistanceToStart != 0 && CurRoom->DistanceToEnd != 0) 
			{
				CurRoom->SpecialActors.Add(CurSpecialActor);
				UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("    Added special to room %d, %d"), (int32)EligibleCoords[RoomIndex].X, (int32)EligibleCoords[RoomIndex].Y);
				bFound = true;
			}
			RoomIndex++;
		}
	}
	UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("DistributeSpecials Done."));
}

/*================ Private functions ============================*/
