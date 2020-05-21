#pragma once

#include "TrEnums.h"
#include "Core.h"
#include "GameFramework/Actor.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<ETRWallState> WallTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FTransform> WallSectionTransforms;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Generates the entire room, including walls, etc.
	// Must set the WallTemplate member var before calling.
	UFUNCTION(Server, Reliable, BlueprintCallable, CallInEditor)
		void GenerateRoom();

	// Generates the outer walls for the room. Called by GenerateRoom.
	UFUNCTION(BlueprintCallable, CallInEditor)
		bool CalculateWalls();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		bool SpawnWalls();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};