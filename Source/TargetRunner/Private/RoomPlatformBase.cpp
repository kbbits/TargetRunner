
#include "RoomPlatformBase.h"
#include "RoomPlatformGridMgr.h"
#include "RoomComponentActor.h"
#include "ObjectPlacer.h"
#include "ObjectPlacerProxyBox.h"
#include "TrEnums.h"
#include "Math/Vector2D.h"
#include "UnrealNetwork.h"
// For intellisense:
#include "..\Public\RoomPlatformBase.h"

// Sets default values
ARoomPlatformBase::ARoomPlatformBase()
{
	bReplicates = true;
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bRoomTemplateSet = false;
}


void ARoomPlatformBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARoomPlatformBase, RoomTemplate);
	//DOREPLIFETIME(ARoomPlatformBase, WallTemplate);
}


// Called when the game starts or when spawned
void ARoomPlatformBase::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void ARoomPlatformBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ARoomPlatformBase::ServerSetRoomTemplate_Implementation(const FRoomTemplate& NewRoomTemplate)
{
	RoomTemplate = NewRoomTemplate;
	// Manually call OnRep here on server
	OnRep_RoomTemplate();
}

bool ARoomPlatformBase::ServerSetRoomTemplate_Validate(const FRoomTemplate& NewRoomTemplate)
{
	return true;
}

/*
void ARoomPlatformBase::OnRep_WallTemplate_Implementation()
{
	if (WallTemplate.Num() > 0 && bRoomTemplateSet)
	{
		if (GetLocalRole() < ROLE_Authority && bGenerateOnClient)
		{
			// Have clients generate the room too.
			GenerateRoomImpl();
		}
	}
}*/


void ARoomPlatformBase::OnRep_RoomTemplate_Implementation()
{
	int32 RoomCellSubdivision;
	ARoomPlatformGridMgr* RoomGridManager;
	if (MyGridManager == nullptr)
	{
		GetGridManager();
	}
	RoomGridManager = Cast<ARoomPlatformGridMgr>(MyGridManager);
	if (!IsValid(RoomGridManager)) {
		UE_LOG(LogTRGame, Error, TEXT("OnRep_RoomTemplate - could not get valid room grid manager"));
		return;
	}
	if (PlatformRandStream.GetInitialSeed() != RoomTemplate.RoomRandSeed) {
		PlatformRandStream.Initialize(RoomTemplate.RoomRandSeed);
	}
	RoomCellSubdivision = RoomGridManager->RoomCellSubdivision;

	// Create appropriate wall state arrays representing the used wall types, sized according to RoomCellSubdivision.
	int32 DoorSection;
	TArray<ETRWallState> TmpWallTemplate;
	// Put the door in the center of the wall by default
	DoorSection = (RoomCellSubdivision / 2);
	TArray<ETRWallState> DoorWall;
	DoorWall.Reserve(RoomCellSubdivision);
	for (int i = 0; i < RoomCellSubdivision; i++)
	{
		if (i == DoorSection) { DoorWall.Add(ETRWallState::Door); }
		else { DoorWall.Add(ETRWallState::Blocked); }
	}
	TArray<ETRWallState> SolidWall;
	SolidWall.Reserve(RoomCellSubdivision);
	for (int i = 0; i < RoomCellSubdivision; i++)
	{
		SolidWall.Add(ETRWallState::Blocked);
	}
	TArray<ETRWallState> EmptyWall;
	EmptyWall.Reserve(RoomCellSubdivision);
	for (int i = 0; i < RoomCellSubdivision; i++)
	{
		EmptyWall.Add(ETRWallState::Empty);
	}

	// Create wall template, sized so each of the four walls gets [RoomCellSubdivision] entries.
	TmpWallTemplate.Empty(4 * RoomCellSubdivision);
	TArray<ETRWallState> Walls;
	Walls.Add(RoomTemplate.NorthWall);
	Walls.Add(RoomTemplate.EastWall);
	Walls.Add(RoomTemplate.SouthWall);
	Walls.Add(RoomTemplate.WestWall);
	// Append wall segments for each wall. 
	for (ETRWallState WallState : Walls)
	{
		if (WallState == ETRWallState::Door && !RoomGridManager->bDoorsAtWallCenter)
		{
			// Put the door in a random wall section
			DoorSection = PlatformRandStream.RandRange(0, RoomCellSubdivision - 1);
			for (int i = 0; i < RoomCellSubdivision; i++)
			{
				if (i == DoorSection) { DoorWall[i] = ETRWallState::Door; }
				else { DoorWall[i] = ETRWallState::Blocked; }
			}
		}
		TmpWallTemplate.Append(WallState == ETRWallState::Blocked ? SolidWall : (WallState == ETRWallState::Door ? DoorWall : EmptyWall));
	}
	WallTemplate = TmpWallTemplate;
	bRoomTemplateSet = true;
	if (GetLocalRole() < ROLE_Authority && bGenerateOnClient)
	{
		// Have clients generate the room too.
		GenerateRoomImpl();
		SpawnContents();
	}
}


