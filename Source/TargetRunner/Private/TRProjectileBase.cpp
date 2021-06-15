// Fill out your copyright notice in the Description page of Project Settings.


#include "TRProjectileBase.h"
#include "..\Public\TRProjectileBase.h"
#include "ResourceFunctionLibrary.h"

// Sets default values
ATRProjectileBase::ATRProjectileBase()
{
	bReplicates = true;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATRProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATRProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

TArray<FResourceRateFilter> ATRProjectileBase::GetDamageRates_Implementation()
{
	return BaseDamageRates;
}

bool ATRProjectileBase::CalculateDamageByType_Implementation(const float Damage, const FResourceType& ResourceType, float& CalculatedDamage)
{
	FResourceRateFilter FoundRate;
	ETRResourceMatch FoundSimilarity;
	if (UResourceFunctionLibrary::FindBestResourceRateFilter(GetDamageRates(), ResourceType, FoundRate, FoundSimilarity))
	{
		CalculatedDamage = Damage * FoundRate.Rate;
	}
	else
	{
		CalculatedDamage = 0.0f;
	}
	return CalculatedDamage > 0.0f;
}

