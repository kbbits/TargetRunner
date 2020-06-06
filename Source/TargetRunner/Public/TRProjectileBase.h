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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons", meta = (ExposeOnSpawn = "true"))
		float BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons", meta = (ExposeOnSpawn = "true"))
		TArray<FResourceRateFilter> BaseDamageRates;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// IDoesDamageByType interface functions

	// Get this projectile's damage rates
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
		TArray<FResourceRateFilter> GetDamageRates();
	virtual TArray<FResourceRateFilter> GetDamageRates_Implementation();

	// Calculates the damage done by this entity to the given resource type.
	// Returns true if CalculatedDamage > 0.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		bool CalculateDamageByType(const float Damage, const FResourceType& ResourceType, float& CalculatedDamage);
	virtual bool CalculateDamageByType_Implementation(const float Damage, const FResourceType& ResourceType, float& CalculatedDamage);

};
