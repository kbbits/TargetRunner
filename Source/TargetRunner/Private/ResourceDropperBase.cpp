
#include "ResourceDropperBase.h"
#include <random>
#include "..\Public\ResourceDropperBase.h"
#include "TRMath.h"
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
	TMap<FIntPoint, TArray<FResourceQuantity>> DistRoomResources;
	TArray<FIntPoint> AllRoomCoords;
	TArray<FIntPoint> DropRoomCoords;
	//TArray<FIntPoint> StartCoords = UTRMath::VectorsToIntPoints(TemplateGrid.StartCells);
	TArray<FIntPoint> ExitCoords = UTRMath::VectorsToIntPoints(TemplateGrid.EndCells);
	FIntPoint RoomCoords;
	int32 RoomCount = URoomFunctionLibrary::GetAllRoomTemplateCoordsInt(TemplateGrid, AllRoomCoords);
	int32 ResourceRoomCount = static_cast<int32>(static_cast<float>(RoomCount) * RandStream.FRandRange(MinRoomPercent, MaxRoomPercent)) - 1; // subtract one for exit room
	int32 RoomsRemaining = ResourceRoomCount;
	int32 TotalPickTries = RoomsRemaining * 2;

	UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("     Exit percent: %f"), ExitPercent);
	UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("     All rooms: %d of %d"), RoomCount, AllRoomCoords.Num());
	UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("     Resource rooms: %d"), ResourceRoomCount);

	UResourceFunctionLibrary::MultiplyResourceQuantity(TotalLevelResources, ExitPercent, true, TotalExitResources);
	UResourceFunctionLibrary::MultiplyResourceQuantity(TotalLevelResources, 1.0f - ExitPercent, true, TotalRoomResources);
	UResourceFunctionLibrary::MultiplyResourceQuantity(TotalRoomResources, 1.0f / static_cast<float>(ResourceRoomCount), true, PerRoomResources);

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
		UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("Total Per room resources:"));
		for (FResourceQuantity TmpResource : PerRoomResources) {
			UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("     %s: %f"), *TmpResource.ResourceType.Code.ToString(), TmpResource.Quantity * ResourceRoomCount);
		}
	}
	// Pick the coords for normal rooms to drop resources in.
	// Randomly for now
	while (RoomsRemaining > 0 && TotalPickTries > 0)
	{
		TotalPickTries--;
		RoomCoords = AllRoomCoords[RandStream.RandRange(0, AllRoomCoords.Num() - 1)];
		// Remove it from list, we only want to try each room once.
		AllRoomCoords.Remove(RoomCoords);
		// Skip it if it picked the exit room.
		if (ExitCoords.Contains(RoomCoords)) {
			continue;
		}
		DropRoomCoords.Add(RoomCoords);
		RoomsRemaining--;
	}
	// Distribute the resource quantities among rooms
	GeneratePerRoomResources(RandStream, TemplateGrid, PerRoomResources, DropRoomCoords, DistRoomResources);
	// Set exit room resources
	FRoomTemplate* ExitRoom = URoomFunctionLibrary::GetRoom(TemplateGrid, ExitCoords[0]);
	if (ExitRoom != nullptr) {
		ExitRoom->Resources = UResourceFunctionLibrary::AddResourceQuantities(ExitRoom->Resources, TotalExitResources);
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


void UResourceDropperBase::GeneratePerRoomResources(UPARAM(ref) FRandomStream& RandStream, UPARAM(ref) FRoomGridTemplate& TemplateGrid, const TArray<FResourceQuantity>& PerRoomResourcesOrig, const TArray<FIntPoint>& RoomCoords, TMap<FIntPoint, TArray<FResourceQuantity>>& RoomResources)
{
	RoomResources.Empty(RoomCoords.Num());
	UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("ResourceDropper::GeneratePerRoomResources - for %d rooms"), RoomCoords.Num());
	if (RoomCoords.Num() == 0) { return; } // Handle no rooms
	// Handle only one room or no variance
	if (RoomCoords.Num() == 1 || RoomResourceVariance <= 0.0f) 
	{
		for (FIntPoint Coord : RoomCoords) 
		{
			FRoomTemplate* TmpRoom = URoomFunctionLibrary::GetRoom(TemplateGrid, Coord);
			if (TmpRoom) 
			{ 
				RoomResources.Add(Coord, PerRoomResourcesOrig);
				TmpRoom->Resources = PerRoomResourcesOrig; 
			}
		}
		UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("ResourceDropper::GeneratePerRoomResources - 1 room or 0 RoomResourceVariance."));
		return;
	}
	/* 
	 * Arrays >= 2 length, normal logic continues below 
	 */	
	// Init all room resources with starting PerRoomResources
	for (FIntPoint Coords : RoomCoords) {
		RoomResources.Add(Coords, PerRoomResourcesOrig);
	}
	// Sort room coords from farthest-off-shortest path to closest, if tied then by farthest distance from start.
	TArray<FIntPoint> SortedCoords(RoomCoords);
	SortedCoords.Sort([&TemplateGrid](const FIntPoint& A, const FIntPoint& B) 
	{
		FRoomTemplate* RoomA = URoomFunctionLibrary::GetRoom(TemplateGrid, A);
		FRoomTemplate* RoomB = URoomFunctionLibrary::GetRoom(TemplateGrid, B);
		if (RoomA == nullptr) {
			return !(RoomB == nullptr);
		}
		else if (RoomB == nullptr) { 
			return true; 
		}
		if (RoomA->DistanceToShortestPath == RoomB->DistanceToShortestPath) {
			return RoomA->DistanceToStart > RoomB->DistanceToStart;
		}
		else {
			return RoomA->DistanceToShortestPath > RoomB->DistanceToShortestPath;
		}
	});	
	// Halfway index is pushed down, odd length arrays don't include the middle one. 
	//   ex: 2 rooms = 0, 3 rooms = 0, 4 rooms = 1, 5 rooms = 1, 6 rooms = 2, etc.
	int32 HalfCoordsIndex = (SortedCoords.Num() / 2) - 1;
	float MovePercent;
	float MoveQuantity;
	float MovePercentMax = RoomResourceVariance;
	float MovePercentMin;
	float DeltaPerRoom = (RoomResourceVariance * -2.0f) / (SortedCoords.Num() - 1);
	int32 OtherRoomIndex;
	FRoomTemplate* TmpRoom = nullptr;
	// Only go throuh the first half of the array, including HalfCoordsIndex
	// because we have these "out-of-the-way" rooms pull from the "on-the-way" rooms. 
	for (int32 i = 0; i <= HalfCoordsIndex; i++)
	{	
		MovePercentMin = MovePercentMax + (DeltaPerRoom * 0.5f);
		if (bEnableClassDebugLog)
		{
			UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("  Generating room resources %s. MinPercent %.2f MaxPercent %.2f"), *SortedCoords[i].ToString(), MovePercentMin, MovePercentMax);
			TmpRoom = URoomFunctionLibrary::GetRoom(TemplateGrid, SortedCoords[i]);
			if (TmpRoom) {
				UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("    ToShortestPath: %d  ToStart: %d  ToEnd: %d"), TmpRoom->DistanceToShortestPath, TmpRoom->DistanceToStart, TmpRoom->DistanceToEnd);
			}
		}
		// Get the room at the other end of the array
		OtherRoomIndex = (SortedCoords.Num() - 1) - i;
		TArray<FResourceQuantity>& TmpRoomQuantities = RoomResources[SortedCoords[i]];
		// For each resource type, pull/push resources from/to other room
		// All rooms have the same resource quantity types by index in their arrays. So same index, q, can be used.
		for (int32 q = 0; q < TmpRoomQuantities.Num(); q++)
		{
			UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("    Resource %s"), *TmpRoomQuantities[q].ResourceType.Code.ToString());
			// Each resource type gets a range of percent to move.
			MovePercent = RandStream.FRandRange(MovePercentMin, MovePercentMax);
			if (MovePercent == 0.0f) {
				continue;
			}
			if (MovePercent > 0.0f) 
			{
				// Percent > 0 then we are pulling from other room. So calculate based on other room quantity
				if (MovePercent >= 1.0f) { 
					MoveQuantity = RoomResources[SortedCoords[OtherRoomIndex]][q].Quantity;
				}
				else {
					MoveQuantity = FMath::TruncToFloat(RoomResources[SortedCoords[OtherRoomIndex]][q].Quantity * MovePercent);
					if (RoomResources[SortedCoords[OtherRoomIndex]][q].Quantity - MoveQuantity < MinRemainingRoomResources) {
						MoveQuantity = RoomResources[SortedCoords[OtherRoomIndex]][q].Quantity;
					}
				}				
				UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("      Pulling %.2f pct = %.1f from %s"), MovePercent, MoveQuantity, *SortedCoords[OtherRoomIndex].ToString());
			}
			else 
			{ 
				// Pushing to other room. So calculate based on this room quantity
				if (MovePercent <= -1.0f) { 
					MoveQuantity = TmpRoomQuantities[q].Quantity * -1.0f;
				}
				else {
					MoveQuantity = FMath::TruncToFloat(TmpRoomQuantities[q].Quantity * MovePercent);
					if (TmpRoomQuantities[q].Quantity - MoveQuantity < MinRemainingRoomResources) {
						MoveQuantity = TmpRoomQuantities[q].Quantity * -1.0f;
					}
				}				
				UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("      Pushing %.2f pct = %.1f to %s"), -MovePercent, -MoveQuantity, *SortedCoords[OtherRoomIndex].ToString());
			}
			// Remove the quantity from the other room
			RoomResources[SortedCoords[OtherRoomIndex]][q].Quantity -= MoveQuantity;
			// Add the quantity to this room
			TmpRoomQuantities[q].Quantity += MoveQuantity;
		}
		MovePercentMax += DeltaPerRoom;
	}
	// TODO: Handle middle room?
	// Now go and set all the picked room templates' resources.
	UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("Final room resources:"));
	
	TArray<FResourceQuantity> TotalFinalQuantities;
	for (FIntPoint DropRoomCoord : SortedCoords)
	{
		TArray<FResourceQuantity>& TmpRoomResources = RoomResources[DropRoomCoord];
		TmpRoom = URoomFunctionLibrary::GetRoom(TemplateGrid, DropRoomCoord);
		if (TmpRoom != nullptr) {
			// Remove any zero quantites from these final counts
			TmpRoom->Resources = UResourceFunctionLibrary::StripZeroQuantities(TmpRoomResources);
			if (bEnableClassDebugLog)
			{
				UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("  %s"), *DropRoomCoord.ToString());
				if (!UResourceFunctionLibrary::ContainsAnyQuantity(TmpRoom->Resources)) {
					UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("    No resources"));
				}
				for (FResourceQuantity RQ : TmpRoom->Resources) {
					UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("    %s %.2f"), *RQ.ResourceType.Code.ToString(), RQ.Quantity);
				}
				TotalFinalQuantities = UResourceFunctionLibrary::AddResourceQuantities(TmpRoom->Resources, TotalFinalQuantities);
			}
		}
		else {
			UE_LOG(LogTRGame, Error, TEXT("ResourceDropper::GeneratePerRoomResoures - Could not get room at %s"), *DropRoomCoord.ToString());
		}
	}
	if (bEnableClassDebugLog)
	{
		UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("Room Final Totals:"));
		for (FResourceQuantity RQ : TotalFinalQuantities) {
			UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("  %s %.1f"), *RQ.ResourceType.Code.ToString(), RQ.Quantity);
		}
	}
}

/*================ Private functions ============================*/