void ARoomPlatformBase::DestroyPlatformImpl()
{
	DestroyResourcePlacers(true);
	DestroySpecialPlacers(true);
	DestroyClutterPlacers(true);
	Super::DestroyPlatformImpl();
}


void ARoomPlatformBase::ServerGenerateRoom_Implementation()
{
	GenerateRoomImpl();
}


void ARoomPlatformBase::GenerateRoomImpl()
{
	if (MyGridManager == nullptr)
	{
		GetGridManager();
	}
	if (CalculateWalls())
	{
		SpawnFloor();
		SpawnWalls();
		SpawnCeiling();
	}
}


bool ARoomPlatformBase::CalculateWalls()
{
	if (MyGridManager)
	{
		float CellSize = MyGridManager->GridCellWorldSize;
		float CellHalfSize = CellSize / 2;
		float SubCellSize = CellSize / MyGridManager->RoomCellSubdivision;
		float FirstOffset = SubCellSize / 2;
		FVector2D Offsets;
		FVector2D WallDirection;
		FVector Margin;
		FRotator CurrentRotation = FRotator(0.0, 0.0, 0.0);
		FVector DefaultScale(1.0, 1.0, 1.0);
		FTransform RoomTransform = PlatformScene->GetComponentTransform();

		// Empty any previous transforms, size the array to hold what we will need.
		WallSectionTransforms.Empty(MyGridManager->RoomCellSubdivision * 4);
		// For each of the four walls: North(+X), East(+Y), South(-X), West(-Y)
		for (int32 w = 0; w < 4; w++) {
			switch (w)
			{
			case 0:
				Offsets.Set(1.0, -1.0);
				WallDirection.Set(0.0, 1.0);
				Margin.Set(-50.0, 0.0, 0.0);
				break;
			case 1:
				Offsets.Set(1.0, 1.0);
				WallDirection.Set(-1.0, 0.0);
				Margin.Set(0.0, -50.0, 0.0);
				break;
			case 2:
				Offsets.Set(-1.0, 1.0);
				WallDirection.Set(0.0, -1.0);
				Margin.Set(50.0, 0.0, 0.0);
				break;
			case 3:
				Offsets.Set(-1.0, -1.0);
				WallDirection.Set(1.0, 0.0);
				Margin.Set(0.0, 50.0, 0.0);
				break;
			}
			FVector WallStartLocalLocation = FVector(Offsets.X * CellHalfSize, Offsets.Y * CellHalfSize, 0.0);
			FVector SectionLocation(WallStartLocalLocation.X + ((SubCellSize / 2) * WallDirection.X), WallStartLocalLocation.Y + ((SubCellSize/2) * WallDirection.Y), 0.0);
			for (int32 i = 0; i < MyGridManager->RoomCellSubdivision; i++)
			{
				// Add the transform.
				WallSectionTransforms.Add(FTransform(CurrentRotation, SectionLocation, DefaultScale) * RoomTransform);
				// Move section location to next
				SectionLocation += FVector(SubCellSize * WallDirection.X, SubCellSize * WallDirection.Y, 0.0);
			}
			CurrentRotation.Yaw += 90;
		}
		return true;
	}
	else {
		UE_LOG(LogTRGame, Error, TEXT("%s - CalculateWalls found null grid manager"), *GetNameSafe(this));
		return false;
	}
}


