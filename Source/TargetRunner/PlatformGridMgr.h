// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlatformBase.h"
#include "PlatformGridRow.h"
#include "Kismet/GameplayStatics.h"
#include "PlatformGridMgr.generated.h"

UCLASS()
class TARGETRUNNER_API APlatformGridMgr : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlatformGridMgr();

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//	TArray<APlatformBase*> MyPlatforms;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//	TArray<FPlatformGridRow> PlatformGrid;
	
	// Rows are along grid x axis, columns (elements in each row) are grid Y axis
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<int, FPlatformGridRow> PlatformGridMap;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
		void Setup();

	UFUNCTION(BlueprintCallable)
		void AddPlatformToGridMap(APlatformBase* platform);

	UFUNCTION(BlueprintCallable)
		APlatformBase* GetPlatformInGridMap(int X, int Y, bool& Found);

	UFUNCTION(BlueprintCallable)
		APlatformBase* RemovePlatformFromGridMap(int X, int Y, bool& Success);

};
