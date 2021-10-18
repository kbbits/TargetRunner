// Fill out your copyright notice in the Description page of Project Settings.

#include "RoomPlatformGridMgr.h"
#include "..\Public\RoomPlatformGridMgr.h"
#include "TRMath.h"
#include "TargetRunner.h"
#include "RoomFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TR_GameMode.h"

// Sets default values
ARoomPlatformGridMgr::ARoomPlatformGridMgr()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ResourceDropperClass = UResourceDropperBase::StaticClass();
	bDoorsAtWallCenter = false;
}


void ARoomPlatformGridMgr::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);	

	//DOREPLIFETIME_CONDITION(ARoomPlatformGridMgr, bDoorsAtWallCenter, COND_InitialOnly);
	DOREPLIFETIME(ARoomPlatformGridMgr, bDoorsAtWallCenter);
}


// Called when the game starts or when spawned
void ARoomPlatformGridMgr::BeginPlay()
{
	Super::BeginPlay();
	
}


void ARoomPlatformGridMgr::OnGridForgeProgress(const FProgressItem ProgressItem)
{
	DebugLog(FString::Printf(TEXT("RoomPlatformGridMgr::OnGridForgeProgress: Code: %s,  %s, %.0f, %.0f"), *ProgressItem.Code.ToString(), *ProgressItem.Message, ProgressItem.CurrentProgress, ProgressItem.OfTotalProgress));
	OnGenerateProgress.Broadcast(ProgressItem);
}



void ARoomPlatformGridMgr::InitRoomComponentMaps(const bool bForceReload)
{
	ATR_GameMode* GM = IsValid(GetWorld()) ? Cast<ATR_GameMode>(UGameplayStatics::GetGameMode(GetWorld())) : nullptr;
	if (GM)
	{
		RoomComponentsTable = GM->RoomComponentsTable;
	}
	if (bForceReload || RoomFloorComponentMap.Num() == 0)
	{
		ClearRoomComponentCaches();
		if (IsValid(RoomComponentsTable))
		{
			for (TPair<FName, uint8*> CompIt : RoomComponentsTable->GetRowMap())
			{
				// CompIt.Key has the key, CompIt.Value has a pointer to the struct of data
				FRoomComponentSpec* SpecRef = reinterpret_cast<FRoomComponentSpec*>(CompIt.Value);	
				if (SpecRef)
				{
					FRoomComponentSpec& Spec = *SpecRef;
					if (Spec.Type == ETRRoomComponentType::Floor)
					{
						for (ETRRoomExitLayout ExitLayout : Spec.ValidExitLayouts)
						{
							if (RoomFloorComponentMap.Contains(ExitLayout))
							{
								RoomFloorComponentMap[ExitLayout].Add(Spec);
							}
							else
							{
								TArray<FRoomComponentSpec> TmpSpecs;
								TmpSpecs.Add(Spec);
								RoomFloorComponentMap.Add(ExitLayout, TmpSpecs);
							}
						}
					}
					else if (Spec.Type == ETRRoomComponentType::Ceiling)
					{
						for (ETRRoomExitLayout ExitLayout : Spec.ValidExitLayouts)
						{
							if (RoomCeilingComponentMap.Contains(ExitLayout))
							{
								RoomCeilingComponentMap[ExitLayout].Add(Spec);
							}
							else
							{
								TArray<FRoomComponentSpec> TmpSpecs;
								TmpSpecs.Add(Spec);
								RoomCeilingComponentMap.Add(ExitLayout, TmpSpecs);
							}
						}
					}
					else if (Spec.Type == ETRRoomComponentType::Wall)
					{
						RoomWallComponentArray.Add(Spec);
					}
					else if (Spec.Type == ETRRoomComponentType::Door)
					{
						RoomDoorComponentArray.Add(Spec);
					}
				}
				else
				{
					UE_LOG(LogTRGame, Error, TEXT("ARoomPlatformGridMgr - InitRoomComponentMap found row in RoomComponentsTable is not a FRoomComponentSpec"));
				}
			}
		}
		else
		{
			UE_LOG(LogTRGame, Error, TEXT("ARoomPlatformGridMgr - RoomComponentsTable is invalid"));
		}
	}
}



