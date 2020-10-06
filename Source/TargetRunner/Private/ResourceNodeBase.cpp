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
	DOREPLIFETIME(AResourceNodeBase, ResourcesByDamageCurrent);
}

void AResourceNodeBase::OnRep_CurrentHealth_Implementation()
{
}

void AResourceNodeBase::OnRep_ResourcesByDamageCurrent()
{
}

void AResourceNodeBase::ServerSetCurrentHealth_Implementation(const float NewCurrentHealth)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = NewCurrentHealth;
		// Replication will call this on clients. This is a direct call for server.
		OnRep_CurrentHealth();
		if (CurrentHealth <= 0.0f)
		{
			OnNodeDestroyed.Broadcast();
		}
	}	
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

bool AResourceNodeBase::ExtractedResourcesForDamage_Implementation(const float Damage, const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities)
{
	ExtractedQuantities.Empty();
	// If we have no resources by damage just return
	if (ResourcesByDamage.Num() == 0) { return false; }
	// If no damage or extraction rates is empty just return
	if (Damage <= 0 || ExtractionRates.Num() == 0) { return false; }

	float DamagePercent = BaseHealth > 0.0f ? FMath::Clamp<float>(Damage / BaseHealth, 0.0f, 1.0f) : 0.0f;
	float ExtractedQuantity;
	FResourceRateFilter FoundRate;
	ETRResourceMatch FoundSimilarity;

	ExtractedQuantities.Reserve(ResourcesByDamage.Num());
	for (FResourceQuantity CurResource : ResourcesByDamage)
	{
		if (UResourceFunctionLibrary::FindBestResourceRateFilter(ExtractionRates, CurResource.ResourceType, FoundRate, FoundSimilarity))
		{
			UE_LOG(LogTRGame, Log, TEXT("ExtractedResourcesForDamage - target resource type: %s Found resource rate: %s  %.2f"), *CurResource.ResourceType.Code.ToString(), *FoundRate.ResourceTypeFilter.Code.ToString(), FoundRate.Rate)
			ExtractedQuantity = FMath::CeilToFloat((CurResource.Quantity * DamagePercent) * FoundRate.Rate);
			if (ExtractedQuantity <= 0.0f) { continue; }
			if (ExtractedQuantity > CurResource.Quantity) { ExtractedQuantity = CurResource.Quantity; }
			ExtractedQuantities.Add(FResourceQuantity(CurResource.ResourceType, ExtractedQuantity));			
		}
	}
	return ExtractedQuantities.Num() > 0;
}

void AResourceNodeBase::ServerExtractResources_Implementation(const TArray<FResourceQuantity>& ExtractQuantities)
{
	if (ExtractQuantities.Num() > 0 && GetLocalRole() == ROLE_Authority) // Don't actually need auth check - function runs on server.
	{
		bool bFound = false;
		bool bFoundAny = false;
		for (FResourceQuantity Extracted : ExtractQuantities)
		{
			bFound = false;
			for (int32 i = 0; !bFound && i < ResourcesByDamageCurrent.Num(); i++)
			{
				if (ResourcesByDamageCurrent[i].ResourceType == Extracted.ResourceType)
				{
					ResourcesByDamageCurrent[i].Quantity = FMath::Max(ResourcesByDamageCurrent[i].Quantity - Extracted.Quantity, 0.0f);
					bFound = true;
					bFoundAny = true;
				}
			}
		}
		// Manually call OnRep so server gets it too.
		if (bFoundAny) { OnRep_ResourcesByDamageCurrent(); }
	}
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
		if (UResourceFunctionLibrary::FindBestResourceRateFilter(ExtractionRates, CurResource.ResourceType, FoundRate, FoundSimilarity))
		{
			ExtractedQuantity = FMath::RoundHalfToZero(CurResource.Quantity * FoundRate.Rate);
			if (ExtractedQuantity < 0.0f) { ExtractedQuantity = 0.0f; }
			ExtractedQuantities.Add(FResourceQuantity(CurResource.ResourceType, ExtractedQuantity));
		}
	}
	return ExtractedQuantities.Num() > 0;
}