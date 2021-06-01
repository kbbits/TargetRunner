// Fill out your copyright notice in the Description page of Project Settings.


#include "TR_GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "TRGameInstance.h"
#include "RoomPlatformGridMgr.h"
#include "ResourceDropperBase.h"
#include "ResourceFunctionLibrary.h"
#include "GridForgePrim.h"
#include "GridForgeManual.h"

// Initializes rand streams and creates the GoodsDropper.
ATR_GameMode::ATR_GameMode(const FObjectInitializer& OI) 
	: Super(OI)
{
	bLevelTemplateReady = false;
	GeneratorRandStream.Reset();
	GridRandStream.Reset();
	GoodsDropper = CreateDefaultSubobject<UGoodsDropper>(TEXT("GoodsDropper"));
	if (GoodsDropper != nullptr)
	{
		if (IsValid(GoodsDropperTable)) 
		{ 
			GoodsDropper->AddDropTableDataToLibrary(GoodsDropperTable); 
		}
		else 
		{
			UE_LOG(LogTRGame, Error, TEXT("TRGameMode constructor GoodsDropperTable was not valid."));
		}
		if (IsValid(ResourceDropTable))
		{
			GoodsDropper->AddDropTableDataToLibrary(ResourceDropTable);
		}
		else
		{
			UE_LOG(LogTRGame, Error, TEXT("TRGameMode constructor ResourceDropTable was not valid."));
		}
	}
	else 
	{ 
		UE_LOG(LogTRGame, Error, TEXT("TRGameMode constructor Could not create GoodsDropper.")); 
	}
}


void ATR_GameMode::BeginPlay()
{
	Super::BeginPlay();	
	UGameInstance* GameInst = GetGameInstance();
	UTRGameInstance* TRGameInst = Cast<UTRGameInstance>(GetGameInstance());
	if (GameInst)
	{   
		if (TRGameInst) 
		{
			// Debug log
			UE_LOG(LogTRGame, Log, TEXT("TRGameMode - BeginPlay setting level template"));
			SetNewLevelTemplate(TRGameInst->GetSelectedLevelTemplate());
		}
		else
		{
			UE_LOG(LogTRGame, Log, TEXT("TRGameMode - BeginPlay incorrect game instance class %s"), *GameInst->GetClass()->GetName());
		}
	}
	else {
		UE_LOG(LogTRGame, Error, TEXT("GameMode - BeginPlay - Could not get game instance."));
	}
}

void ATR_GameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
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
	if (NewTemplate.LevelSeed == 0) { UE_LOG(LogTRGame, Warning, TEXT("GameMode - SetNewLevelTemplate - Template has 0 seed.")); }
	LevelTemplate = NewTemplate;
	// Debug log
	UE_LOG(LogTRGame, Log, TEXT("TRGameMode - SetNewLevelTemplate %s ID: %s"), *LevelTemplate.DisplayName.ToString(), *LevelTemplate.LevelId.ToString());
	UE_LOG(LogTRGame, Log, TEXT("TRGameMode - SetNewLevelTemplate seed: %d"), LevelTemplate.LevelSeed);
	InitGoodsDropper();
	ReseedAllStreams(LevelTemplate.LevelSeed);
	bLevelTemplateReady = true;
}


FLevelTemplate ATR_GameMode::GetLevelTemplate()
{
	return LevelTemplate;
}


void ATR_GameMode::InitGoodsDropper_Implementation()
{
	if (GoodsDropper == nullptr)
	{
		GoodsDropper = NewObject<UGoodsDropper>(this, FName(TEXT("GoodsDropper")));
	}
	if (GoodsDropper)
	{
		if (IsValid(GoodsDropperTable))
		{
			GoodsDropper->AddDropTableDataToLibrary(GoodsDropperTable);
		}
		else
		{
			UE_LOG(LogTRGame, Error, TEXT("TRGameMode InitGoodsDropper GoodsDropperTable not valid."));
		}
		if (IsValid(ResourceDropTable))
		{
			GoodsDropper->AddDropTableDataToLibrary(ResourceDropTable);
		}
		else
		{
			UE_LOG(LogTRGame, Error, TEXT("TRGameMode InitGoodsDropper ResourceDropTable was not valid."));
		}
	}
}


