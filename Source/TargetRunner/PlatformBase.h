// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Core.h"
#include "GameFramework/Actor.h"
#include "PlatformBase.generated.h"

class APlatformGridMgr;

UCLASS()
class TARGETRUNNER_API APlatformBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlatformBase();

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, SaveGame, Meta = (ExposeOnSpawn = "true"))
		int32 GridX;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, SaveGame, Meta = (ExposeOnSpawn = "true"))
		int32 GridY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bCanRemoveDynamic;

	// Only valid on server!
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		APlatformGridMgr* MyGridManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<FName, AActor*> PlatformActorCache;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void InitPlatformScale();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure)
		FVector2D GetGridCoordinates();

};
