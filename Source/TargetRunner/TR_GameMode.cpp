// Fill out your copyright notice in the Description page of Project Settings.


#include "TR_GameMode.h"

ATR_GameMode::ATR_GameMode()
	: Super()
{
	GoodsDropper = CreateDefaultSubobject<UGoodsDropper>(TEXT("GoodsDropper"));
	if (IsValid(GoodsDropperTable)) { GoodsDropper->AddDropTableDataToLibrary(GoodsDropperTable); }
}

void ATR_GameMode::BeginPlay()
{
	Super::BeginPlay();
	//GeneratorRandStream.Reset();
	GridRandStream.Initialize(GeneratorRandStream.RandRange(1, 20000) * 10000);
}

bool ATR_GameMode::InitGridManager_Implementation(APlatformGridMgr* GridManager)
{
	FVector2D MinExtents;
	FVector2D MaxExtents;	
	GetGridExtents(MinExtents, MaxExtents);
	GridManager->GridExtentMinX = MinExtents.X;
	GridManager->GridExtentMaxX = MaxExtents.X;
	GridManager->GridExtentMinY = MinExtents.Y;
	GridManager->GridExtentMaxY = MaxExtents.Y;
	GridManager->GridCellWorldSize = GridCellSize;

	return true;
}

void ATR_GameMode::GetGridExtents(FVector2D& MinExtents, FVector2D& MaxExtents)
{
	int32 ExtentMaxX = static_cast<int32>(FMath::GetMappedRangeValueClamped(FVector2D(0.0, 100.0), FVector2D(2.0, 6.0), LevelDifficulty));
	int32 ExtentMinX = -ExtentMaxX;
	int32 ExtentMaxY = static_cast<int32>(FMath::GetMappedRangeValueClamped(FVector2D(0.0, 100.0), FVector2D(1.0, 5.0), LevelDifficulty));
	int32 ExtentMinY = -ExtentMaxY;
	MinExtents.Set(ExtentMinX, ExtentMinY);
	MaxExtents.Set(ExtentMaxX, ExtentMaxY);
}


float ATR_GameMode::FRandRangeGrid(const float Min, const float Max)
{
	return GridRandStream.FRandRange(Min, Max);
}

int32 ATR_GameMode::RandRangeGrid(const int32 Min, const int32 Max)
{
	return GridRandStream.RandRange(Min, Max);
}

void ATR_GameMode::ToolClassByName_Implementation(const FName ToolName, TSubclassOf<ATRToolBase>& ToolClass, bool& bValid)
{
	if (ToolClassMap.Contains(ToolName))
	{
		ToolClass = ToolClassMap.FindRef(ToolName);
		bValid = IsValid(ToolClass);
	}
	else
	{
		bValid = false;
	}
}
