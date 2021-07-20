// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PlatformGridMgr.h"
#include "GridForgeBase.h"
#include "RoomPlatformBase.h"
#include "RoomGridTemplate.h"
#include "ResourceDropperBase.h"
#include "RoomComponentSpec.h"
#include "ProgressItem.h"
#include "RoomPlatformGridMgr.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGenerateProgress, const FProgressItem, Progress);

// Sub-grid of the platform is desribed by the GridManager's RoomCellSubdivision. The room is divided in a (RoomCellSubdivisions - 1) x (RoomCellSubdivisions - 1) sub-grid.
// Sub-grid coords are 0 based. i.e. (0, 0) is first cell in the South West corner of room. (1, 0) is one sub-grid cell north of (0, 0).
// and last cell ((RoomCellSubdivisions - 1), (RoomCellSubdivisions - 1)) is in the North East corner.
// As with other grids, North is + X direction of the related GridManager.
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

#if WITH_EDITORONLY_DATA
	// Only exists in editor builds. To auto-spawn the whole grid during GenerateGrid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
		bool bSpawnRoomsAfterGenerate;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Meta = (ExposeOnSpawn = "true"))
		bool bDoorsAtWallCenter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
		TArray<FResourceQuantity> ResourcesToDistribute;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
		TArray<TSubclassOf<AActor>> SpecialsToDistribute;
	
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

	// Room components data to use. Describes floors and ceiling room actors to use when spawning rooms.
	// Using different data tables allows, for example, different themes.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UDataTable* RoomComponentsTable;

protected:

	// Our cache of floor room components info loaded from data table and kept as a map for use.
	TMap<ETRRoomExitLayout, TArray<FRoomComponentSpec>> RoomFloorComponentMap;
	// Our cache of ceiling room components info loaded from data table and kept as a map for use.
	TMap<ETRRoomExitLayout, TArray<FRoomComponentSpec>> RoomCeilingComponentMap;

	// Our cache of solid wall room components info loaded from data table and kept as an array for use.
	TArray<FRoomComponentSpec> RoomWallComponentArray;
	// Our cache of door room components info loaded from data table and kept as an array for use.
	TArray<FRoomComponentSpec> RoomDoorComponentArray;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// GridForge will bind to this to update us on its progress.
	void OnGridForgeProgress(const FProgressItem ProgressItem);

	// Load room component info from data table into our cached maps.
	void InitRoomComponentMaps(const bool bForceReload = false);

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
	UFUNCTION(BlueprintNativeEvent, CallInEditor)
		void SpawnRooms();

	// Spanwn the room from the room template grid with the given coordinates.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SpawnRoom(FVector2D GridCoords);

	// Called when spawning of all rooms has completed.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SpawnsFinished();

	//UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	UFUNCTION(BlueprintNativeEvent)
		void SetRoomGridTemplateData(const FRoomGridTemplate& UpdatedTemplate, const TArray<FVector2D>& RoomCoords, const TArray<FRoomTemplate>& RoomTemplates);

	virtual void GenerateGridImpl() override;

	virtual void DestroyGridImpl() override;

	// Wake connected neighbor rooms within StasisWakeRange distance from given platform coords.
	// Does not wake diagonal neighbors.
	virtual void WakeNeighborsImpl(FVector2D AroundGridCoords) override;

	//UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	//	TSoftObjectPtr<UPrefabricatorAssetInterface> GetRoomComponentPrefab(const ETRRoomComponentType Type, const ETRRoomExitLayout ExitLayout, bool& bFound);

	// Pick a floor or ceiling component type for the given exit layout.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		TSubclassOf<ARoomComponentActor> GetRoomComponentActorInLayoutMap(const ETRRoomComponentType Type, const ETRRoomExitLayout ExitLayout, bool& bFound);
	
	// Pick a wall or door component type for the room at the given coords.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		TSubclassOf<ARoomComponentActor> GetRoomComponentActorInArray(const ETRRoomComponentType Type, const FIntPoint RoomCoords, bool& bFound);

	// Pick room component of the given type for the room at the given coords.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		TSubclassOf<ARoomComponentActor> GetRoomComponentActor(const ETRRoomComponentType Type, const FIntPoint RoomCoords, FRoomExitInfo& ExitInfo, bool& bFound);
	
	// Clears internal RoomComponentMaps built from the RoomComponentsTable.
	// Caches will be rebuilt when needed from underlying data table.
	UFUNCTION(BlueprintCallable)
		void ClearRoomComponentCaches();

};