// Called every frame
void ARoomPlatformGridMgr::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}


ARoomPlatformBase* ARoomPlatformGridMgr::GetRoomInGrid(const FVector2D& RoomCoords)
{
	bool bFound;
	return Cast<ARoomPlatformBase>(GetPlatformInGrid(RoomCoords, bFound));
}


ARoomPlatformBase* ARoomPlatformGridMgr::GetRoomNeighbor(const FVector2D& RoomCoords, const ETRDirection DirectionToNeighbor)
{
	return Cast<ARoomPlatformBase>(GetPlatformNeighbor(RoomCoords, DirectionToNeighbor));
}


void ARoomPlatformGridMgr::GenerateGridImpl()
{
	bool bSuccessful = false;
	// Destroy old grid, if any
	DestroyGrid();
	DebugLog(FString::Printf(TEXT("%s RoomPlatformGridMgr::GenerateGrid - Generating grid. Extents: MinX:%d MinY:%d  MaxX:%d MaxY:%d"), *this->GetName(), GridExtentMinX, GridExtentMinY, GridExtentMaxX, GridExtentMaxY));
	FRandomStream* GridStreamFound = nullptr;
	FRandomStream* ResourceStreamFound = nullptr;
	// Grab the game mode.
	ATR_GameMode* GameMode = Cast<ATR_GameMode>(GetWorld()->GetAuthGameMode());
	if (IsValid(GameMode))
	{
		// Get our random streams from game mode
		GridStreamFound = &GameMode->GetGridStream();
		ResourceStreamFound = &GameMode->GetResourceDropperStream();
	}
	else 
	{
		UE_LOG(LogTRGame, Error, TEXT("%s RoomPlatformGridMgr::GenerateGrid - Could not get GameMode. Using default rand stream."), *this->GetName());
		// Use our default streams
		GridStreamFound = &DefaultGridRandStream;
		GridStreamFound->Reset();
		ResourceStreamFound = &DefaultResourceDropperStream;
		ResourceStreamFound->Reset();
	}
	
	FRandomStream& GridRandStream = *GridStreamFound;
	FRandomStream& ResourceDropperStream = *ResourceStreamFound;
	DebugLog(FString::Printf(TEXT("RoomPlatformGridMgr::GenerateGridImpl - GridRandStream seed: %d"), GridRandStream.GetInitialSeed()));
	// Create the grid forge
	UGridForgeBase* GridForge = NewObject<UGridForgeBase>(this, GridForgeClass);
	if (IsValid(GridForge))
	{ 
		GridForge->OnGenerateGridProgressDelegate.BindUObject(this, &ARoomPlatformGridMgr::OnGridForgeProgress);
	}
	else
	{
		UE_LOG(LogTRGame, Error, TEXT("%s RoomPlatformGridMgr::GenerateGrid - Could not construct GridForge."), *this->GetName());
		return;
	}
	GridForge->EmptyGridTemplateCells();
	// Populate basic properties of the template
	RoomGridTemplate.GridCellWorldSize = GridCellWorldSize;
	RoomGridTemplate.RoomCellSubdivision = RoomCellSubdivision;
	RoomGridTemplate.bDoorsAtWallCenter = bDoorsAtWallCenter;
	if (RoomGridTemplate.RoomCellSubdivision == 0) { RoomGridTemplate.RoomCellSubdivision = 1; }
	RoomGridTemplate.GridExtentMinX = GridExtentMinX;
	RoomGridTemplate.GridExtentMaxX = GridExtentMaxX;
	RoomGridTemplate.GridExtentMinY = GridExtentMinY;
	RoomGridTemplate.GridExtentMaxY = GridExtentMaxY;
	if (bGenerateStartEnd)
	{
		RoomGridTemplate.StartCells.Empty();
		RoomGridTemplate.EndCells.Empty();
	}
	else
	{
		RoomGridTemplate.StartCells.Add(StartGridCoords);
		RoomGridTemplate.EndCells.Add(ExitGridCoords);
	}
	// Use override blackout cells if present
	if (OverrideBlackoutCells.Num() > 0)
	{
		// Use the manually specified blackout cells.
		GridForge->BlackoutCells = OverrideBlackoutCells;
	}
	else
	{
		GridForge->BlackoutCells.Empty();
	}
	
	// Fill the RoomGridTemplate's grid
	GridForge->GenerateGridTemplate(GridRandStream, RoomGridTemplate, bSuccessful);

	if (bSuccessful)
	{
		DebugLog(FString::Printf(TEXT("%s RoomPlatformGridMgr::GenerateGrid - Generating grid successful."), *GetNameSafe(this)));
		StartGridCoords = RoomGridTemplate.StartCells[0];
		ExitGridCoords = RoomGridTemplate.EndCells[0];
		// Allocate resources to room templates
		UResourceDropperBase* ResourceDropper = NewObject<UResourceDropperBase>(this, ResourceDropperClass);
		if (ResourceDropper == nullptr) {
			UE_LOG(LogTRGame, Error, TEXT("%s RoomPlatformGridMgr::GenerateGrid - Could not construct ResourceDropper."), *GetNameSafe(this));
			return;
		}
		// Drop the resources
		ResourceDropper->DistributeResources(ResourceDropperStream, ResourcesToDistribute, RoomGridTemplate);
		// Drop special actors
		ResourceDropper->DistributeSpecials(ResourceDropperStream, SpecialsToDistribute, RoomGridTemplate);
		// Move/Create player start locations
		MovePlayerStarts();
		#if WITH_EDITOR
			if (bSpawnRoomsAfterGenerate) {	SpawnRooms();}
		#endif
	} 
	else
	{
		UE_LOG(LogTRGame, Warning, TEXT("Grid generation failed with grid forge: %s"), *GetNameSafe(GridForge));
	}
	// Clear the cell grid, just to be tidy.
	GridForge->EmptyGridTemplateCells();

	if (GetLocalRole() == ROLE_Authority)
	{
		// Update the clients' room grid template
		//ClientUpdateRoomGridTemplate(RoomGridTemplate);
	}
	GridForge->OnGenerateGridProgressDelegate.Unbind();
}