void ARoomPlatformBase::SpawnWalls_Implementation()
{
	// Override and implement in BP.
}


bool ARoomPlatformBase::SpawnFloor_Implementation()
{
	// Override and implement in BP.
	return true;
}


bool ARoomPlatformBase::SpawnCeiling_Implementation()
{
	// Override and implement in BP.
	return true;
}

bool ARoomPlatformBase::SpawnContents_Implementation()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (!SpawnResources())
		{
			UE_LOG(LogTRGame, Log, TEXT("RoomPlatformBase %s SpawnResources failed"), *GetName());
			return false;
		}
		DestroyResourcePlacers();
		if (!SpawnSpecials())
		{
			UE_LOG(LogTRGame, Log, TEXT("RoomPlatformBase %s SpawnSpecials failed"), *GetName());
			return false;
		}
		DestroySpecialPlacers();
		if (!SpawnClutter())
		{
			UE_LOG(LogTRGame, Log, TEXT("RoomPlatformBase %s SpawnClutter failed"), *GetName());
			return false;
		}		
		DestroyClutterPlacers();
		return true;
	}
	else
	{
		DestroyResourcePlacers();
		DestroySpecialPlacers();
		DestroyClutterPlacers();
		return true;
	}
}


bool ARoomPlatformBase::SpawnResources_Implementation()
{
	// Override and implement in BP.
	return true;
}


bool ARoomPlatformBase::SpawnSpecials_Implementation()
{
	// Don't need to do anything if there are no special actors to spawn.
	if (RoomTemplate.SpecialActors.Num() == 0) { return true; }
	
	TArray<AActor*> Placers;
	AObjectPlacer* Placer;
	int32 Index;
	bool bSuccess;
	AActor* NewSpecialActor;
	
	// Find all the special actor object placers.
	PlatformControlZone->GetOverlappingActors(Placers, AObjectPlacerSpecialActor::StaticClass());
	if (Placers.Num() == 0)
	{
		UE_LOG(LogTRGame, Warning, TEXT("%s - SpawnSpecials no placers found in room (%d, %d)"), *GetNameSafe(this), GridX, GridY);
		return true;
	}
	// For each special actor to place, pick a placer and have it place one.
	for (auto Itr(RoomTemplate.SpecialActors.CreateIterator()); Itr; Itr++)
	{
		if (Placers.Num() == 0) { break; }
		Index = PlatformRandStream.RandRange(0, Placers.Num() - 1);
		Placer = Cast<AObjectPlacerSpecialActor>(Placers[Index]);
		if (Placer)
		{
			Placer->DefaultClassToPlace = Itr->Get();
			NewSpecialActor = Placer->PlaceOne(PlatformRandStream, this, bSuccess);
			if (NewSpecialActor)
			{
				PlatformActorCache.Add(FName(FString::Printf(TEXT("SpecialActor_%d"), Itr.GetIndex())), NewSpecialActor);
			}
		}
		else
		{
			UE_LOG(LogTRGame, Warning, TEXT("%s - SpawnSpecials invalid placer for %s in room (%d, %d)"), *GetNameSafe(this), *GetNameSafe(Itr->Get()), GridX, GridY);
		}
		Placers.RemoveAt(Index);
	}
	return true;
}


