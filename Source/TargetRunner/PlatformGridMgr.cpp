// Fill out your copyright notice in the Description page of Project Settings.

#include "PlatformGridMgr.h"
#include "RoomPlatformBase.h"
#include "TargetRunner.h"
#include "TRMath.h"
#include "Kismet/GameplayStatics.h"
#include "TR_GameMode.h"

// Sets default values
APlatformGridMgr::APlatformGridMgr()
{
	bReplicates = true;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StasisWakeRange = 1;
}


void APlatformGridMgr::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME_CONDITION(APlatformGridMgr, GridCellWorldSize, COND_InitialOnly);
	//DOREPLIFETIME_CONDITION(APlatformGridMgr, GridExtentMinX, COND_InitialOnly);
	//DOREPLIFETIME_CONDITION(APlatformGridMgr, GridExtentMinY, COND_InitialOnly);
	//DOREPLIFETIME_CONDITION(APlatformGridMgr, GridExtentMaxX, COND_InitialOnly);
	//DOREPLIFETIME_CONDITION(APlatformGridMgr, GridExtentMaxY, COND_InitialOnly);
	//DOREPLIFETIME_CONDITION(APlatformGridMgr, StartGridCoords, COND_InitialOnly);
	//DOREPLIFETIME_CONDITION(APlatformGridMgr, ExitGridCoords, COND_InitialOnly);
	//DOREPLIFETIME_CONDITION(APlatformGridMgr, RoomCellSubdivision, COND_InitialOnly);

	DOREPLIFETIME(APlatformGridMgr, GridCellWorldSize);
	DOREPLIFETIME(APlatformGridMgr, GridExtentMinX);
	DOREPLIFETIME(APlatformGridMgr, GridExtentMinY);
	DOREPLIFETIME(APlatformGridMgr, GridExtentMaxX);
	DOREPLIFETIME(APlatformGridMgr, GridExtentMaxY);
	DOREPLIFETIME(APlatformGridMgr, StartGridCoords);
	DOREPLIFETIME(APlatformGridMgr, ExitGridCoords);
	DOREPLIFETIME(APlatformGridMgr, RoomCellSubdivision);
}


// Called when the game starts or when spawned
void APlatformGridMgr::BeginPlay()
{
	Super::BeginPlay();	
}


void APlatformGridMgr::MovePlayerStarts()
{
	FVector Offset(150.0, -200, 100.0);
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
				Offset.Set(150.0 - ((CurStart / 3) * 150.0), -200, 100.0);
			}
			else
			{
				Offset += FVector(0.0, 200.0, 0.0);
			}
		}
		PlayerStart = nullptr;
	}	
}


// Called every frame
void APlatformGridMgr::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (GetLocalRole() == ROLE_Authority) 
	{
		ServerISMQueueTime += DeltaTime;
		if (ServerISMQueue.Num() > 0)
		{			
			if (ServerISMQueueTime >= ServerISMQueueMaxTime)
			{
				ServerISMQueueTime = 0.0f;
				TArray<FISMContext> ISMContexts;
				int32 Size = 0;
				if (ServerISMQueue.Num() > ISMQueueMaxBatchSize) 
				{
					ISMContexts.Empty(ISMQueueMaxBatchSize);
					for (int32 i = 0; i < ISMQueueMaxBatchSize; i++)
					{
						Size += sizeof(ServerISMQueue[i].Mesh);
						Size += ServerISMQueue[i].SpawnTransforms.GetTypeSize() * ServerISMQueue[i].SpawnTransforms.Num();
						ISMContexts.Add(ServerISMQueue[i]);
					}
					ServerISMQueue.RemoveAt(0, ISMQueueMaxBatchSize, false);
				}
				else {
					ISMContexts.Append(ServerISMQueue);
					ServerISMQueue.Empty(25);
				}
				UE_LOG(LogTRGame, Log, TEXT("PlatformGridMgr::Tick - Processing %d ISM contexts in queue"), ISMContexts.Num());
				UE_LOG(LogTRGame, Log, TEXT(" ISMContext size %d"), sizeof(ISMContexts));
				UE_LOG(LogTRGame, Log, TEXT(" ISMContext batch size %d bytes"), ISMContexts.GetTypeSize() * ISMContexts.Num());
				UE_LOG(LogTRGame, Log, TEXT(" ISMContext calculated size %d bytes"), Size);
				MC_SpawnISMs(ISMContexts);
			}
		}
		else if (ServerISMQueueTime > 120.0f) {
			ServerISMQueueTime = ServerISMQueueMaxTime;
		}
	}
	
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


