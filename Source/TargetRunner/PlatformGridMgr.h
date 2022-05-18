// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/Vector.h"
#include "GameFramework/PlayerStart.h"
#include "PlatformBase.h"
#include "RoomPlatformBase.h"
#include "PlatformGridRow.h"
#include "GridForgeBase.h"
#include "RoomGridTemplate.h"
#include "PlatformGridMgr.generated.h"

UENUM(BlueprintType)
enum class EInGrid : uint8
{
	NotInGrid 	UMETA(DisplayName = "Not In Grid"),
	InGrid  	UMETA(DisplayName = "In Grid")
};


USTRUCT(BlueprintType)
struct FISMContext 
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
		TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(BlueprintReadWrite)
		FSoftObjectPath MeshPath;

	UPROPERTY(BlueprintReadWrite)
		TArray<FTransform> SpawnTransforms;

};


/*
* The base class responsible for generating the map grid and spawning platforms (i.e. "rooms") into the world.
* Most of the functionality and data is server-side only.
*/
UCLASS()
class TARGETRUNNER_API APlatformGridMgr : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlatformGridMgr();
	
	// Rows are along grid x axis, columns (elements in each row) are grid Y axis.
	// Also note, that the grid itself is oriented to the grid manager actor. i.e. if the grid manager is rotated 90 degrees around Z axis
	// the grid will also be rotated 90 degrees in the world.
	// NOTE: On clients, only PlatformBase actors that exist when FillGridFromExistingPlatforms() is called will be in the grid.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<int32, FPlatformGridRow> PlatformGridMap;

	// The size, in world units, of each grid cell.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Meta = (ExposeOnSpawn = "true"))
		float GridCellWorldSize;

	// Grid extents indicate the overall size of the grid. That is, the minimum and maxium valid grid coordinates.
	// Min extents can be negative (and typically are). Max extents are usually positive, origin is at 0,0 of grid.
	// GridExtentMinX must be < GridExtentMaxX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Meta = (ExposeOnSpawn = "true"))
		int32 GridExtentMinX;
	// Must be > GridExtentMinX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Meta = (ExposeOnSpawn = "true"))
		int32 GridExtentMaxX;
	// Must be < GridExtentMaxY
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Meta = (ExposeOnSpawn = "true"))
		int32 GridExtentMinY;
	// Must be > GridExtentMinY
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Meta = (ExposeOnSpawn = "true"))
		int32 GridExtentMaxY;

	// Location for player starts. Usually determined during grid generation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Meta = (ExposeOnSpawn = "true"))
		FVector2D StartGridCoords;

	// Location of level exit. Usually determined during grid generation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Meta = (ExposeOnSpawn = "true"))
		FVector2D ExitGridCoords;

	// The number of subdivisions along each X & Y axis that each cell is divided.
	// Creating RoomCellSubdivision x RoomCellSubdivision total subcells in each room.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Meta = (ExposeOnSpawn = "true"))
		int32 RoomCellSubdivision;
	
	// Use this stream if no GameMode (ex: in Editor)
	// Should only be used on server.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRandomStream DefaultGridRandStream;

	// A map of actor references initialized and used at runtime for efficiency.
	// Only valid on server.
	// Currently unused?
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TMap<FName, AActor*> GridActorCache;

	// Distance from a given platform that we will wake actors.
	// Default = 1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 StasisWakeRange;

	// This object, if valid, will be used as the owner and world referece when spawning map actors.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		UObject* SpawnOwner;

protected:

	UPROPERTY()
	TArray<APlayerStart*> PlayerStarts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bEnableClientISMs = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<FISMContext> ServerISMQueue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 ISMQueueMaxBatchSize = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ServerISMQueueTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ServerISMQueueMaxTime = 0.25f;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bEnableClassDebugLog = false;
