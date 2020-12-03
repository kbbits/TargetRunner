// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PlatformGridMgr.h"
#include "GridForgeBase.h"
#include "RoomPlatformBase.h"
#include "RoomGridTemplate.h"
#include "ResourceDropperBase.h"
#include "ProgressItem.h"
#include "RoomPlatformGridMgr.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGenerateProgress, const FProgressItem, Progress);

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

#if WITH_EDITOR
	// Only exists in editor builds. To auto-spawn the whole grid during GenerateGrid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
		bool bSpawnRoomsAfterGenerate;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
		TArray<FResourceQuantity> ResourcesToDistribute;
	
	// This is set by the GameMode during InitGridManager
	// Can be setup manually for use in-editor.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn="true"))
		TSubclassOf<UGridForgeBase> GridForgeClass;

	// This is set by the GameMode during InitGridManager
	// Can be set manually for use in-editor.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
		TSubclassOf<ARoomPlatformBase> RoomClass;

	// This is set by the GameMode during InitGridManager
	// Can be set manually for use in-editor.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
		TSubclassOf<UResourceDropperBase> ResourceDropperClass;
	
	// A grid containing the temlates for the rooms. This is populated during grid generation.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
		FRoomGridTemplate RoomGridTemplate;

	// These cells are not available to be part of the maze grid.
	// These are usually determined during grid generation if this array is empty, but can be manually specified here.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FVector2D> OverrideBlackoutCells;

	// Use this stream if no GameMode (ex: in Editor)
	// Should only be used on server.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRandomStream DefaultResourceDropperStream;

	// Delegate event notification for grid/room generation progress.
	// Listeners bind to this to recieve room generation progress.
	FOnGenerateProgress OnGenerateProgress;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// GridForge will bind to this to update us on its progress.
	void OnGridForgeProgress(const FProgressItem ProgressItem);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Gets the room at the given coordinates.
	// Will return nullptr if no room exists at those coords.
	UFUNCTION(BlueprintPure)
		ARoomPlatformBase* GetRoomInGrid(const FVector2D& RoomCoords);

	UFUNCTION(BlueprintPure)
		ARoomPlatformBase* GetRoomNeighbor(const FVector2D& RoomCoords, const ETRDirection DirectionToNeighbor);

	// Spawns all rooms in the room template grid
	//UFUNCTION(Server, Reliable, BlueprintCallable, CallInEditor)
	UFUNCTION(BlueprintNativeEvent, CallInEditor)
		void SpawnRooms();

	// Spanwn the room from the room template grid with the given coordinates.
	//UFUNCTION(Server, Reliable, BlueprintCallable)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SpawnRoom(FVector2D GridCoords);	

	//UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	UFUNCTION(BlueprintNativeEvent)
		void SetRoomGridTemplateData(const FRoomGridTemplate& UpdatedTemplate, const TArray<FVector2D>& RoomCoords, const TArray<FRoomTemplate>& RoomTemplates);

	virtual void GenerateGridImpl() override;

	virtual void DestroyGridImpl() override;

	// Wake connected neighbor rooms within StasisWakeRange distance from given platform coords.
	// Does not wake diagonal neighbors.
	virtual void WakeNeighborsImpl(FVector2D AroundGridCoords) override;

protected:

	
};