FTransform APlatformGridMgr::GetGridCellSubGridWorldTransform(const FVector2D& GridCoords, const FVector2D& SubGridCoords)
{
	float CellSize = GridCellWorldSize;
	float GridCellHalfSize = GridCellWorldSize / 2.0f;
	float SubCellSize = GridCellWorldSize / RoomCellSubdivision;
	float SubCellOffset = SubCellSize / 2.0f;
	FVector CellLocation = GetActorLocation();
	FVector GridCellLocation = FVector(GridCoords.X * GridCellWorldSize, GridCoords.Y * GridCellWorldSize, 0.0);
	GridCellLocation.X = (((GridCoords.X * GridCellWorldSize) - GridCellHalfSize) + SubCellOffset) + (SubGridCoords.X * SubCellSize);
	GridCellLocation.Y = (((GridCoords.Y * GridCellWorldSize) - GridCellHalfSize) + SubCellOffset) + (SubGridCoords.Y * SubCellSize);
	GridCellLocation.Z = 0.0f;
	CellLocation += GridCellLocation.RotateAngleAxis(GetActorRotation().Yaw, FVector(0.0, 0.0, 1.0));
	return FTransform(GetActorRotation(), CellLocation, FVector(1.0, 1.0, 1.0));
}


void APlatformGridMgr::GenerateGrid_Implementation()
{
	// Destroy old grid, if any
	DestroyGrid();
	
	UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("%s PlatformGridMgr::GenerateGrid - Generating grid."), *GetNameSafe(this));

	ATR_GameMode* GameMode = Cast<ATR_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		FRandomStream& GridRandStream = GameMode->GetGridStream();
		GridRandStream.Reset();
		UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("PlatformGridManager::GenerateGrid - GridRandStream seed: %d"), GridRandStream.GetInitialSeed());
	}
	else
	{
		DefaultGridRandStream.Reset();
		UE_LOG(LogTRGame, Warning, TEXT("%s PlatformGridManager::GenerateGrid - Could not get GameMode."), *GetNameSafe(this));
	}

	GenerateGridImpl();	
}


void APlatformGridMgr::GenerateGridImpl()
{
	// TODO Implement something for base class.
}


void APlatformGridMgr::DestroyGrid_Implementation()
{
	DestroyGridImpl();
	if (GetLocalRole() == ROLE_Authority)
	{
		UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("DestroyGrid destroying spawned player starts: %d."), PlayerStarts.Num());
		// Destroy any player starts we spawned.
		for (APlayerStart* tmpPlayerStart : PlayerStarts)
		{
			tmpPlayerStart->Destroy();
		}
		PlayerStarts.Empty();

		// Have the clients update their grid maps
		//ClientFillGridFromExistingPlatforms();
	}
}


