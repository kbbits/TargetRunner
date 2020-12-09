// Fill out your copyright notice in the Description page of Project Settings.


#include "TREnemyCharacter.h"
#include "TREnemyAIController.h"
#include "BrainComponent.h"
#include "ResourceFunctionLibrary.h"

// Sets default values
ATREnemyCharacter::ATREnemyCharacter()
{
	bReplicates = true;
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Level = 1;
	HealthAttribute = CreateDefaultSubobject<UActorAttributeComponent>(TEXT("HealthAttribute"));
	AddOwnedComponent(HealthAttribute);
	HealthAttribute->SetIsReplicated(true);
	HealthAttribute->AttributeData.Name = FName(TEXT("Health"));
	HealthAttribute->AttributeData.MinValue = 0.f;
	HealthAttribute->AttributeData.MaxValue = 5.f;
	HealthAttribute->AttributeData.CurrentValue = 5.f;
	HealthAttribute->OnHitMinimum.AddDynamic(this, &ATREnemyCharacter::OnHealthDepleted);
}

void ATREnemyCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATREnemyCharacter, StasisState);
}

// Called when the game starts or when spawned
void ATREnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	ScaleToLevel();
	ResetAttributesToMax();
	OnRep_StasisState(StasisState);
}


// Called every frame
void ATREnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ATREnemyAIController* AI = GetTRAIController();
	AActor* MyTarget = nullptr;
	float Distance;

	bTargetInRange = false;
	if (AI)
	{
		MyTarget = AI->GetCurrentTarget();
		if (MyTarget)
		{
			Distance = GetDistanceTo(MyTarget);
			if (GetMinAttackRange() <= Distance && Distance <= GetMaxAttackRange())
			{
				bTargetInRange = true;
			}
		}
		else
		{
			bAimOnTarget = false;
		}
	}
}



void ATREnemyCharacter::ScaleToLevel_Implementation()
{
	bScaleUpApplied = true;
}


// Called to bind functionality to input
void ATREnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


void ATREnemyCharacter::OnRep_StasisState_Implementation(ETRStasisState OldState)
{
	ATREnemyAIController* AIController = GetTRAIController();
	if (AIController == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("TREnemyCharacter - no AI Controller found"));
		return;
	}
	if (StasisState == ETRStasisState::Awake)
	{
		SetActorTickEnabled(true);
		AIController->SetActorTickEnabled(true);
		if (AIController->GetBrainComponent()) {
			AIController->GetBrainComponent()->StartLogic();
		}
		// Handle components?
		UE_LOG(LogTRGame, Log, TEXT("TREnemyCharacter - Stasis Awakened: %s"), *GetNameSafe(this));
	}
	else if (StasisState == ETRStasisState::InStasis)
	{
		StopAnimMontage();
		SetActorTickEnabled(false);
		AIController->SetActorTickEnabled(false);
		if (AIController->GetBrainComponent()) {
			AIController->GetBrainComponent()->StopLogic(FString(TEXT("Stasis")));
		}
		AIController->StopMovement();
		UE_LOG(LogTRGame, Log, TEXT("TREnemyCharacter - Put In Stasis: %s"), *GetNameSafe(this));
	}
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


ETRStasisState ATREnemyCharacter::GetStasisState_Implementation()
{
	return StasisState;
}


void ATREnemyCharacter::StasisSleep_Implementation()
{
	ETRStasisState OldState = StasisState;
	StasisState = ETRStasisState::InStasis;
	if (GetLocalRole() == ROLE_Authority)
	{
		// Must call OnRep directly on server
		OnRep_StasisState(OldState);
	}
}


void ATREnemyCharacter::StasisWake_Implementation()
{
	ETRStasisState OldState = StasisState;
	StasisState = ETRStasisState::Awake;
	if (GetLocalRole() == ROLE_Authority)
	{
		// Must call OnRep directly on server
		OnRep_StasisState(OldState);
	}
}