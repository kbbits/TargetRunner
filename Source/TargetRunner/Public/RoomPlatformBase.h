#pragma once

#include "TrEnums.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomTemplate.h"
#include "PlatformBase.h"
#include "Math/TransformNonVectorized.h"
#include "RoomPlatformBase.generated.h"

class ARoomComponentActor;

/*
* Subclass of platform as a Room.
*/
UCLASS()
class TARGETRUNNER_API ARoomPlatformBase : public APlatformBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARoomPlatformBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FTransform> WallSectionTransforms;
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bRoomTemplateSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bGenerateOnClient;

	//UPROPERTY(EditInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_WallTemplate)
	/**  */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
		TArray<ETRWallState> WallTemplate;

	// Our cache of refs to the RoomComponentActors this platform spawned.
	UPROPERTY()
		TArray<TWeakObjectPtr<ARoomComponentActor>> SpawnedRoomComponents;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_RoomTemplate)
		FRoomTemplate RoomTemplate;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** 
	*[Server]
	* Set this room's room template. Must do this before GenerateRoom can be called. 
	*/
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void ServerSetRoomTemplate(const FRoomTemplate& NewRoomTemplate);

	//UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	//	void OnRep_WallTemplate();
	//void OnRep_WallTemplate_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void OnRep_RoomTemplate();
	void OnRep_RoomTemplate_Implementation();

	// Override of base class to handle cleanup of Room stuff.
	virtual void DestroyPlatformImpl() override;

	// [Server]
	// Generates the entire room, including walls, etc.
	// Must call SetRoomTemplate first.
	UFUNCTION(Server, Reliable, BlueprintCallable, CallInEditor)
		void ServerGenerateRoom();

	// [Any]
	// Native version for implemention.
	// Generates the entire room, including walls, etc.
	void GenerateRoomImpl();

	// [Server]
	// Generates the outer walls for the room. Called by GenerateRoom.
	UFUNCTION(BlueprintCallable, CallInEditor)
		bool CalculateWalls();
	
	// Spawns the walls.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void SpawnWalls();

	// Spawns the floor.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		bool SpawnFloor();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		bool SpawnCeiling();

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

	// [Server]
	// Spawns clutter actor instances in this room. (if any)
	// Call this on server!
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		bool SpawnClutter();

	// Checks that all spawned RoomComponentActors have finished their spawn and setup.
	// This includes checking if the RoomComponents have copied out their ISM instances.
	UFUNCTION(BlueprintCallable)
		bool AllTrackedRoomComponentsSetup();

	// Add this RoomComponentActor to our collection of spawned RCAs so it will be tracked for updates and destruction, etc.
	UFUNCTION(BlueprintCallable)
		int32 TrackRoomComponentActor(ARoomComponentActor* RoomComponentActor);

	// Get all the tracked RoomComponentActors (that still have have valid refs).
	// Returns the number of valid RCAs found.
	UFUNCTION(BlueprintCallable)
		int32 GetTrackedRoomComponentActors(TArray<ARoomComponentActor*>& RoomComponentActors);

	// Destroys all the RoomComponentActors that this plaform spawned.
	// Returns the number of RCAs destroyed.
	UFUNCTION(BlueprintCallable)
		int32 DestroyTrackedRoomComponents();

	// If this room is connected to another room in the given direction, this returns the connected room.
	// If not connected in that direction, this returns nullptr.
	UFUNCTION(BlueprintPure)
		ARoomPlatformBase* GetConnectedNeighbor(const ETRDirection Direction);

	// Must include this in header since we are subclass of concrete class?
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	// Helper to cleanup special actor ObjectPlacers.
	void DestroySpecialPlacers();
};