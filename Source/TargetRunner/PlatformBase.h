// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "PlatformBase.generated.h"

class APlatformGridMgr;

UCLASS()
class TARGETRUNNER_API APlatformBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlatformBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USceneComponent* RootScene;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USceneComponent* PlatformScene;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UBoxComponent* PlatformControlZone;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, SaveGame, Meta = (ExposeOnSpawn = "true"))
		int32 GridX;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, SaveGame, Meta = (ExposeOnSpawn = "true"))
		int32 GridY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bCanRemoveDynamic;

	// Call GetGridManager to initialize this.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		APlatformGridMgr* MyGridManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<FName, AActor*> PlatformActorCache;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRandomStream PlatformRandStream;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bStasisWokeNeighbors;

protected:

	UPROPERTY(EditAnywhere)
		bool bStasisAwake;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void InitPlatformScale();

	// Gets the current grid manager in the level.
	// Returns nullptr if no grid manager could be found.
	UFUNCTION(BlueprintCallable)
		APlatformGridMgr* GetGridManager();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure)
		FVector2D GetGridCoordinates();

	UFUNCTION(BlueprintPure)
		FORCEINLINE FIntPoint GetGridCoordinatesInt() {	return FIntPoint(GridX, GridY);	};

	// [Server]
	// Note this destroys self.
	// Destroys all actors in the PlatformActorCache, then destroys self.
	// If overridden in blueprints, you should call the parent function at end because this destroys self.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, CallInEditor)
		void DestroyPlatform();
	virtual void DestroyPlatformImpl();

	// [Call on Server]
	// Puts all IStasisObject actors in this platform's control zone into stasis
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void StasisSleepActors();

	// [Call on Server]
	// Awakens all IStasisObject actors in this platform's control zone out of stasis
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void StasisWakeActors();

};
