// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceNodeBase.h"
#include "..\Public\ResourceNodeBase.h"
#include "ResourceFunctionLibrary.h"

// Sets default values
AResourceNodeBase::AResourceNodeBase()
{
	bReplicates = true;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene"));
	SetRootComponent(RootScene);
}

// Called when the game starts or when spawned
void AResourceNodeBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AResourceNodeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AResourceNodeBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AResourceNodeBase, CurrentHealth);
}

TArray<FResourceQuantity> AResourceNodeBase::GetResourceQuantities_Implementation()
{
	return UResourceFunctionLibrary::AddResourceQuantities(ResourcesByDamageCurrent, ResourcesOnDestroy);
}

float AResourceNodeBase::GetResourceQuantity_Implementation(const FResourceType ResourceType)
{
	float QuantityTotal = 0.0f;
	for (FResourceQuantity CurQuantity : ResourcesByDamage)
	{
		if (CurQuantity.ResourceType == ResourceType) { QuantityTotal += CurQuantity.Quantity; }
	}
	for (FResourceQuantity CurQuantity : ResourcesOnDestroy)
	{
		if (CurQuantity.ResourceType == ResourceType) { QuantityTotal += CurQuantity.Quantity; }
	}
	return QuantityTotal;	
}


bool AResourceNodeBase::ExtractedResourcesForDamage_Implementation(const float Damage, const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities, const bool bReduceCurrent)
{
	ExtractedQuantities.Empty();
	// If we have no resources by damage just return
	if (ResourcesByDamage.Num() == 0) { return false; }
	// If no damage or extraction rates is empty just return
	if (Damage <= 0 || ExtractionRates.Num() == 0) { return false; }

	float DamagePercent = BaseHealth > 0 ? FMath::Clamp<float>(Damage / BaseHealth, 0.0, 1.0) : 0.0f;
	float CurrentQuantity;
	float ExtractedQuantity;
	bool bFound;
	FResourceRateFilter FoundRate;
	ETRResourceMatch FoundSimilarity;

	ExtractedQuantities.Reserve(ResourcesByDamage.Num());
	for (FResourceQuantity CurResource : ResourcesByDamage)
	{
		if (UResourceFunctionLibrary::FindResourceRateFilter(ExtractionRates, CurResource.ResourceType, FoundRate, FoundSimilarity))
		{
			ExtractedQuantity = (CurResource.Quantity * DamagePercent) * FoundRate.Rate;
			if (ExtractedQuantity < 0.0f) { ExtractedQuantity = 0.0f; }

			bFound = false;
			for (int32 i = 0; !bFound && i < ResourcesByDamageCurrent.Num(); i++)
			{
				if (ResourcesByDamageCurrent[i].ResourceType == CurResource.ResourceType)
				{
					CurrentQuantity = ResourcesByDamageCurrent[i].Quantity;
					if (ExtractedQuantity > CurrentQuantity) { ExtractedQuantity = CurrentQuantity; }
					if (bReduceCurrent)
					{
						ResourcesByDamageCurrent[i].Quantity = ResourcesByDamageCurrent[i].Quantity - ExtractedQuantity;
					}
					bFound = true;
				}
			}
			ExtractedQuantities.Add(FResourceQuantity(CurResource.ResourceType, ExtractedQuantity));			
		}
	}
	return ExtractedQuantities.Num() > 0;
}


bool AResourceNodeBase::ExtractedResourcesOnDestroy_Implementation(const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities)
{
	ExtractedQuantities.Empty();
	// If we have no resources by damage just return
	if (ResourcesOnDestroy.Num() == 0) { return false; }
	// If no extraction rates just return
	if (ExtractionRates.Num() == 0) { return false; }

	float ExtractedQuantity;
	FResourceRateFilter FoundRate;
	ETRResourceMatch FoundSimilarity;

	ExtractedQuantities.Reserve(ResourcesByDamage.Num());
	for (FResourceQuantity CurResource : ResourcesOnDestroy)
	{
		if (UResourceFunctionLibrary::FindResourceRateFilter(ExtractionRates, CurResource.ResourceType, FoundRate, FoundSimilarity))
		{
			ExtractedQuantity = CurResource.Quantity * FoundRate.Rate;
			if (ExtractedQuantity < 0.0f) { ExtractedQuantity = 0.0f; }
			ExtractedQuantities.Add(FResourceQuantity(CurResource.ResourceType, ExtractedQuantity));
		}
	}
	return ExtractedQuantities.Num() > 0;
}