void ARoomPlatformGridMgr::DestroyGridImpl()
{
	//TArray<int32> RowNums;
	//TArray<int32> PlatformNums;
	//PlatformGridMap.GenerateKeyArray(RowNums);

	//DebugLog(FString::Printf(TEXT("Destroying %d rows."), RowNums.Num()));
	//for (int32 Row : RowNums)
	//{
	//	PlatformGridMap.Find(Row)->RowPlatforms.GenerateKeyArray(PlatformNums);
	//	for (int32 Col : PlatformNums)
	//	{
	//		APlatformBase* Platform = PlatformGridMap.Find(Row)->RowPlatforms[Col];
	//		if (IsValid(Platform))
	//		{
	//			DebugLog(FString::Printf(TEXT("Destroying room X:%d Y:%d."), Platform->GridX, Platform->GridY));
	//			Platform->Destroy();
	//		}
	//	}
	//	PlatformGridMap.Find(Row)->RowPlatforms.Empty();
	//}
	//PlatformGridMap.Empty();

	Super::DestroyGridImpl();
	TArray<int32> RowNums;
	RowNums.Empty(RoomGridTemplate.Grid.Num());
	RoomGridTemplate.Grid.GenerateKeyArray(RowNums);
	for (int32 Row : RowNums)
	{
		RoomGridTemplate.Grid.Find(Row)->RowRooms.Empty();
	}
	RoomGridTemplate.Grid.Empty();
	RoomGridTemplate.StartCells.Empty();
	RoomGridTemplate.EndCells.Empty();
	ClearRoomComponentCaches();

	if (GetLocalRole() == ROLE_Authority)
	{
		// Update the clients' room grid template
		//ClientUpdateRoomGridTemplate(RoomGridTemplate);
	}
}