void ATR_GameMode::GetClutterDropGoods(TArray<FGoodsQuantity>& DroppedGoods)
{
	// Clutter table names in format Clutter<level tier>. ex: Clutter01
	FName DropTableName; 
	if (bLevelTemplateReady)
	{
		DropTableName = FName(FString::Printf(TEXT("Clutter%02d"), static_cast<int32>(LevelTemplate.Tier)));
	}
	else
	{
		DropTableName = FName(TEXT("Clutter00"));
	}
	DroppedGoods.Empty();
	if (GoodsDropper)
	{
		DroppedGoods.Append(GoodsDropper->EvaluateGoodsDropTableByName(DropTableName));
	}
	else
	{
		UE_LOG(LogTRGame, Error, TEXT("TRGameMode GetClutterDropGoods - Null GoodsDropper."));
	}
}


void ATR_GameMode::GetClutterDropResources(TArray<FResourceQuantity>& DroppedResources)
{
	// Clutter resource drop table names in format ClutterResources<level tier>. ex: ClutterResources01
	FName DropTableName;
	FResourceType TmpResourceType;
	TArray<FGoodsQuantity> DroppedGoods;
	DroppedResources.Empty();
	if (bLevelTemplateReady)
	{
		DropTableName = FName(FString::Printf(TEXT("ClutterResources%02d"), static_cast<int32>(LevelTemplate.Tier)));
	}
	else
	{
		DropTableName = FName(TEXT("ClutterResources00"));
	}	
	if (GoodsDropper)
	{
		// Get resources by using goods droppper to drop resource code named goods.
		DroppedGoods.Append(GoodsDropper->EvaluateGoodsDropTableByName(DropTableName));
		// Translate each of those goods to a resource quantity
		for (FGoodsQuantity CurGoods : DroppedGoods)
		{
			if (CurGoods.Quantity > 0.0f)
			{
				UResourceFunctionLibrary::ResourceTypeForCode(CurGoods.Name, TmpResourceType);
				if (TmpResourceType.IsValid())
				{
					DroppedResources.Add(FResourceQuantity(TmpResourceType, CurGoods.Quantity));
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTRGame, Error, TEXT("TRGameMode GetClutterDropGoods - Null GoodsDropper."));
	}
	/*FResourceTypeData TmpResourceTypeData;
	ATR_GameState* TRGameState = GetGameState<ATR_GameState>();
	FRandomStream& RandStream = GetResourceDropperStream();
	if (bLevelTemplateReady && TRGameState)
	{
		for (int i = 0; i < LevelTemplate.ResourcesAvailable.Num(); i++)
		{
			FResourceQuantity LevelQuantity = LevelTemplate.ResourcesAvailable[i];
			if(TRGameState->GetResourceTypeData(LevelQuantity.ResourceType, TmpResourceTypeData))
			{
				if (TmpResourceTypeData.Tier < LevelTemplate.Tier)
				{
					RandStream.FRandRange(LevelTemplate.Tier - TmpResourceTypeData.Tier, (LevelTemplate.Tier - TmpResourceTypeData.Tier) * LevelTemplate.Tier);
				}
				else if (TmpResourceTypeData.Tier == LevelTemplate.Tier)
				{
				}
				else
				{ // > LevelTier

				}
			}
		}
	} */
}


bool ATR_GameMode::InitGridManager_Implementation()
{
	// Initializes our reference
	GetGridManager();
	if (GridManager == nullptr) { 
		UE_LOG(LogTRGame, Warning, TEXT("TRGameMode::InitGridManager could not find any grid manager actors")); 
		return false;
	}
	if (!GetLevelTemplate().IsValid()) { 
		UE_LOG(LogTRGame, Warning, TEXT("TRGameMode::InitGridManager invalid level tempate. LevelSeed: %d"), LevelTemplate.LevelSeed);
		return false; 
	}

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
		RoomGridManager->ResourceDropperClass = GetResourceDropperClass();
		RoomGridManager->ResourcesToDistribute = LevelTemplate.ResourcesAvailable;
		RoomGridManager->SpecialsToDistribute = LevelTemplate.SpecialsAvailable;
	}

	UE_LOG(LogTRGame, Log, TEXT("TRGameMode::InitGridManager - extents: MinX:%d MinY:%d  MaxX:%d MaxY:%d"), (int32)MinExtents.X, (int32)MinExtents.Y, (int32)MaxExtents.X, (int32)MaxExtents.Y)
	return true;
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


// Must set a Level Template before extents will be valid.
void ATR_GameMode::GetGridExtents(FVector2D& MinExtents, FVector2D& MaxExtents)
{
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


bool ATR_GameMode::SpawnLevel_Implementation()
{
	GetGridManager();
	if (GridManager == nullptr) {
		UE_LOG(LogTRGame, Error, TEXT("SpawnLevel - No valid grid manager"));
		return false;
	}
	if (!LevelTemplate.IsValid()) {
		UE_LOG(LogTRGame, Error, TEXT("SpawnLevel - invalid level tempate. LevelSeed: %d"), LevelTemplate.LevelSeed);
		return false;
	}
		
	// If it is a RoomGridManager, spawn the rooms.
	ARoomPlatformGridMgr* RoomGridManager = Cast<ARoomPlatformGridMgr>(GridManager);
	if (RoomGridManager != nullptr)
	{
		RoomGridManager->SpawnRooms();
	}
	return true;
}


void ATR_GameMode::ReseedAllStreams_Implementation(const int32 NewSeed)
{
	GeneratorRandStream.Initialize(NewSeed);
	// Save five places here in case we need to use the stream for new code later, it won't change the other stream seeds.
	for (int32 i = 0; i < 5; i++)
	{
		GeneratorRandStream.RandRange(1, INT_MAX - 1);
		//UE_LOG(LogTRGame, Log, TEXT("    random int: %d"), GeneratorRandStream.RandRange(1, INT_MAX - 1));
	}
	UE_LOG(LogTRGame, Log, TEXT("ReseedAllStreams - Generator seed: %d"), GeneratorRandStream.GetInitialSeed());	
	GridRandStream.Initialize(GeneratorRandStream.RandRange(1, INT_MAX - 1));
	UE_LOG(LogTRGame, Log, TEXT("ReseedAllStreams - Grid seed: %d"), GridRandStream.GetInitialSeed());
	ResourceDropperRandStream.Initialize(GeneratorRandStream.RandRange(1, INT_MAX - 1));
	UE_LOG(LogTRGame, Log, TEXT("ReseedAllStreams - ResourceDropper seed: %d"), ResourceDropperRandStream.GetInitialSeed());
	if (GoodsDropper != nullptr)
	{
		int32 GoodsDropperSeed = GeneratorRandStream.RandRange(1, INT_MAX - 1);
		GoodsDropper->SeedRandomStream(GoodsDropperSeed);
		UE_LOG(LogTRGame, Log, TEXT("ReseedAllStreams - GoodsDropper seed: %d"), GoodsDropperSeed);
	}
}


FRandomStream& ATR_GameMode::GetGeneratorStream()
{
	return GeneratorRandStream;
}


FRandomStream& ATR_GameMode::GetGridStream()
{
	return GridRandStream;
}

FRandomStream& ATR_GameMode::GetResourceDropperStream()
{
	return ResourceDropperRandStream;
}


float ATR_GameMode::FRandRangeGrid(const float Min, const float Max)
{
	return GridRandStream.FRandRange(0.0f, Max - Min) + Min;
}


int32 ATR_GameMode::RandRangeGrid(const int32 Min, const int32 Max)
{
	return GridRandStream.RandRange(0, Max - Min) + Min;
}


void ATR_GameMode::ToolClassByName_Implementation(const FName ToolName, TSubclassOf<AToolActorBase>& ToolClass, bool& bValid)
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

TSubclassOf<UResourceDropperBase> ATR_GameMode::GetResourceDropperClass()
{
	return DefaultResourceDropperClass;
}


TSubclassOf<ARoomPlatformBase> ATR_GameMode::GetRoomClass_Implementation()
{
	if (LevelTemplate.IsValid())
	{
		// TODO: set room class based on LevelTemplate.Theme
		return DefaultRoomClass;
	}
	return DefaultRoomClass;
}