void APlatformGridMgr::DestroyGridImpl()
{
	TArray<int32> RowNums;
	TArray<int32> PlatformNums;
	TInlineComponentArray<UInstancedStaticMeshComponent*> ISMComps;
	GetComponents(ISMComps, false);
	for (UInstancedStaticMeshComponent* ISMComp : ISMComps)
	{
		ISMComp->ClearInstances();
		ISMComp->UnregisterComponent();
		if (IsValid(ISMComp)) { ISMComp->DestroyComponent(); }
	}

	PlatformGridMap.GenerateKeyArray(RowNums);
	UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("Destroying %d rows."), RowNums.Num());
	for (int32 Row : RowNums)
	{
		PlatformGridMap.Find(Row)->RowPlatforms.GenerateKeyArray(PlatformNums);
		for (int32 Col : PlatformNums)
		{
			APlatformBase* Platform = PlatformGridMap.Find(Row)->RowPlatforms[Col];
			if (IsValid(Platform))
			{
				UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("Destroying room X:%d Y:%d."), Platform->GridX, Platform->GridY);
				Platform->DestroyPlatform();
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
	Platform = GetPlatformInGrid(MyCoords + UTRMath::DirectionToOffsetVector(DirectionToNeighbor), bFound);
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


void APlatformGridMgr::WakeNeighbors(const FVector2D AroundGridCoords)
{
	// Call native Impl function.
	WakeNeighborsImpl(AroundGridCoords);
}

void APlatformGridMgr::WakeNeighborsImpl(const FVector2D AroundGridCoords)
{
	bool bFound;
	const TArray<ETRDirection> OrthogonalDirections = { ETRDirection::North, ETRDirection::East, ETRDirection::South, ETRDirection::West };
	FVector2D OffsetCoords;
	APlatformBase* CurPlatform = nullptr;	
	APlatformBase* AroundPlatform = GetPlatformInGrid(AroundGridCoords, bFound);

	// Orthogonal directions only
	if (bFound && AroundPlatform && !AroundPlatform->bStasisWokeNeighbors)
	{
		for (ETRDirection Direction : OrthogonalDirections)
		{
			OffsetCoords = AroundGridCoords;
			for (int32 i = 1; i <= StasisWakeRange; i++)
			{
				CurPlatform = GetPlatformNeighbor(OffsetCoords, Direction);
				if (CurPlatform)
				{
					CurPlatform->StasisWakeActors();
				}
				OffsetCoords = OffsetCoords + UTRMath::DirectionToOffsetVector(Direction);
			}
		}
		AroundPlatform->bStasisWokeNeighbors = true;
	}
	
	// All directions
	//APlatformBase* AroundPlatform = GetPlatformInGrid(AroundGridCoords, bFound);
	//if (bFound && AroundPlatform && !AroundPlatform->bStasisWokeNeighbors)
	//{
	//	for (int32 i = -StasisWakeRange; i <= StasisWakeRange; i++)
	//	{
	//		OffsetCoords.X = i;
	//		for (int32 j = -StasisWakeRange; j <= StasisWakeRange; j++)
	//		{
	//			OffsetCoords.Y = j;
	//			// Skip the center one
	//			if (OffsetCoords.IsZero()) { continue; }
	//			// Get the neighbor
	//			CurPlatform = GetPlatformInGrid(AroundGridCoords + OffsetCoords, bFound);
	//			if (bFound && CurPlatform)
	//			{
	//				CurPlatform->StasisWakeActors();
	//			}
	//		}
	//	}
	//	AroundPlatform->bStasisWokeNeighbors = true;
	//}
}



void APlatformGridMgr::SpawnISMs(const TArray<FISMContext>& ISMContexts)
{
	UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("PlatformGridMgr::SpawnISMs - Spawning %d ISM contexts"), ISMContexts.Num());
	for (FISMContext ISMContext : ISMContexts)
	{
		if (ISMContext.SpawnTransforms.Num() == 0) {
			UE_LOG(LogTRGame, Warning, TEXT("PlatformGridMgr::SpawnISMs - Got ISMContext %s with 0 spawn transforms"), *ISMContext.MeshPath.ToString());
		}
		else {
			UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("PlatformGridMgr::SpawnISMs - Adding ISMContext %s with %d spawn transforms"), *ISMContext.MeshPath.ToString(), ISMContext.SpawnTransforms.Num());
		}
		
		UStaticMesh* MeshRef = nullptr;
		if (ISMContext.Mesh.IsValid()) {
			MeshRef = ISMContext.Mesh.Get();
		}
		if (MeshRef == nullptr) 
		{
			UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("PlatformGridMgr::SpawnISMs - Getting mesh from soft path %s"), *ISMContext.MeshPath.ToString());
			MeshRef = Cast<UStaticMesh>(ISMContext.MeshPath.ResolveObject());
			if (MeshRef == nullptr) 
			{
				UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("PlatformGridMgr::SpawnISMs - Loading mesh from soft path %s"), *ISMContext.MeshPath.ToString());
				MeshRef = Cast<UStaticMesh>(ISMContext.MeshPath.TryLoad());
			}
		}
		if (MeshRef)
		{
			TSoftObjectPtr<UStaticMesh> MeshSOPtr(MeshRef);
			FSoftObjectPath MeshSOPath(MeshRef);
			if (ISMContext.MeshPath != MeshSOPath)
			{
				UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("PlatformGridMgr::SpawnISMs - changing mesh path from %s to %s"), *ISMContext.MeshPath.ToString(), *MeshSOPath.ToString());
				ISMContext.MeshPath = MeshSOPath;
			}
			UMaterialInterface* MatRef = MeshRef->GetMaterial(0);
			if (MatRef)
			{
				for (FTransform SpawnTransform : ISMContext.SpawnTransforms) {
					SpawnISM(MeshSOPtr, MatRef, SpawnTransform);
				}
				ISMContext.Mesh.Reset(); // TODO: Remove this?
			}
			else {
				UE_LOG(LogTRGame, Error, TEXT("PlatformGridMgr::SpawnISMs - Could not get material for mesh %s"), *ISMContext.MeshPath.ToString());
			}
		}
		else {
			UE_LOG(LogTRGame, Error, TEXT("PlatformGridMgr::SpawnISMs - Could not load static mesh %s"), *ISMContext.MeshPath.ToString());
		}
	}
	if (GetLocalRole() == ROLE_Authority && bEnableClientISMs) {
		ServerISMQueue.Append(ISMContexts);
	}
}