#endif

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MovePlayerStarts();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Fills (or re-fills) the platform grid from existing platform actors.
	UFUNCTION(BlueprintCallable)
		void FillGridFromExistingPlatforms();

	// To have the clients re-fill their platform grids.
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
		void ClientFillGridFromExistingPlatforms();

	UFUNCTION(BlueprintPure)
		FTransform GetGridCellWorldTransform(const FVector2D& GridCoords);

	UFUNCTION(BlueprintPure)
		FTransform GetGridCellSubGridWorldTransform(const FVector2D& GridCoords, const FVector2D& SubGridCoords);

	// [Server]
	UFUNCTION(Server, Reliable, BlueprintCallable, CallInEditor)
		void GenerateGrid();

	// Called by GenerateGrid. Native subclasses should override this one.
	virtual void GenerateGridImpl();

	// Calls DestroyGriImpl and on server destroys the player starts, all rooms, etc.
	//UFUNCTION(Server, Reliable, BlueprintCallable, CallInEditor)
	UFUNCTION(BlueprintNativeEvent, CallInEditor)
		void DestroyGrid();

	// Called by DestroyGrid. Subclasses should override this one.
	virtual void DestroyGridImpl();

	UFUNCTION(BlueprintCallable)
		void AddPlatformToGridMap(APlatformBase* platform);

	UFUNCTION(BlueprintCallable)
		APlatformBase* GetPlatformInGridMap(const int32 X, const int32 Y, bool& Found);
	UFUNCTION(BlueprintCallable)
		APlatformBase* GetPlatformInGrid(const FVector2D Coords, bool& Found);
	UFUNCTION(BlueprintCallable)
		APlatformBase* GetPlatformNeighbor(const FVector2D& MyCoords, const ETRDirection DirectionToNeighbor);

	UFUNCTION(BlueprintCallable)
		APlatformBase* RemovePlatformFromGridMap(const int32 X, const int32 Y, bool& Success);
	UFUNCTION(BlueprintCallable)
		APlatformBase* RemovePlatformFromGrid(const FVector2D Coords, bool& Success);

	// Returns the number of spawned platforms (rooms) in the current grid.
	// Note: This iterates the grid to do the count. Cache this value when appropriate.
	UFUNCTION(BlueprintCallable)
		int32 GetPlatformCount();

	// The total number of cells across the X axis of the grid.
	UFUNCTION(BlueprintPure)
		int32 GetGridWidthX();

	// The total number of cells across the Y axis of the grid.
	UFUNCTION(BlueprintPure)
		int32 GetGridWidthY();

	// Are the grid coordinates within the grid extents?
	UFUNCTION(BlueprintPure)
		bool IsInGrid(const FVector2D Coords);

	// Switch on whether the grid coordinates are within the grid extents.
	UFUNCTION(BlueprintCallable, meta = (ExpandEnumAsExecs = "InGrid"))
		bool IsInGridSwitch(FVector2D Coords, EInGrid& InGrid);

	// Each grid cell can also be identified by a number. The number of a given cell depends on the extents of the grid. 
	// Cells are numbered starting at GridExtentMinX, GridExtentMinY, proceeding along the +Y axis, then up the +X axis.
	// Resulting in cell 0 being at [GridExtentMinX, GridExtentMinY] and the highest cell number at [GridExtentMaxX, GridExtentMaxY].
	UFUNCTION(BlueprintPure)
		int32 GridCoordsToCellNumber(const FVector2D Coords);

	// Wake neighbor platforms within StasisWakeRange distance from given platform coords.
	// Does not wake diagonal neighbors.
	UFUNCTION(BlueprintCallable)
		void WakeNeighbors(const FVector2D AroundGridCoords);

	// Native function to do the work, so native subclasses can override.
	virtual void WakeNeighborsImpl(const FVector2D AroundGridCoords);

	// [Server]
	// Adds instances for the given meshes as ISMs
	// This will queue the ISMs for replication to client(s).
	UFUNCTION(BlueprintCallable)
		void SpawnISMs(const TArray<FISMContext>& ISMContexts);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void MC_SpawnISMs(const TArray<FISMContext>& ISMContexts);
		
	// Add an instance of a static mesh using the given material. SpawnTransform is in world space.
	// This will queue the replication of ISM instance to client.
	// Returns the index of the instance spawned.
	UFUNCTION()
		int32 SpawnISM(UPARAM(ref) TSoftObjectPtr<UStaticMesh> Mesh, UPARAM(ref) UMaterialInterface* Material, const FTransform& SpawnTransform);

	// Debug
#if WITH_EDITOR
	FORCEINLINE void DebugLog(const FString& LogString) { if (bEnableClassDebugLog) { UE_LOG(LogTRGame, Log, TEXT("%s"), *LogString); } };
#else
	FORCEINLINE void DebugLog(const FString& LogString) { };
#endif
};
