// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/Vector.h"
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

UCLASS()
class TARGETRUNNER_API APlatformGridMgr : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlatformGridMgr();
	
	// Rows are along grid x axis, columns (elements in each row) are grid Y axis
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<int32, FPlatformGridRow> PlatformGridMap;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	//	FRoomGridTemplate GridTemplate;

	// The size, in world units, of each grid cell.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float GridCellWorldSize;

	// Grid extents indicate the overall size of the grid. That is, the minimum and maxium valid grid coordinates.
	// Min extents are negative, max extents are positive, origin is at 0,0.
	// GridExtentMinX must be <= 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 GridExtentMinX;
	// Must be >= 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 GridExtentMaxX;
	// Must be <= 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 GridExtentMinY;
	// Must be >= 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 GridExtentMaxY;

	// The number of subdivisions along each X & Y axis that each cell is divided.
	// Creating RoomCellSubdivision x RoomCellSubdivision total subcells in each cell.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 RoomCellSubdivision;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UGridForgeBase> GridForgeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<ARoomPlatformBase> RoomClass;

	// A map of actor references initialized and used at runtime for efficiency.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TMap<FName, AActor*> GridActorCache;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
		void Setup();

	UFUNCTION(BlueprintPure)
		FTransform GetGridCellWorldTransform(const FVector2D& GridCoords);

	UFUNCTION(Server, Reliable, BlueprintCallable, CallInEditor)
		void GenerateGrid();

	UFUNCTION(Server, Reliable, BlueprintCallable)
		void DestroyGrid();

	UFUNCTION(BlueprintCallable)
		void AddPlatformToGridMap(APlatformBase* platform);

	UFUNCTION(BlueprintCallable)
		APlatformBase* GetPlatformInGridMap(const int32 X, const int32 Y, bool& Found);
	UFUNCTION(BlueprintCallable)
		APlatformBase* GetPlatformInGrid(const FVector2D Coords, bool& Found);

	UFUNCTION(BlueprintCallable)
		APlatformBase* RemovePlatformFromGridMap(const int32 X, const int32 Y, bool& Success);
	UFUNCTION(BlueprintCallable)
		APlatformBase* RemovePlatformFromGrid(const FVector2D Coords, bool& Success);

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
		
};