void ARoomPlatformGridMgr::SpawnRooms_Implementation()
{
	TArray<int32> RowNums;
	TArray<int32> ColNums;
	RoomGridTemplate.Grid.GenerateKeyArray(RowNums);
	for (int32 Row : RowNums)
	{
		ColNums.Empty(RoomGridTemplate.Grid.Find(Row)->RowRooms.Num());
		RoomGridTemplate.Grid.Find(Row)->RowRooms.GenerateKeyArray(ColNums);
		for (int32 Col : ColNums)
		{
			FRoomTemplate* Room = RoomGridTemplate.Grid.Find(Row)->RowRooms.Find(Col);
			if (Room != nullptr)
			{
				SpawnRoom(FVector2D(Row, Col));
			}
		}
	}
	// Now spawn all room contents
	TArray<FVector2D> AllRoomCoords;
	bool bFound;
	URoomFunctionLibrary::GetAllRoomTemplateCoords(RoomGridTemplate, AllRoomCoords, true);
	for (FVector2D RoomCoord : AllRoomCoords)
	{
		ARoomPlatformBase* RoomPlatform = Cast<ARoomPlatformBase>(GetPlatformInGrid(RoomCoord, bFound));
		if (RoomPlatform != nullptr)
		{
			RoomPlatform->SpawnContents(); // calls SpawnResources, SpawnSpecials, SpawnClutter
		}
	}
	if (GetLocalRole() == ROLE_Authority)
	{
		// Have the clients update their grid maps
		//ClientFillGridFromExistingPlatforms();
	}
}

#if WITH_EDITOR
void ARoomPlatformGridMgr::SpawnRoomsEd_Implementation()
{
	SpawnRooms();
}
#endif


void ARoomPlatformGridMgr::SpawnRoom_Implementation(FVector2D GridCoords)
{
	// First remove any existing platform.
	bool bSuccess;
	APlatformBase* OldRoom = RemovePlatformFromGrid(GridCoords, bSuccess);
	if (IsValid(OldRoom))
	{
		DebugLog(FString::Printf(TEXT("Destroying old room X:%d Y:%d."), OldRoom->GridX, OldRoom->GridY));
		OldRoom->DestroyPlatform();
		OldRoom = nullptr;
	}

	ARoomPlatformBase* NewRoom;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = this;
	ATR_GameMode* GameMode = Cast<ATR_GameMode>(GetWorld()->GetAuthGameMode());

	DebugLog(FString::Printf(TEXT("%s SpawnRoom - Spanwing room actor for X:%d Y:%d."), *this->GetName(), (int32)GridCoords.X, (int32)GridCoords.Y));
	
	FRoomGridRow* GridRow = RoomGridTemplate.Grid.Find(GridCoords.X);
	if (GridRow == nullptr) { UE_LOG(LogTRGame, Error, TEXT("%s - Could not find grid row %d"), *this->GetName(), (int32)GridCoords.X); }
	FRoomTemplate* RoomTemplate = GridRow->RowRooms.Find(GridCoords.Y);
	if (RoomTemplate == nullptr) { UE_LOG(LogTRGame, Error, TEXT("%s - Could not find room template at X:%d Y:%d"), *this->GetName(), (int32)GridCoords.X, (int32)GridCoords.Y); }

	NewRoom = GetWorld()->SpawnActor<ARoomPlatformBase>(RoomClass, GetGridCellWorldTransform(GridCoords), SpawnParams);
	if (NewRoom != nullptr)
	{
		NewRoom->MyGridManager = this;
		if (IsValid(GameMode))
		{
			// Get our random stream from game mode
			//NewRoom->PlatformRandStream.Initialize(GameMode->GetGridStream().RandRange(1, INT_MAX - 1));
			RoomTemplate->RoomRandSeed = GameMode->GetGridStream().RandRange(1, INT_MAX - 1);
		}
		else
		{
			// Use our default streams
			//NewRoom->PlatformRandStream.Initialize(DefaultGridRandStream.RandRange(1, INT_MAX - 1));
			RoomTemplate->RoomRandSeed = DefaultGridRandStream.RandRange(1, INT_MAX - 1);
		}
		NewRoom->GridX = GridCoords.X;
		NewRoom->GridY = GridCoords.Y;
		AddPlatformToGridMap(NewRoom);
		// Setting room template will trigger replication to clients. Clients will handle generation via the OnRep hooks.
		NewRoom->ServerSetRoomTemplate(*RoomTemplate);		
		NewRoom->ServerGenerateRoom();
	}
}