bool ARoomPlatformBase::SpawnClutter_Implementation()
{
	TArray<AActor*> Placers;
	AObjectPlacerProxyBoxClutter* Placer;
	//bool bSuccess;
	//AActor* NewClutterActor;
	TArray<AActor*> NewClutterActors;
	FRandomStream RandStream;
	RandStream.GenerateNewSeed();

	UE_LOG(LogTRGame, Log, TEXT("%s - SpawnClutter in room (%d, %d)"), *GetNameSafe(this), GridX, GridY);

	// Find all the special actor object placers.
	PlatformControlZone->GetOverlappingActors(Placers, AObjectPlacerProxyBoxClutter::StaticClass());
	if (Placers.Num() == 0)
	{
		UE_LOG(LogTRGame, Warning, TEXT("%s - SpawnClutter no AObjectPlacerProxyBoxClutter placers found in room (%d, %d)"), *GetNameSafe(this), GridX, GridY);
		return true;
	}
	// For each clutter placer, have it place it's actors
	for (AActor* PlacerActor : Placers)
	{
		Placer = Cast<AObjectPlacerProxyBoxClutter>(PlacerActor);
		if (Placer)
		{
			if (!IsValid(Placer->GetClassToPlace())) {
				UE_LOG(LogTRGame, Warning, TEXT("Clutter object placer %s has invalid ClassToPlace"), *Placer->GetName());
				continue;
			}
			//NewClutterActor = Placer->PlaceOne(PlatformRandStream, this, bSuccess);
			NewClutterActors = Placer->PlaceAll(RandStream, nullptr);
			for (AActor* NewClutterActor : NewClutterActors)
			{
				PlatformActorCache.Add(FName(FString::Printf(TEXT("Clutter_%s"), *FGuid::NewGuid().ToString())), NewClutterActor);
			}
		}
	}
	return true;
}


bool ARoomPlatformBase::AllTrackedRoomComponentsSetup()
{
	TWeakObjectPtr<ARoomComponentActor> RoomComponent;
	for (int32 i = 0; i < SpawnedRoomComponents.Num(); i++)
	{
		// Check all the room components with valid refs
		RoomComponent = SpawnedRoomComponents[i];
		if (RoomComponent.IsValid()) {
			// Check that ISMs are copied
			if (!RoomComponent.Get()->AllISMsCopiedOut()) {
				return false;
			}
		}
	}
	return true;
}


int32 ARoomPlatformBase::TrackRoomComponentActor(ARoomComponentActor* RoomComponentActor)
{
	SpawnedRoomComponents.Add(RoomComponentActor);
	return SpawnedRoomComponents.Num();
}


int32 ARoomPlatformBase::GetTrackedRoomComponentActors(TArray<ARoomComponentActor*>& RoomComponentActors)
{
	int32 NumValid = 0;
	TWeakObjectPtr<ARoomComponentActor> RoomComponent;
	for (int32 i = 0; i < SpawnedRoomComponents.Num(); i++)
	{
		// Get all the room components with valid refs
		RoomComponent = SpawnedRoomComponents[i];
		if (RoomComponent.IsValid() && IsValid(RoomComponent.Get())) {
			RoomComponentActors.Add(RoomComponent.Get());
			NumValid++;
		}
	}
	return NumValid;
}


int32 ARoomPlatformBase::DestroyTrackedRoomComponents()
{
	int32 NumDestroyed = 0;
	TWeakObjectPtr<ARoomComponentActor> RoomComponent;
	for (int32 i = 0; i < SpawnedRoomComponents.Num(); i++)
	{
		// Destroy all the room components with valid refs
		RoomComponent = SpawnedRoomComponents[i];
		if (RoomComponent.IsValid()) {
			RoomComponent.Get()->Destroy();
			NumDestroyed++;
		}
	}
	SpawnedRoomComponents.Empty();
	return NumDestroyed;
}


