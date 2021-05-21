// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TargetRunner.h"
#include "ResourceQuantity.h"
#include "ResourceNodeBase.h"
#include "LevelExitBase.generated.h"

UCLASS()
class TARGETRUNNER_API ALevelExitBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALevelExitBase();

	// The class of Resource Node to spawn on any resource node sites.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<AResourceNodeBase>> NodeClasses;

	// The ResourceType and quantities for this exit. This affects, for example, the damage done by sources that implement IDoesDamageByType interface.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
		TArray<FResourceQuantity> ExitResourceTypes;

protected:
	// Root scene for this actor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USceneComponent* RootScene;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Set up any child resource nodes, etc. Usually called at BeginPlay
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SetupNodes();

};