void ARoomPlatformGridMgr::SpawnsFinished_Implementation()
{
	ClearRoomComponentCaches();
}


void ARoomPlatformGridMgr::SetRoomGridTemplateData_Implementation(const FRoomGridTemplate& UpdatedTemplate, const TArray<FVector2D>& RoomCoords, const TArray<FRoomTemplate>& RoomTemplates)
{
	RoomGridTemplate = UpdatedTemplate;
	int32 CoordX;
	int32 CoordY;
	for (int i = 0; i < RoomCoords.Num(); i++)
	{
		CoordX = (int32)RoomCoords[i].X;
		CoordY = (int32)RoomCoords[i].Y;
		if (!RoomGridTemplate.Grid.Contains(CoordX))
		{
			FRoomGridRow NewRow;
			RoomGridTemplate.Grid.Add(CoordX, NewRow);
		}
		RoomGridTemplate.Grid.Find(CoordX)->RowRooms.Add(CoordY, RoomTemplates[i]);
	}
	// Multicast replication but only needed on clients.
	//if (GetLocalRole() < ROLE_Authority)
	//{
	//	RoomGridTemplate = UpdatedTemplate;
	//}
}


void ARoomPlatformGridMgr::WakeNeighborsImpl(FVector2D AroundGridCoords)
{
	const TArray<ETRDirection> OrthogonalDirections = { ETRDirection::North, ETRDirection::East, ETRDirection::South, ETRDirection::West };
	FVector2D OffsetCoords;
	ARoomPlatformBase* CurRoom = nullptr;
	ARoomPlatformBase* NeighborRoom = nullptr;
	ARoomPlatformBase* AroundRoom = GetRoomInGrid(AroundGridCoords);

	if (AroundRoom && !AroundRoom->bStasisWokeNeighbors)
	{
		// Orthogonal directions only
		for (ETRDirection Direction : OrthogonalDirections)
		{
			OffsetCoords = AroundGridCoords;
			for (int32 i = 1; i <= StasisWakeRange; i++)
			{
				CurRoom = GetRoomInGrid(OffsetCoords);
				if (CurRoom)
				{
					// Connected neighbors only
					NeighborRoom = CurRoom->GetConnectedNeighbor(Direction);
					if (NeighborRoom)
					{
						NeighborRoom->StasisWakeActors();
					}
				}
				// Move OffsetCoords to get next room over, even if there was no room here.
				OffsetCoords += UTRMath::DirectionToOffsetVector(Direction);
			}
		}
		AroundRoom->bStasisWokeNeighbors = true;
	}
}



/*
TSoftObjectPtr<UPrefabricatorAssetInterface> ARoomPlatformGridMgr::GetRoomComponentPrefab_Implementation(const ETRRoomComponentType Type, const ETRRoomExitLayout ExitLayout, bool& bFound)
{
	InitRoomComponentMaps();
	float TotalWeight = 0.0f;
	float CurWeightSum = 0.0f;
	float PickedWeight = 0.0f;
	TMap<ETRRoomExitLayout, TArray<FRoomComponentSpec>>* CompMap = nullptr;
	if (Type == ETRRoomComponentType::Floor)
	{
		CompMap = &RoomFloorComponentMap;
	}
	else if (Type == ETRRoomComponentType::Ceiling)
	{
		CompMap = &RoomCeilingComponentMap;
	}
	if (CompMap && CompMap->Contains(ExitLayout))
	{
		if ((*CompMap)[ExitLayout].Num() > 1)
		{
			TotalWeight = 0.0f;
			for (FRoomComponentSpec Spec : (*CompMap)[ExitLayout])
			{
				TotalWeight += Spec.PickWeight;
			}
			PickedWeight = FRandRangeGrid(0.0f, TotalWeight);
			CurWeightSum = 0.0f;
			for (FRoomComponentSpec Spec : (*CompMap)[ExitLayout])
			{
				CurWeightSum += Spec.PickWeight;
				if (CurWeightSum >= PickedWeight)
				{
					bFound = true;
					return Spec.ComponentPrefab;
				}
			}
		}
		else
		{
			bFound = true;
			return (*CompMap)[ExitLayout][0].ComponentPrefab;
		}
	}
	bFound = false;
	return nullptr;
}
*/


