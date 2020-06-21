// Fill out your copyright notice in the Description page of Project Settings.

#include "PlatformGridMgr.h"
#include "RoomPlatformBase.h"
#include "TargetRunner.h"
#include "Kismet/GameplayStatics.h"
#include "TR_GameMode.h"

// Sets default values
APlatformGridMgr::APlatformGridMgr()
{
	bReplicates = true;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void APlatformGridMgr::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APlatformGridMgr, GridCellWorldSize, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(APlatformGridMgr, GridExtentMinX, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(APlatformGridMgr, GridExtentMinY, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(APlatformGridMgr, GridExtentMaxX, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(APlatformGridMgr, GridExtentMaxY, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(APlatformGridMgr, StartGridCoords, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(APlatformGridMgr, ExitGridCoords, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(APlatformGridMgr, RoomCellSubdivision, COND_InitialOnly);

}

// Called when the game starts or when spawned
void APlatformGridMgr::BeginPlay()
{
	Super::BeginPlay();	
}

void APlatformGridMgr::MovePlayerStarts()
{
	FVector Offset(150.0, -200, 32.0);
	int32 MaxPlayerStarts = 4;
	APlayerStart* PlayerStart;
	FTransform SpawnTransform;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// First, destroy any we'd already spawned.
	for (APlayerStart* tmpPlayerStart : PlayerStarts)
	{
		tmpPlayerStart->Destroy();
	}
	PlayerStarts.Empty(MaxPlayerStarts);

	//UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
	//for (AActor* TmpActor : PlayerStarts)
	for (int32 CurStart = 1; CurStart <= MaxPlayerStarts; CurStart++)
	{
		SpawnTransform = GetGridCellWorldTransform(StartGridCoords);
		SpawnTransform.AddToTranslation(Offset);
		SpawnTransform.SetRotation(FRotator(0.0, FMath::FRandRange(0.0, 360.0), 0.0).Quaternion());
		PlayerStart = GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), SpawnTransform, SpawnParams);
		if (PlayerStart)
		{
			PlayerStart->PlayerStartTag = FName(FString::Printf(TEXT("Player%d.RoomStart"), CurStart));
			PlayerStarts.Add(PlayerStart);
			if (CurStart % 3 == 0)
			{
				Offset.Set(150.0 - ((CurStart / 3) * 150.0), -200, 32.0);
			}
			else
			{
				Offset += FVector(0.0, 200.0, 0.0);
			}
		}
		PlayerStart = nullptr;
		//CurStart++;
	}	
}

// Called every frame
void APlatformGridMgr::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlatformGridMgr::FillGridFromExistingPlatforms()
{
	TArray<AActor*> FoundPlatforms;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlatformBase::StaticClass(), FoundPlatforms);

	for (AActor* TActor : FoundPlatforms)
	{
		APlatformBase* Platform = Cast<APlatformBase>(TActor);
		AddPlatformToGridMap(Platform);
	}
}

void APlatformGridMgr::ClientFillGridFromExistingPlatforms_Implementation()
{
	// Multicast replication but only the clients need to update.
	if (GetLocalRole() < ROLE_Authority)
	{
		FillGridFromExistingPlatforms();
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

	UE_LOG(LogTRGame, Log, TEXT("%s GenerateGrid - Generating grid."), *GetNameSafe(this));

	// TODO Seed this correctly - currently uses values added in editor.
	//GridRandStream.GenerateNewSeed();
	ATR_GameMode* GameMode = Cast<ATR_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		FRandomStream& GridRandStream = GameMode->GetGridStream();
		GridRandStream.Reset();
		UE_LOG(LogTRGame, Log, TEXT("GridRandStream seed: %d"), GridRandStream.GetInitialSeed());	
	}
	else
	{
		DefaultGridRandStream.Reset();
		UE_LOG(LogTRGame, Error, TEXT("%s GenerateGrid - Could not get GameMode."), *GetNameSafe(this));
		//return;
	}

	GenerateGridImpl();	
}

void APlatformGridMgr::GenerateGridImpl()
{
	// TODO Implement for base class
}

void APlatformGridMgr::DestroyGrid_Implementation()
{
	DestroyGridImpl();
	if (GetLocalRole() == ROLE_Authority)
	{
		// Destroy any player starts we spawned.
		for (APlayerStart* tmpPlayerStart : PlayerStarts)
		{
			tmpPlayerStart->Destroy();
		}
		PlayerStarts.Empty();

		// Have the clients update their grid maps
		ClientFillGridFromExistingPlatforms();
	}
}

void APlatformGridMgr::DestroyGridImpl()
{
	TArray<int32> RowNums;
	TArray<int32> PlatformNums;
	PlatformGridMap.GenerateKeyArray(RowNums);

	DebugLog(FString::Printf(TEXT("Destroying %d rows."), RowNums.Num()));
	for (int32 Row : RowNums)
	{
		PlatformGridMap.Find(Row)->RowPlatforms.GenerateKeyArray(PlatformNums);
		for (int32 Col : PlatformNums)
		{
			APlatformBase* Platform = PlatformGridMap.Find(Row)->RowPlatforms[Col];
			if (IsValid(Platform))
			{
				DebugLog(FString::Printf(TEXT("Destroying room X:%d Y:%d."), Platform->GridX, Platform->GridY));
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

APlatformBase* APlatformGridMgr::GetPlatformNeighbor(const FVector2D& MyCoords, const ETRDirection DirectionToNeighbor)
{
	APlatformBase* Platform = nullptr;
	bool bFound = false;
	switch (DirectionToNeighbor)
	{
	case ETRDirection::North:
		Platform = GetPlatformInGrid(MyCoords + FVector2D(1.0f, 0.0f), bFound);
		break;
	case ETRDirection::East:
		Platform = GetPlatformInGrid(MyCoords + FVector2D(0.0f, 1.0f), bFound);
		break;
	case ETRDirection::South:
		Platform = GetPlatformInGrid(MyCoords + FVector2D(-1.0f, 0.0f), bFound);
		break;
	case ETRDirection::West:
		Platform = GetPlatformInGrid(MyCoords + FVector2D(0.0f, -1.0f), bFound);
	}
	if (!bFound) { return nullptr; }
	return Platform;
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

int32 APlatformGridMgr::GetPlatformCount()
{
	int32 TotalCount = 0;
	TArray<int32> RowNums;
	TArray<int32> PlatformNums;
	PlatformGridMap.GenerateKeyArray(RowNums);
	for (int32 RowNum : RowNums)
	{
		TotalCount += PlatformGridMap.Find(RowNum)->RowPlatforms.Num();
	}
	return TotalCount;
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
