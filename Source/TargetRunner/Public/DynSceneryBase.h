// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlatformBase.h"
#include "DynSceneryBase.generated.h"

/*
* Base actor class for the DynScenery system. The DynScenery system is no longer in use in the current incarnation of TR.
* In brief, the DynScenery system provides a data-driven implementation of dynamic scene generation. 
* DynScene actor types are selected and placed into layers (DynSceneLayer). These layers are then spawned into the world (bottom up).
* The result is a collection of scene actors that stack on top of each other providing randomized scene generation.
*/
UCLASS()
class TARGETRUNNER_API ADynSceneryBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADynSceneryBase();

	// The platform this DynScenery actor is on.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="DynScenery", Meta = (ExposeOnSpawn = "true"))
		APlatformBase* Platform;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
