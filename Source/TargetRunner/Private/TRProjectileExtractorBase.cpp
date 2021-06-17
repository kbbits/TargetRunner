// Fill out your copyright notice in the Description page of Project Settings.


#include "TRProjectileExtractorBase.h"

// Sets default values
ATRProjectileExtractorBase::ATRProjectileExtractorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
}


// Called when the game starts or when spawned
void ATRProjectileExtractorBase::BeginPlay()
{
	Super::BeginPlay();	
}


// Called every frame
void ATRProjectileExtractorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


TArray<FResourceRateFilter> ATRProjectileExtractorBase::GetExtractionRates_Implementation()
{
	return ResourceExtractionRates;
}

