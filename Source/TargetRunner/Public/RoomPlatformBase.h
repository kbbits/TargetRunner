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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USceneComponent* RoomRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USceneComponent* RoomSceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* FloorMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
		TArray<ETRWallState> WallTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FTransform> WallSectionTransforms;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_RoomTemplate)
		FRoomTemplate RoomTemplate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bRoomTemplateSet;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void OnRep_RoomTemplate();
	void OnRep_RoomTemplate_Implementation();

	// Generates the entire room, including walls, etc.
	// Must set the WallTemplate member var before calling.
	UFUNCTION(Server, Reliable, BlueprintCallable, CallInEditor)
		void GenerateRoom();

	// Generates the outer walls for the room. Called by GenerateRoom.
	UFUNCTION(BlueprintCallable, CallInEditor)
		bool CalculateWalls();
	
	// Spawns the walls.
	// Call this on server!
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void SpawnWalls();

	// Spawns room contents. By default this just calls SpawnResources.
	// If you override this, remember to call Parent.
	// Call this on server!
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		bool SpawnContents();

	// Spawns the resources.
	// Call this on server!
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		bool SpawnResources();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};