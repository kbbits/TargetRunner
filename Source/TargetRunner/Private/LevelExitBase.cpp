// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelExitBase.h"
#include "..\Public\LevelExitBase.h"

// Sets default values
ALevelExitBase::ALevelExitBase()
{
	bReplicates = true;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene"));
	SetRootComponent(RootScene);
}

// Called when the game starts or when spawned
void ALevelExitBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALevelExitBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALevelExitBase::SetupNodes_Implementation()
{
	
}

