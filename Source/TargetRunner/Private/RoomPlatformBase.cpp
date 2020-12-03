
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
		return SpawnResources();
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
