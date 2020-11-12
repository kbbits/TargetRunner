// Fill out your copyright notice in the Description page of Project Settings.


#include "TREnemyCharacter.h"
#include "TREnemyAIController.h"
#include "ResourceFunctionLibrary.h"

// Sets default values
ATREnemyCharacter::ATREnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthAttribute = CreateDefaultSubobject<UActorAttributeComponent>(TEXT("HealthAttribute"));
	AddOwnedComponent(HealthAttribute);
	HealthAttribute->SetIsReplicated(true);
	HealthAttribute->AttributeData.Name = FName(TEXT("Health"));
	HealthAttribute->AttributeData.MinValue = 0.f;
	HealthAttribute->AttributeData.MaxValue = 5.f;
	HealthAttribute->AttributeData.CurrentValue = 5.f;
	HealthAttribute->OnHitMinimum.AddDynamic(this, &ATREnemyCharacter::OnHealthDepleted);
}

// Called when the game starts or when spawned
void ATREnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	ResetAttributesToMax();
}


// Called every frame
void ATREnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


// Called to bind functionality to input
void ATREnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


void ATREnemyCharacter::ResetAttributesToMax_Implementation()
{
	TArray<UActorAttributeComponent*> AttributeComps;
	GetComponents<UActorAttributeComponent>(AttributeComps);
	for (UActorAttributeComponent* Attr : AttributeComps)
	{
		Attr->ResetToMax();
	}
}


void ATREnemyCharacter::OnHealthDepleted_Implementation()
{

}


float ATREnemyCharacter::GetMinAttackRange_Implementation()
{
	return 100.f;
}


float ATREnemyCharacter::GetMaxAttackRange_Implementation()
{
	return 1500.f;
}


bool ATREnemyCharacter::IsDead_Implementation()
{
	return HealthAttribute->GetCurrent() <= 0.f;
}


ATREnemyAIController* ATREnemyCharacter::GetTRAIController()
{
	if (TRAIController == nullptr)
	{
		TRAIController = Cast<ATREnemyAIController>(GetController());
	}	
	return TRAIController;
}


bool ATREnemyCharacter::GetExtractedResourcesOnDestroy_Implementation(const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities)
{
	UResourceFunctionLibrary::CalculateExtractedResources(ResourcesOnDestroy, 1.f, ExtractionRates, ExtractedQuantities);
	return ExtractedQuantities.Num() > 0;
}


TArray<FResourceQuantity> ATREnemyCharacter::GetResourceQuantities_Implementation()
{
	//return UResourceFunctionLibrary::AddResourceQuantities(ResourcesByDamageCurrent, ResourcesOnDestroy);
	if (IsDead()) { 
		return TArray<FResourceQuantity>(); 
	}
	return ResourcesOnDestroy;
}

float ATREnemyCharacter::GetResourceQuantity_Implementation(const FResourceType ResourceType)
{
	float QuantityTotal = 0.0f;
	//for (FResourceQuantity CurQuantity : ResourcesByDamage)
	//{
	//	if (CurQuantity.ResourceType == ResourceType) { QuantityTotal += CurQuantity.Quantity; }
	//}
	for (FResourceQuantity CurQuantity : ResourcesOnDestroy)
	{
		if (CurQuantity.ResourceType == ResourceType) { QuantityTotal += CurQuantity.Quantity; }
	}
	return QuantityTotal;
}