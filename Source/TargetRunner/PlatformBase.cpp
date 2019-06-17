// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformBase.h"
#include "UnrealNetwork.h"

// Sets default values
APlatformBase::APlatformBase()
{
	bReplicates = true;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APlatformBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APlatformBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlatformBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlatformBase, GridX);
	DOREPLIFETIME(APlatformBase, GridY);
}

FVector2D APlatformBase::GetGridCoordinates()
{
	FVector2D GridVector = FVector2D(float(GridX), float(GridY));
	return GridVector;
}

