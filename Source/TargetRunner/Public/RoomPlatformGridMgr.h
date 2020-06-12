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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
		bool bGenerateStartEnd;

	// For dev-time use. To auto-spawn the whole grid during GenerateGrid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
		bool bSpawnGridAfterGenerate;
	
	// This is set by the GameMode during InitGridManager
	// Can be setup manually for use in-editor.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn="true"))
		TSubclassOf<UGridForgeBase> GridForgeClass;

	// This is set by the GameMode during InitGridManager
	// Can be set manually for use in-editor.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
		TSubclassOf<ARoomPlatformBase> RoomClass;
	
	// A grid containing the temlates for the rooms. This is populated during grid generation.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
		FRoomGridTemplate RoomGridTemplate;

	// These cells are not available to be part of the maze grid.
	// These are determined during grid generation.
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
		TArray<FVector2D> BlackoutCells;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Spanwn the room from the room template grid with the given coordinates.
	UFUNCTION(Server, Reliable, BlueprintCallable)
		void SpawnRoom(FVector2D GridCoords);

	// Spawns all rooms in the room templat grid
	UFUNCTION(Server, Reliable, BlueprintCallable, CallInEditor)
		void SpawnRooms();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
		void ClientUpdateRoomGridTemplate(const FRoomGridTemplate& UpdatedTemplate);

	virtual void GenerateGridImpl() override;

	virtual void DestroyGridImpl() override;
	
	TArray<FRoomTemplate*> GetAllRoomTemplates();
};