TSubclassOf<ARoomComponentActor> ARoomPlatformGridMgr::GetRoomComponentActorInLayoutMap_Implementation(const ETRRoomComponentType Type, const ETRRoomExitLayout ExitLayout, bool& bFound)
{
	InitRoomComponentMaps();
	float TotalWeight = 0.0f;
	float CurWeightSum = 0.0f;
	float PickedWeight = 0.0f;
	TMap<ETRRoomExitLayout, TArray<FRoomComponentSpec>>* CompMap = nullptr;
	FRandomStream* GridStreamFound = nullptr;
	ATR_GameMode* GameMode = Cast<ATR_GameMode>(GetWorld()->GetAuthGameMode());
	if (IsValid(GameMode))
	{
		GridStreamFound = &GameMode->GetGridStream();
	}
	else
	{
		GridStreamFound = &DefaultGridRandStream;
	}
	if (Type == ETRRoomComponentType::Floor)
	{
		CompMap = &RoomFloorComponentMap;
	}
	else if (Type == ETRRoomComponentType::Ceiling)
	{
		CompMap = &RoomCeilingComponentMap;
	}
	else {
		UE_LOG(LogTRGame, Error, TEXT("RoomPlatformGridMgr GetRoomComponentActorInLayoutMap - called with invalid room type %s"), *GetEnumValueAsString<ETRRoomComponentType>(Type));
		bFound = false;
		return nullptr;
	}
	if (CompMap && CompMap->Contains(ExitLayout))
	{
		if ((*CompMap)[ExitLayout].Num() > 1)
		{
			// Sum total weight
			TotalWeight = 0.0f;
			for (FRoomComponentSpec Spec : (*CompMap)[ExitLayout]) {
				TotalWeight += Spec.PickWeight;
			}
			// Find the one at the picked weight.
			PickedWeight = GridStreamFound->FRandRange(0.0f, TotalWeight);
			CurWeightSum = 0.0f;
			for (FRoomComponentSpec Spec : (*CompMap)[ExitLayout])
			{
				CurWeightSum += Spec.PickWeight;
				if (CurWeightSum >= PickedWeight)
				{
					//UE_LOG(LogTRGame, Log, TEXT("RoomGridMgr GetRoomComponentActorInLayoutMap picked room component %s from %d choices for exit layout %s (picked weight: %f)"),
					//	*Spec.ComponentActor->GetName(),
					//	(*CompMap)[ExitLayout].Num(),
					//	*GetEnumValueAsString<ETRRoomExitLayout>(ExitLayout),
					//	PickedWeight);
					bFound = true;
					return Spec.ComponentActor;
				}
			}
		}
		else
		{
			bFound = true;
			return (*CompMap)[ExitLayout][0].ComponentActor;
		}
	}
	else
	{
		if (!CompMap) {
			UE_LOG(LogTRGame, Warning, TEXT("RoomPlatformGridMgr GetRoomComponentActorInLayoutMap - No room component of type %s"), *GetEnumValueAsString<ETRRoomComponentType>(Type));
		}
		if (CompMap && !CompMap->Contains(ExitLayout)) {
			UE_LOG(LogTRGame, Warning, TEXT("RoomPlatformGridMgr GetRoomComponentActorInLayoutMap - No %s room component for exit layout %s"), 
				*GetEnumValueAsString<ETRRoomComponentType>(Type), 
				*GetEnumValueAsString<ETRRoomExitLayout>(ExitLayout));
		}
	}
	bFound = false;
	return nullptr;
}


