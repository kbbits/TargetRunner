// Fill out your copyright notice in the Description page of Project Settings.

#include "PlatformGridMgr.h"
#include "RoomPlatformBase.h"
#include "TargetRunner.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

// Sets default values
APlatformGridMgr::APlatformGridMgr()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APlatformGridMgr::BeginPlay()
{
	Super::BeginPlay();	
}

void APlatformGridMgr::MovePlayerStarts()
{
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
	for (AActor* TmpActor : PlayerStarts)
	{
		APlayerStart* PlayerStart = Cast<APlayerStart>(TmpActor);
		PlayerStart->AddActorWorldOffset(GetGridCellWorldTransform(StartGridCoords).GetLocation());
		PlayerStart->AddActorWorldRotation(FRotator(0.0, FMath::FRandRange(0.0, 360.0), 0.0));
	}	
}

// Called every frame
void APlatformGridMgr::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlatformGridMgr::Setup()
{
	TArray<AActor*> FoundPlatforms;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlatformBase::StaticClass(), FoundPlatforms);

	for (AActor* TActor : FoundPlatforms)
	{
		APlatformBase* Platform = Cast<APlatformBase>(TActor);
		AddPlatformToGridMap(Platform);
	}
}

FTransform APlatformGridMgr::GetGridCellWorldTransform(const FVector2D& GridCoords)
{
	FVector CellLocation = GetActorLocation();
	CellLocation += FVector(GridCoords.X * GridCellWorldSize, GridCoords.Y * GridCellWorldSize, 0.0).RotateAngleAxis(GetActorRotation().Yaw, FVector(0.0, 0.0, 1.0));
	return FTransform(GetActorRotation(), CellLocation, FVector(1.0, 1.0, 1.0));
}

void APlatformGridMgr::GenerateGrid_Implementation()
{
	// Destroy old grid, if any
	DestroyGrid();

	UE_LOG(LogTRGame, Log, TEXT("%s GenerateGrid - Generating grid."), *this->GetName());

	GenerateGridImpl();
}

void APlatformGridMgr::GenerateGridImpl()
{
	// TODO Implement for base class
}

void APlatformGridMgr::DestroyGrid_Implementation()
{
	DestroyGridImpl();
}

void APlatformGridMgr::DestroyGridImpl()
{
	TArray<int32> RowNums;
	TArray<int32> PlatformNums;
	PlatformGridMap.GenerateKeyArray(RowNums);

	UE_LOG(LogTRGame, Warning, TEXT("Destroying %d rows."), RowNums.Num());
	for (int32 Row : RowNums)
	{
		PlatformGridMap.Find(Row)->RowPlatforms.GenerateKeyArray(PlatformNums);
		for (int32 Col : PlatformNums)
		{
			APlatformBase* Platform = PlatformGridMap.Find(Row)->RowPlatforms[Col];
			if (IsValid(Platform))
			{
				UE_LOG(LogTRGame, Warning, TEXT("Destroying room X:%d Y:%d."), Platform->GridX, Platform->GridY);
				Platform->Destroy();
			}
		}
		PlatformGridMap.Find(Row)->RowPlatforms.Empty();
	}
	PlatformGridMap.Empty();
}

void APlatformGridMgr::AddPlatformToGridMap(APlatformBase* Platform)
{
	if (Platform != nullptr) {
		int32 row = Platform->GridX;
		int32 col = Platform->GridY;
		Platform->MyGridManager = this;

		if (!PlatformGridMap.Contains(row))
		{
			FPlatformGridRow newRow;
			PlatformGridMap.Add(row, newRow);
		}
		PlatformGridMap[row].RowPlatforms.Add(col, Platform);
	}
}

APlatformBase * APlatformGridMgr::GetPlatformInGridMap(const int32 X, const int32 Y, bool& Found)
{
	Found = false;
	if (PlatformGridMap.Contains(X)) {
		if (PlatformGridMap[X].RowPlatforms.Contains(Y))
		{
			Found = true;
			return PlatformGridMap[X].RowPlatforms[Y];
		}	
	}
	return nullptr;
}

APlatformBase* APlatformGridMgr::GetPlatformInGrid(const FVector2D Coords, bool& Found)
{
	return GetPlatformInGridMap(static_cast<int32>(Coords.X), static_cast<int32>(Coords.Y), Found);
}

APlatformBase * APlatformGridMgr::RemovePlatformFromGridMap(const int32 X, const int32 Y, bool& Success)
{
	Success = false;
	APlatformBase *Platform = GetPlatformInGridMap(X, Y, Success);
	if (Success && Platform != nullptr) {
		PlatformGridMap[X].RowPlatforms.Remove(Y);
		Success = true;
	}
	return Platform;
}

APlatformBase* APlatformGridMgr::RemovePlatformFromGrid(const FVector2D Coords, bool& Success)
{
	return RemovePlatformFromGridMap(static_cast<int32>(Coords.X), static_cast<int32>(Coords.Y), Success);
}

int32 APlatformGridMgr::GetGridWidthX()
{
	// Add one for the zero row
	return (GridExtentMaxX - GridExtentMinX) + 1;
}

int32 APlatformGridMgr::GetGridWidthY()
{
	// Add one for the zero row
	return (GridExtentMaxY - GridExtentMinY) + 1;
}

bool APlatformGridMgr::IsInGrid(const FVector2D Coords)
{
	int32 X = static_cast<int32>(Coords.X);
	int32 Y = static_cast<int32>(Coords.Y);
	if (X < GridExtentMinX || X > GridExtentMaxX) return false;
	if (Y < GridExtentMinY || Y > GridExtentMaxY) return false;
	return true;
}

bool APlatformGridMgr::IsInGridSwitch(FVector2D Coords, EInGrid& InGrid)
{
	if (IsInGrid(Coords)) {	InGrid = EInGrid::InGrid; }
	else { InGrid = EInGrid::NotInGrid;	}
	return InGrid == EInGrid::InGrid;
}

int32 APlatformGridMgr::GridCoordsToCellNumber(const FVector2D Coords)
{
	if (!IsInGrid(Coords)) return -1;
	//      Cells in prior rows                                                   Cells in this row before us
	return ((static_cast<int32>(Coords.X) - GridExtentMinX) * GetGridWidthY()) + (static_cast<int32>(Coords.Y) - GridExtentMinY);
}
