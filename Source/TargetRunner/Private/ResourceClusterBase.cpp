// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceClusterBase.h"

// Sets default values
AResourceClusterBase::AResourceClusterBase()
{
	bReplicates = true;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene"));
	SetRootComponent(RootScene);
}

// Called when the game starts or when spawned
void AResourceClusterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AResourceClusterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


//void AResourceClusterBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//}

