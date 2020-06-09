// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DoesDamageByType.h"
#include "TRProjectileBase.h"
#include "ExtractsResources.h"
#include "TRProjectileExtractorBase.generated.h"

/*
 * A Projectile that also extracts resources.
*/
UCLASS()
class TARGETRUNNER_API ATRProjectileExtractorBase : public ATRProjectileBase, public IExtractsResources
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATRProjectileExtractorBase();

	// The effective extraction rates of this projectile against given resource types. Expressed as a percent (0.0 - 1.0).
	// Values > 1.0 are allowed. Calculated extracted amounts against a resource type not matching any filters will = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
		TArray<FResourceRateFilter> ResourceExtractionRates;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// IExtractsResources interface functions

	// Get this projectile's resource extraction rates
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Extracts Resources")
		TArray<FResourceRateFilter> GetExtractionRates();
	virtual TArray<FResourceRateFilter> GetExtractionRates_Implementation();
	
};