void APlatformGridMgr::MC_SpawnISMs_Implementation(const TArray<FISMContext>& ISMContexts)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("MC_SpawnISMs - spawning %d ISM contexts"), ISMContexts.Num());
		for (FISMContext ISMContext : ISMContexts)
		{
			if (ISMContext.SpawnTransforms.Num() == 0) {
				UE_LOG(LogTRGame, Warning, TEXT("PlatformGridMgr::MC_SpawnISMs - Got ISMContext %s with 0 spawn transforms"), *ISMContext.MeshPath.ToString());
			}
		}
		SpawnISMs(ISMContexts);
	}
}


bool APlatformGridMgr::MC_SpawnISMs_Validate(const TArray<FISMContext>& ISMContexts)
{
	return true;
}


int32 APlatformGridMgr::SpawnISM(UPARAM(ref) TSoftObjectPtr<UStaticMesh> Mesh, UPARAM(ref) UMaterialInterface* Material, const FTransform& SpawnTransform)
{
	if (!Material->CheckMaterialUsage_Concurrent(EMaterialUsage::MATUSAGE_InstancedStaticMeshes))
	{
		UE_LOG(LogTRGame, Error, TEXT("PlatformGridMgr SpawnISM called with a material not set for use with ISM: %s"), *Material->GetPathName());
		return -1;
	}

	UStaticMesh* MeshRef = Mesh.Get();
	if (!MeshRef)
	{
		UE_LOG(LogTRGame, Error, TEXT("GridManager SpawnISM could not load static mesh %s"), *Mesh.GetAssetName());
		return -1;
	}
	TInlineComponentArray<UInstancedStaticMeshComponent*> ISMComps;
	UInstancedStaticMeshComponent* FoundISMComp = nullptr;
	GetComponents(ISMComps, false);
	for (UInstancedStaticMeshComponent* ISMComp : ISMComps)
	{
		if (ISMComp->GetStaticMesh() && ISMComp->GetStaticMesh()->GetPathName().Equals(MeshRef->GetPathName()))
		{
			if (ISMComp->GetMaterial(0)->GetPathName().Equals(Material->GetPathName()))
			{
				FoundISMComp = ISMComp;
				break;
			}
		}
	}
	if (!FoundISMComp)
	{
		UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("GridManager creating new ISM component for mesh %s"), *Mesh.GetAssetName());
		// Create an ISM component for this mesh and material
		FoundISMComp = NewObject<UInstancedStaticMeshComponent>(this);
		FoundISMComp->SetMobility(EComponentMobility::Movable);
		FoundISMComp->SetOnlyOwnerSee(false);
		check(GetRootComponent() != nullptr);
		FoundISMComp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		//FoundISMComp->bCastDynamicShadow = false;
		//FoundISMComp->CastShadow = false;		
		FoundISMComp->SetVisibility(true);
		FoundISMComp->SetStaticMesh(MeshRef);
		FoundISMComp->SetMaterial(0, Material);
		FoundISMComp->SetIsReplicated(true);
		FoundISMComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
		FoundISMComp->OnComponentCreated();
		FoundISMComp->RegisterComponent();
	}
	if (FoundISMComp)
	{
		UE_CLOG(bEnableClassDebugLog, LogTRGame, Log, TEXT("GridManager Adding ISM instance for %s at %s"), *Mesh.GetAssetName(), *SpawnTransform.GetLocation().ToString());
		return FoundISMComp->AddInstanceWorldSpace(SpawnTransform);
	}
	else {
		UE_LOG(LogTRGame, Error, TEXT("GridManager Could not find or create ISM component for %s"), *MeshRef->GetPathName());
	}
	return -1;
}


float APlatformGridMgr::GetRCAMinISMMoveDelay()
{
	return RoomComponentMinISMMoveDelay;
}


float APlatformGridMgr::GetRCAMaxISMMoveDelay()
{
	return RoomComponentMaxISMMoveDelay;
}