TSubclassOf<ARoomComponentActor> ARoomPlatformGridMgr::GetRoomComponentActorInArray_Implementation(const ETRRoomComponentType Type, const FIntPoint RoomCoords, bool& bFound)
{
	InitRoomComponentMaps();
	float TotalWeight = 0.0f;
	float CurWeightSum = 0.0f;
	float PickedWeight = 0.0f;
	TArray<FRoomComponentSpec>* CompArray = nullptr;
	FRandomStream* GridStreamFound = nullptr;
	ATR_GameMode* GameMode = Cast<ATR_GameMode>(GetWorld()->GetAuthGameMode());
	if (IsValid(GameMode))
	{
		GridStreamFound = &GameMode->GetGridStream();
	}
	else
	{
		GridStreamFound = &DefaultGridRandStream;
	}
	if (Type == ETRRoomComponentType::Wall)
	{
		CompArray = &RoomWallComponentArray;
	}
	else if (Type == ETRRoomComponentType::Door)
	{
		CompArray = &RoomDoorComponentArray;
	}
	else {
		UE_LOG(LogTRGame, Error, TEXT("RoomPlatformGridMgr GetRoomComponentActorInArray - called with invalid room type %s"), *GetEnumValueAsString<ETRRoomComponentType>(Type));
		bFound = false;
		return nullptr;
	}
	if (CompArray && CompArray->Num() > 0)
	{
		if (CompArray->Num() > 1)
		{
			// Sum total weight
			TotalWeight = 0.0f;
			for (FRoomComponentSpec Spec : (*CompArray)) {
				TotalWeight += Spec.PickWeight;
			}
			// Find the one at the picked weight.
			PickedWeight = GridStreamFound->FRandRange(0.0f, TotalWeight);
			CurWeightSum = 0.0f;
			for (FRoomComponentSpec Spec : (*CompArray))
			{
				CurWeightSum += Spec.PickWeight;
				if (CurWeightSum >= PickedWeight)
				{
					//UE_LOG(LogTRGame, Log, TEXT("RoomGridMgr GetRoomComponentActorInArray picked room component %s from %d choices for room X:%d Y:%d (picked weight: %f)"),
					//	*Spec.ComponentActor->GetName(),
					//	CompArray->Num(),
					//	RoomCoords.X,
					//	RoomCoords.Y,
					//	PickedWeight);
					bFound = true;
					return Spec.ComponentActor;
				}
			}
		}
		else
		{
			bFound = true;
			return (*CompArray)[0].ComponentActor;
		}
	}
	else
	{
		UE_LOG(LogTRGame, Warning, TEXT("RoomPlatformGridMgr GetRoomComponentActorInArray - No room component of type %s"), *GetEnumValueAsString<ETRRoomComponentType>(Type));
	}
	bFound = false;
	return nullptr;
}

TSubclassOf<ARoomComponentActor> ARoomPlatformGridMgr::GetRoomComponentActor_Implementation(const ETRRoomComponentType Type, const FIntPoint RoomCoords, FRoomExitInfo& ExitInfo, bool& bFound)
{
	ExitInfo = URoomFunctionLibrary::GetRoomExitInfo(RoomGridTemplate, RoomCoords);
	if (ExitInfo.ExitLayout == ETRRoomExitLayout::None)
	{
		UE_LOG(LogTRGame, Error, TEXT("RoomPlatformGridMgr - Could not get room exit info for room X:%d Y:%d. Exit layout was None."), RoomCoords.X, RoomCoords.Y);
		bFound = false;
		return ARoomComponentActor::StaticClass();
	}
	if (Type == ETRRoomComponentType::Floor || Type == ETRRoomComponentType::Ceiling) {
		return GetRoomComponentActorInLayoutMap(Type, ExitInfo.ExitLayout, bFound);
	}
	else if (Type == ETRRoomComponentType::Wall || Type == ETRRoomComponentType::Door) {
		return GetRoomComponentActorInArray(Type, RoomCoords, bFound);
	}
	UE_LOG(LogTRGame, Error, TEXT("RoomPlatformGridMgr - Could not get room exit info for room X:%d Y:%d. Room component type was invalid %s."), RoomCoords.X, RoomCoords.Y, *GetEnumValueAsString<ETRRoomComponentType>(Type));
	bFound = false;
	return ARoomComponentActor::StaticClass();
}


void ARoomPlatformGridMgr::ClearRoomComponentCaches()
{
	RoomFloorComponentMap.Empty();
	RoomCeilingComponentMap.Empty();
	RoomWallComponentArray.Empty();
	RoomDoorComponentArray.Empty();
}