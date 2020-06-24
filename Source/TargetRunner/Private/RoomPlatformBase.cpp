
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
	
	RoomRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RoomRoot);
	RoomSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RoomScene"));
	RoomSceneComponent->SetupAttachment(GetRootComponent());
	FloorMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMeshComponent"));
	FloorMeshComponent->AttachToComponent(RoomSceneComponent, FAttachmentTransformRules::KeepRelativeTransform);
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

void ARoomPlatformBase::GenerateRoom_Implementation()
{
	if (CalculateWalls())
	{
		SpawnWalls();
		SpawnContents(); // Calls SpawnResources
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
		FRotator CurrentRotation = FRotator(0.0, 0.0, 0.0); // GetActorRotation(); //+ FRotator(0.0, -90.0, 0.0);
		FVector DefaultScale(1.0, 1.0, 1.0);

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
				WallSectionTransforms.Add(FTransform(CurrentRotation, SectionLocation, DefaultScale));
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


bool ARoomPlatformBase::SpawnContents_Implementation()
{
	return SpawnResources();
}


bool ARoomPlatformBase::SpawnResources_Implementation()
{
	// Override and implement in BP.
	return true;
}


void ARoomPlatformBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(ARoomPlatformBase, RoomCellSubdivision);
}

