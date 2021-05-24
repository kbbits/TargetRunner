
#include "RoomPlatformBase.h"
#include "..\PlatformGridMgr.h"
#include "TrEnums.h"
#include "Math/Vector2D.h"
#include "UnrealNetwork.h"
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
	DOREPLIFETIME(ARoomPlatformBase, WallTemplate);
	DOREPLIFETIME(ARoomPlatformBase, FloorTemplate);
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


void ARoomPlatformBase::OnRep_RoomTemplate_Implementation()
{
	bRoomTemplateSet = true;
}


void ARoomPlatformBase::DestroyPlatformImpl()
{
	Super::DestroyPlatformImpl();
	for (AActor* CurSpecial : SpawnedSpecialActors)
	{
		CurSpecial->Destroy();
	}
	SpawnedSpecialActors.Empty();
}

void ARoomPlatformBase::GenerateRoom_Implementation()
{
	if (MyGridManager == nullptr)
	{
		GetGridManager();
	}
	if (CalculateWalls())
	{
		SpawnFloor();
		SpawnWalls();		
		//SpawnContents(); // Calls SpawnResources
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


bool ARoomPlatformBase::SpawnContents_Implementation()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (!SpawnResources())
		{
			return false;
		}
		return SpawnSpecials();
	}
	else
	{
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
	GetGridManager();
	if (!MyGridManager)
	{
		UE_LOG(LogTRGame, Error, TEXT("%s - SpawnSpecials found null grid manager."), *GetNameSafe(this));
		return false;
	}
	if (MyGridManager->RoomCellSubdivision <= 0)
	{
		UE_LOG(LogTRGame, Error, TEXT("%s - SpawnSpecials RoomCellSubdivision is 0."), *GetNameSafe(this));
		return false;
	}

	FTransform SpawnTransform;
	TArray<int32> OpenIndexes;
	int32 CurOpenIndex;
	FIntPoint SubCellCoords;
	int32 ArraySize = MyGridManager->RoomCellSubdivision * MyGridManager->RoomCellSubdivision;
	FActorSpawnParameters SpawnParams;
	AActor* NewSpecialActor;
	bool bHasHit;
	FHitResult Hit;
	FVector TraceStart;

	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = this;

	for (int32 i = 0; i < ArraySize; i++)
	{
		if (FloorTemplate[i] == ETRFloorState::Open)
		{
			OpenIndexes.Add(i);
		}
	}
	if (OpenIndexes.Num() == 0)
	{
		UE_LOG(LogTRGame, Error, TEXT("%s - SpawnSpecials no open sub-grid cells found"), *GetNameSafe(this));
		return false;
	}
	for (TSubclassOf<AActor> SpecialActorClass : RoomTemplate.SpecialActors)
	{
		CurOpenIndex = OpenIndexes[PlatformRandStream.RandRange(0, OpenIndexes.Num() - 1)];
		SubCellCoords.Y = FMath::TruncToInt(CurOpenIndex / MyGridManager->RoomCellSubdivision);
		SubCellCoords.X = CurOpenIndex % MyGridManager->RoomCellSubdivision;
		SpawnTransform = MyGridManager->GetGridCellSubGridWorldTransform(GetGridCoordinates(), SubCellCoords);
		SpawnTransform.SetRotation(FQuat::FindBetweenVectors(FVector(1.0f, 0.0f, 0.0f), FVector(1.0f, 0.0f, 0.0f).RotateAngleAxis(PlatformRandStream.RandRange(0, 3) * 90, FVector(0.0f, 0.0f, 1.0f))));

		TraceStart = SpawnTransform.GetLocation() + FVector(0.0f, 0.0f, 10000.0f);
		bHasHit = GetWorld()->SweepSingleByChannel(Hit, TraceStart, SpawnTransform.GetLocation(), FQuat::Identity, ECollisionChannel::ECC_Visibility, FCollisionShape::MakeBox(FVector(100, 100, 100)));
		if (bHasHit)
		{
			SpawnTransform.SetLocation(FVector(SpawnTransform.GetLocation().X, SpawnTransform.GetLocation().Y, Hit.ImpactPoint.Z));
		}		
		NewSpecialActor = GetWorld()->SpawnActor<AActor>(SpecialActorClass, SpawnTransform, SpawnParams);

		SpawnedSpecialActors.Add(NewSpecialActor);
	}
	return true;
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


ETRFloorState ARoomPlatformBase::GetSubGridFloorState(FIntPoint SubGridCoords)
{
	GetGridManager();
	if (MyGridManager == nullptr)
	{
		return ETRFloorState::Unknown;
	}
	if (FloorTemplate.Num() < (SubGridCoords.X + 1) * (SubGridCoords.Y + 1))
	{
		return ETRFloorState::Unknown;
	}
	int32 Index = (SubGridCoords.Y * MyGridManager->RoomCellSubdivision) + SubGridCoords.X;
	if (!FloorTemplate.IsValidIndex(Index))
	{
		return ETRFloorState::Unknown;
	}
	return FloorTemplate[Index];
}


ETRFloorState ARoomPlatformBase::SetSubGridFloorState(FIntPoint SubGridCoords, ETRFloorState NewState)
{
	ETRFloorState OldState;
	int32 Index = (SubGridCoords.Y * MyGridManager->RoomCellSubdivision) + SubGridCoords.X;
	GetGridManager();
	if (MyGridManager == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("%s - SetSubGridFloorState found null grid manager"), *GetNameSafe(this));
		return ETRFloorState::Unknown;
	}
	if (!FloorTemplate.IsValidIndex(Index))
	{
		UE_LOG(LogTRGame, Error, TEXT("%s - SetSubGridFloorState invalid index, %d"), *GetNameSafe(this), Index);
		return ETRFloorState::Unknown;
	}
	OldState = FloorTemplate[Index];
	FloorTemplate[Index] = NewState;
	return OldState;
}
