// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DoesDamageByType.h"
#include "ResourceType.h"
#include "ResourceRateFilter.h"
#include "TRProjectileBase.generated.h"

UCLASS()
class TARGETRUNNER_API ATRProjectileBase : public AActor, public IDoesDamageByType
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATRProjectileBase();

	// The basic damage this projectile will deal. Final damage calculated by DoesDamageByType logic.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons", meta = (ExposeOnSpawn = "true"))
		float BaseDamage;

	// The effective damage rates of this projectile against given resource types. Expressed as a percent of base damage (0.0 - 1.0).
	// Values > 1.0 are allowed. Calculated damage against a resource type not matching any filters will = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons", meta = (ExposeOnSpawn = "true", TitleProperty="ResourceTypeFilter"))
		TArray<FResourceRateFilter> BaseDamageRates;

	// The number of times projectile will bounce when hitting world surfaces. Default = 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons", meta = (ExposeOnSpawn = "true"))
		int32 Bounces;

	// The the speed of the projectile,
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons", meta = (ExposeOnSpawn = "true"))
		float ProjectileSpeed;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// IDoesDamageByType interface functions

	// Get this projectile's damage rates
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons", meta = (TitleProperty = "ResourceTypeFilter"))
		TArray<FResourceRateFilter> GetDamageRates();
	virtual TArray<FResourceRateFilter> GetDamageRates_Implementation();

	// Calculates the damage done by this entity to the given resource type.
	// Returns true if CalculatedDamage > 0.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		bool CalculateDamageByType(const float Damage, const FResourceType& ResourceType, float& CalculatedDamage);
	virtual bool CalculateDamageByType_Implementation(const float Damage, const FResourceType& ResourceType, float& CalculatedDamage);

};
