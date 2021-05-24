#pragma once

#include "TrEnums.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomTemplate.h"
#include "PlatformBase.h"
#include "Math/TransformNonVectorized.h"
#include "RoomPlatformBase.generated.h"

UCLASS()
class TARGETRUNNER_API ARoomPlatformBase : public APlatformBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARoomPlatformBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
		TArray<ETRWallState> WallTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FTransform> WallSectionTransforms;

	// Use the GetSubGridFloorState and SetSubGridFloorState functions.
	// A flat array representing the (RoomCellSubdivisions - 1) x (RoomCellSubdivisions - 1) sub-grid within this room.
	// Sub-grid coords are 0 based. i.e. (0, 0) is first cell in the South West corner of room
	// and last cell in the North East corner.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
		TArray<ETRFloorState> FloorTemplate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_RoomTemplate)
		FRoomTemplate RoomTemplate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bRoomTemplateSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<AActor*> SpawnedSpecialActors;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void OnRep_RoomTemplate();
	void OnRep_RoomTemplate_Implementation();

	// Override of base class to handle cleanup of Room stuff.
	virtual void DestroyPlatformImpl() override;

	// [Server]
	// Generates the entire room, including walls, etc.
	// Must set the WallTemplate member var before calling.
	UFUNCTION(Server, Reliable, BlueprintCallable, CallInEditor)
		void GenerateRoom();

	// [Server]
	// Generates the outer walls for the room. Called by GenerateRoom.
	UFUNCTION(BlueprintCallable, CallInEditor)
		bool CalculateWalls();
	
	// [Server]
	// Spawns the walls.
	// Call this on server!
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void SpawnWalls();

	// [Server]
	// Spawns the floor.
	// Call this on server!
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		bool SpawnFloor();

	// [Server]
	// Spawns room contents. By default this calls SpawnResources and SpawnSpecials.
	// If you override this, remember to call Parent.
	// Call this on server!
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		bool SpawnContents();

	// [Server]
	// Spawns the resources.
	// Call this on server!
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		bool SpawnResources();

	// [Server]
	// Spawns special actor instances in this room. (if any)
	// Call this on server!
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		bool SpawnSpecials();

	// If this room is connected to another room in the given direction, this returns the connected room.
	// If not connected in that direction, this returns nullptr.
	UFUNCTION(BlueprintPure)
		ARoomPlatformBase* GetConnectedNeighbor(const ETRDirection Direction);

	// Get the state of the sub-grid floor at the sub-grid coords.
	UFUNCTION(BlueprintPure)
		ETRFloorState GetSubGridFloorState(FIntPoint SubGridCoords);

	// Sets the floor state at the given sub-grid coords.
	// Returns the previous floor state at that sub-grid coord.
	UFUNCTION(BlueprintPure)
		ETRFloorState SetSubGridFloorState(FIntPoint SubGridCoords, ETRFloorState NewState);

	// Must include this in header since we are subclass of custom class?
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};