ARoomPlatformBase* ARoomPlatformBase::GetConnectedNeighbor(const ETRDirection Direction)
{
	if (MyGridManager == nullptr)
	{
		GetGridManager();
	}
	bool bConnected = false;
	ARoomPlatformBase* NeighborRoom = Cast<ARoomPlatformBase>(MyGridManager->GetPlatformNeighbor(GetGridCoordinates(), Direction));
	if (NeighborRoom == nullptr) { return nullptr; }
	switch (Direction)
	{
	case ETRDirection::North :
		bConnected = (RoomTemplate.NorthWall != ETRWallState::Blocked && NeighborRoom->RoomTemplate.SouthWall != ETRWallState::Blocked);
		break;
	case ETRDirection::East :
		bConnected = (RoomTemplate.EastWall != ETRWallState::Blocked && NeighborRoom->RoomTemplate.WestWall != ETRWallState::Blocked);
		break;
	case ETRDirection::South :
		bConnected = (RoomTemplate.SouthWall != ETRWallState::Blocked && NeighborRoom->RoomTemplate.NorthWall != ETRWallState::Blocked);
		break;
	case ETRDirection::West :
		bConnected = (RoomTemplate.WestWall != ETRWallState::Blocked && NeighborRoom->RoomTemplate.EastWall != ETRWallState::Blocked);
		break;
	}
	return bConnected ? NeighborRoom : nullptr;
}


void ARoomPlatformBase::DestroyResourcePlacers_Implementation(const bool bDestroySpawns)
{
	TArray<AActor*> AllPlacers;
	AObjectPlacerResource* Placer;
	// Find all the special actor object placers.
	PlatformControlZone->GetOverlappingActors(AllPlacers, AObjectPlacerResource::StaticClass());
	for (AActor* PlacerActor : AllPlacers)
	{
		Placer = Cast<AObjectPlacerResource>(PlacerActor);
		if (Placer) {
			if (bDestroySpawns) { Placer->ClearPlaced(); }
			Placer->Destroy();
		}
	}
	TInlineComponentArray<UChildActorComponent*> CACs;
	TSubclassOf<AActor> ChildClass;
	GetComponents(CACs, false);
	for (UChildActorComponent* CAC : CACs)
	{
		if (CAC->GetChildActorClass().Get()->IsChildOf(AObjectPlacerResource::StaticClass()))
		{
			CAC->SetChildActorClass(nullptr);
		}
	}
}


void ARoomPlatformBase::DestroySpecialPlacers_Implementation(const bool bDestroySpawns)
{
	TArray<AActor*> AllPlacers;
	AObjectPlacerSpecialActor* Placer;
	// Find all the special actor object placers.
	PlatformControlZone->GetOverlappingActors(AllPlacers, AObjectPlacerSpecialActor::StaticClass());
	for (AActor* PlacerActor : AllPlacers)
	{
		Placer = Cast<AObjectPlacerSpecialActor>(PlacerActor);
		if (Placer) {
			if (bDestroySpawns) { Placer->ClearPlaced(); }
			Placer->Destroy(); 
		}
	}
	TInlineComponentArray<UChildActorComponent*> CACs;
	TSubclassOf<AActor> ChildClass;
	GetComponents(CACs, false);
	for (UChildActorComponent* CAC : CACs)
	{
		if (CAC->GetChildActorClass().Get()->IsChildOf(AObjectPlacerSpecialActor::StaticClass()))
		{
			CAC->SetChildActorClass(nullptr);
		}
	}
}


void ARoomPlatformBase::DestroyClutterPlacers_Implementation(const bool bDestroySpawns)
{
	TArray<AActor*> AllPlacers;
	AObjectPlacerProxyBoxClutter* Placer;
	// Find all the clutter object placers.
	PlatformControlZone->GetOverlappingActors(AllPlacers, AObjectPlacerProxyBoxClutter::StaticClass());
	for (AActor* PlacerActor : AllPlacers)
	{
		Placer = Cast<AObjectPlacerProxyBoxClutter>(PlacerActor);
		if (Placer) {
			if (bDestroySpawns) { Placer->ClearPlaced(); }
			Placer->Destroy(); 
		}
	}
	TInlineComponentArray<UChildActorComponent*> CACs;
	TSubclassOf<AActor> ChildClass;
	GetComponents(CACs, false);
	for (UChildActorComponent* CAC : CACs)
	{
		if (CAC->GetChildActorClass().Get()->IsChildOf(AObjectPlacerProxyBoxClutter::StaticClass()))
		{
			CAC->SetChildActorClass(nullptr);
		}
	}
}

