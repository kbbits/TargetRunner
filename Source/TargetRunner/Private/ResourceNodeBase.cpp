// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceNodeBase.h"
#include "ResourceFunctionLibrary.h"
// for intellisense
#include "..\Public\ResourceNodeBase.h"

// Sets default values
AResourceNodeBase::AResourceNodeBase()
{
	bReplicates = true;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene"));
	SetRootComponent(RootScene);
	CurrentHealth = BaseHealth;
}


void AResourceNodeBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
}


// Called when the game starts or when spawned
void AResourceNodeBase::BeginPlay()
{
	Super::BeginPlay();
	if (CurrentHealth != BaseHealth) { CurrentHealth = BaseHealth; }
}


// Called every frame
void AResourceNodeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AResourceNodeBase::InitRandStream()
{
	if (NodeRandSeed > 0)
	{
		if (NodeRandStream.GetInitialSeed() != NodeRandSeed) {
			NodeRandStream.Initialize(NodeRandSeed);
		}
	}
	else if (NodeRandStream.GetInitialSeed() == 0){
		NodeRandStream.GenerateNewSeed();
	}
}


void AResourceNodeBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AResourceNodeBase, CurrentHealth);
	DOREPLIFETIME(AResourceNodeBase, TotalResources);
	DOREPLIFETIME(AResourceNodeBase, ResourcesByDamageCurrent);
	DOREPLIFETIME(AResourceNodeBase, NodeResourceType);
	DOREPLIFETIME(AResourceNodeBase, NodeRandSeed);
}


void AResourceNodeBase::SetResources_Implementation(const FResourceType& NewNodeResourceType, const TArray<FResourceQuantity>& NewTotalResources, const float PercentResourcesByDamage)
{
	NodeResourceType = NewNodeResourceType;
	TotalResources = NewTotalResources;

	// If total resources is empty
	if (!UResourceFunctionLibrary::ContainsAnyQuantity(TotalResources))
	{
		ResourcesByDamage.Empty();
		ResourcesByDamageCurrent.Empty();
		ResourcesOnDestroy.Empty();
		return;
	}
	// Distribute total resources
	if (PercentResourcesByDamage <= 0.0f)
	{
		// All go to ResourcesOnDestroy
		ResourcesByDamage.Empty();
		ResourcesOnDestroy.Empty(TotalResources.Num());
		ResourcesOnDestroy.Append(TotalResources);
	}
	else
	{
		if (PercentResourcesByDamage >= 1.0f)
		{
			// All go to ResourcesByDamage
			ResourcesByDamage.Empty(TotalResources.Num());
			ResourcesByDamage.Append(TotalResources);
			ResourcesOnDestroy.Empty();
		}
		else 
		{
			// Allocate percentage of resources
			UResourceFunctionLibrary::MultiplyResourceQuantity(TotalResources, PercentResourcesByDamage, true, ResourcesByDamage);
			UResourceFunctionLibrary::SubtractResourceQuantities(TotalResources, ResourcesByDamage, ResourcesOnDestroy, true);
		}
	}
	ResourcesByDamageCurrent = ResourcesByDamage;
	if (GetLocalRole() == ROLE_Authority)
	{
		OnRep_NodeResourceType();
		OnRep_TotalResources();
	}
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
			//UE_LOG(LogTRGame, Log, TEXT("ExtractedResourcesForDamage - target resource type: %s Found resource rate: %s  %.2f"), *CurResource.ResourceType.Code.ToString(), *FoundRate.ResourceTypeFilter.Code.ToString(), FoundRate.Rate);
			ExtractedQuantity = FMath::CeilToFloat((CurResource.Quantity * DamagePercent) * FoundRate.Rate);
			if (ExtractedQuantity <= 0.0f) { continue; }
			if (ExtractedQuantity > CurResource.Quantity) { ExtractedQuantity = CurResource.Quantity; }
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
		if (UResourceFunctionLibrary::FindBestResourceRateFilter(ExtractionRates, CurResource.ResourceType, FoundRate, FoundSimilarity))
		{
			ExtractedQuantity = FMath::RoundHalfToZero(CurResource.Quantity * FoundRate.Rate);
			if (ExtractedQuantity < 0.0f) { ExtractedQuantity = 0.0f; }
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


bool AResourceNodeBase::ServerExtractResources_Validate(const TArray<FResourceQuantity>& ExtractQuantities)
{
	return true;
}


void AResourceNodeBase::OnRep_CurrentHealth_Implementation()
{
	if (CurrentHealth <= 0.0f)	{
		OnNodeDestroyed.Broadcast();
	}
}


void AResourceNodeBase::OnRep_ResourcesByDamageCurrent()
{
}


void AResourceNodeBase::OnRep_NodeResourceType_Implementation()
{
	InitRandStream();
}


void AResourceNodeBase::OnRep_TotalResources_Implementation()
{
}


void AResourceNodeBase::ServerSetCurrentHealth_Implementation(const float NewCurrentHealth)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = NewCurrentHealth;
		// Replication will call this on clients. This is a direct call for server.
		OnRep_CurrentHealth();	
	}	
}


bool AResourceNodeBase::ServerSetCurrentHealth_Validate(const float NewCurrentHealth)
{
	return true;
}


void AResourceNodeBase::ServerDeltaCurrentHealth_Implementation(const float CurrentHealthDelta)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth += CurrentHealthDelta;
		// Replication will call this on clients. This is a direct call for server.
		OnRep_CurrentHealth();
	}
}


bool AResourceNodeBase::ServerDeltaCurrentHealth_Validate(const float CurrentHealthDelta)
{
	return true;
}


TArray<FResourceQuantity> AResourceNodeBase::GetResourceQuantities_Implementation()
{
	return TotalResources;
}


float AResourceNodeBase::GetResourceQuantity_Implementation(const FResourceType& ResourceType)
{
	return UResourceFunctionLibrary::GetResourceQuantity(TotalResources, ResourceType);
}


FText AResourceNodeBase::GetItemDisplayName_Implementation()
{
	if (TotalResources.Num() == 0) { return FText(); }
	if (TotalResources.Num() > 0)
	{
		return FText::FromString(UResourceFunctionLibrary::GoodsNameForResource(TotalResources[0].ResourceType).ToString());
	}
	return FText();
}


FInspectInfo AResourceNodeBase::GetInspectInfo_Implementation()
{
	FInspectInfo Info;
	Info.DisplayName = GetItemDisplayName();
	for (FResourceQuantity Resource : TotalResources)
	{
		Info.DetailInfo.Add(FInspectInfoItem(FText::FromString(UResourceFunctionLibrary::GoodsNameForResource(Resource.ResourceType).ToString()), FText::FromString(FString::Printf(TEXT("%.0f"), Resource.Quantity))));
	}
	return Info;
}