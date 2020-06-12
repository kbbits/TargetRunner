// Fill out your copyright notice in the Description page of Project Settings.


#include "TR_GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "RoomPlatformGridMgr.h"
#include "GridForgePrim.h"
#include "GridForgeManual.h"

// Initializes rand streams and creates the GoodsDropper.
ATR_GameMode::ATR_GameMode(const FObjectInitializer& OI) 
	: Super(OI)
{
	GeneratorRandStream.Reset();
	GridRandStream.Reset();
	GoodsDropper = OI.CreateDefaultSubobject<UGoodsDropper>(this, TEXT("GoodsDropper"));
	if (IsValid(GoodsDropperTable)) { GoodsDropper->AddDropTableDataToLibrary(GoodsDropperTable); }
}


void ATR_GameMode::BeginPlay()
{
	Super::BeginPlay();
	
}


//APlatformGridMgr* ATR_GameMode::SpawnNewGridManager(const TSubclassOf<APlatformGridMgr> GridManagerClass, const FTransform Transform)
//{
//	if (GridManager == nullptr)
//	{
//		FActorSpawnParameters SpawnParams;
//		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
//		SpawnParams.Owner = this;
//
//		UE_LOG(LogTRGame, Verbose, TEXT("%s SpawnNewGridManager - Spanwing %s actor at %s."), *this->GetName(), *GridManagerClass.Get()->GetName(), *Transform.GetLocation().ToString());
//		GridManager = GetWorld()->SpawnActor<APlatformGridMgr>(GridManagerClass, Transform, SpawnParams);
//	}
//	return GridManager;
//}


void ATR_GameMode::SetNewLevelTemplate(const FLevelTemplate& NewTemplate)
{
	LevelTemplate = NewTemplate;
	ReseedAllStreams(LevelTemplate.LevelSeed);
}


FLevelTemplate ATR_GameMode::GetLevelTemplate()
{
	return LevelTemplate;
}


APlatformGridMgr* ATR_GameMode::GetGridManager()
{
	if (GridManager == nullptr)
	{
		TArray<AActor*> FoundManagers;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlatformGridMgr::StaticClass(), FoundManagers);
		if (FoundManagers.Num() > 0)
		{
			GridManager = Cast<APlatformGridMgr>(FoundManagers[0]);
		}
	}	
	return GridManager;
}


bool ATR_GameMode::InitGridManager_Implementation()
{
	// Initializes our reference
	GetGridManager();
	if (GridManager == nullptr || !LevelTemplate.IsValid()) { return false; }

	FVector2D MinExtents;
	FVector2D MaxExtents;
	GetGridExtents(MinExtents, MaxExtents);
	GridManager->GridExtentMinX = MinExtents.X;
	GridManager->GridExtentMaxX = MaxExtents.X;
	GridManager->GridExtentMinY = MinExtents.Y;
	GridManager->GridExtentMaxY = MaxExtents.Y;
	GridManager->GridCellWorldSize = GridCellSize;
	GridManager->RoomCellSubdivision = RoomCellSubdivision;
	// start and exit cells are set during generate grid.
	ARoomPlatformGridMgr* RoomGridManager = Cast<ARoomPlatformGridMgr>(GridManager);
	if (RoomGridManager != nullptr)
	{
		RoomGridManager->GridForgeClass = GetGridForgeClass();
		RoomGridManager->RoomClass = GetRoomClass();
	}	
	return true;
}


// Must set a Level Template before extents will be valid.
void ATR_GameMode::GetGridExtents(FVector2D& MinExtents, FVector2D& MaxExtents)
{
	// Random extents
	//int32 ExtentMaxX = RandRangeGrid(1, 10);
	//int32 ExtentMinX = -ExtentMaxX;
	//int32 ExtentMaxY = RandRangeGrid(1, 10);
	//int32 ExtentMinY = -ExtentMaxY;
	//MinExtents.Set(ExtentMinX, ExtentMinY);
	//MaxExtents.Set(ExtentMaxX, ExtentMaxY);

	if (LevelTemplate.IsValid())
	{
		MinExtents.Set(LevelTemplate.MinX, LevelTemplate.MinY);
		MaxExtents.Set(LevelTemplate.MaxX, LevelTemplate.MaxY);
	}
	else
	{
		UE_LOG(LogTRGame, Error, TEXT("%s GetGridExtents - Cannot get extents, LevelTemplate is not valid."), *this->GetName());
		MinExtents.Set(0.0f, 0.0f);
		MaxExtents.Set(0.0f, 0.0f);
	}
}


void ATR_GameMode::ReseedAllStreams_Implementation(const int32 NewSeed)
{
	GeneratorRandStream.Initialize(NewSeed);
	// Save a slot here
	GeneratorRandStream.RandRange(0, 1);
	//GridRandStream.Initialize(GeneratorRandStream.RandRange(1, 20000) * 10000);
	GridRandStream.Initialize(GeneratorRandStream.RandRange(1, INT_MAX));
}

FRandomStream& ATR_GameMode::GetGeneratorStream()
{
	return GeneratorRandStream;
}

FRandomStream& ATR_GameMode::GetGridStream()
{
	return GridRandStream;
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

// Returns the grid forge for the current level templte.
// Returns UGridForgePrim class if level template is not valid.
TSubclassOf<UGridForgeBase> ATR_GameMode::GetGridForgeClass()
{
	if (LevelTemplate.IsValid())
	{
		if (LevelTemplate.GridForgeType == FName(TEXT("Default")))
			return DefaultGridForgeClass;
		if (LevelTemplate.GridForgeType == FName(TEXT("Prim")))
			return UGridForgePrim::StaticClass();
		if (LevelTemplate.GridForgeType == FName(TEXT("Manual")))
			return UGridForgeManual::StaticClass();
		return UGridForgeBase::StaticClass();
	}
	return UGridForgePrim::StaticClass();
}

TSubclassOf<ARoomPlatformBase> ATR_GameMode::GetRoomClass_Implementation()
{
	if (LevelTemplate.IsValid())
	{
		// TODO: set room class based on LevelTemplate.Theme
	}
	return DefaultRoomClass;
}