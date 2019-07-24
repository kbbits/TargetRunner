// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlatformBase.h"
#include "DynSceneryBase.generated.h"

UCLASS()
class TARGETRUNNER_API ADynSceneryBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADynSceneryBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="DynScenery", Meta = (ExposeOnSpawn = "true"))
		APlatformBase* Platform;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
