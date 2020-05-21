// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/Vector.h"
#include "PlatformBase.h"
#include "PlatformGridRow.h"
#include "PlatformGridMgr.generated.h"

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

	// The size, in world units, of each grid cell.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float GridCellWorldSize;

	// The offset of the grid's origin. Default is origin at 0,0,0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FVector GridWorldOffset;

	// Grid extents indicate the overall size of the grid. That is, the minimum and maxium valid grid coordinates.
	// Min extents are negative, max extents are positive, origin is at 0,0.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 GridExtentMinX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 GridExtentMaxX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 GridExtentMinY;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 GridExtentMaxY;

	// The number of subdivisions along each X & Y axis that each cell is divided.
	// Creating RoomCellSubdivision x RoomCellSubdivision total subcells in each cell.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 RoomCellSubdivision;

	// A map of actor references initialized and used at runtime for efficiency.
	TMap<FName, AActor*> GridActorCache;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
		void Setup();

	UFUNCTION(BlueprintCallable)
		void AddPlatformToGridMap(APlatformBase* platform);

	UFUNCTION(BlueprintCallable)
		APlatformBase* GetPlatformInGridMap(int32 X, int32 Y, bool& Found);

	UFUNCTION(BlueprintCallable)
		APlatformBase* RemovePlatformFromGridMap(int32 X, int32 Y, bool& Success);

};
