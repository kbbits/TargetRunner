// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformGridMgr.h"

// Sets default values
APlatformGridMgr::APlatformGridMgr()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APlatformGridMgr::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlatformGridMgr::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlatformGridMgr::Setup()
{
	TArray<AActor*> FoundPlatforms;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlatformBase::StaticClass(), FoundPlatforms);

	for (AActor* TActor : FoundPlatforms)
	{
		APlatformBase* Platform = Cast<APlatformBase>(TActor);

		if (Platform != nullptr) {
			MyPlatforms.Add(Platform);
		}
	}
}