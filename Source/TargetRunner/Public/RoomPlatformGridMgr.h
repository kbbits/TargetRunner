// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PlatformGridMgr.h"
#include "GridForgeBase.h"
#include "RoomPlatformBase.h"
#include "RoomGridTemplate.h"
#include "RoomPlatformGridMgr.generated.h"

UCLASS()
class TARGETRUNNER_API ARoomPlatformGridMgr : public APlatformGridMgr
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoomPlatformGridMgr();
	
	// If true, new start and end coords will be generated when grid is generated.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		bool bGenerateStartEnd;

	// For dev-time use. To auto-spawn the whole grid during GenerateGrid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		bool bSpawnGridAfterGenerate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UGridForgeBase> GridForgeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<ARoomPlatformBase> RoomClass;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
		FRoomGridTemplate RoomGridTemplate;

	// These cells are not available to be part of the maze grid
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FVector2D> BlackoutCells;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Server, Reliable, BlueprintCallable, CallInEditor)
		void SpawnRoom(FVector2D GridCoords);

	virtual void GenerateGridImpl() override;

	virtual void DestroyGridImpl() override;
	
	TArray<FRoomTemplate*> GetAllRoomTemplates();
};
