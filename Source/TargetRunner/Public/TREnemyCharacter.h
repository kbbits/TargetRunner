// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NiagaraSystem.h"
#include "ActorAttributeComponent.h"
#include "ExtractableResource.h"
#include "StasisObject.h"
#include "ResourceQuantity.h"
#include "ResourceRateFilter.h"
#include "TREnemyCharacter.generated.h"

UCLASS()
class TARGETRUNNER_API ATREnemyCharacter : public ACharacter, public IExtractableResource, public IStasisObject
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATREnemyCharacter();

	// Base health of enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		UActorAttributeComponent* HealthAttribute;

	// True if aim is completed (or irrelevant) and weapon is aligned to target.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, SaveGame)
		bool bAimOnTarget;

	// True if the current target is within MinAttackRange to MaxAttackRange.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, SaveGame)
		bool bTargetInRange;

	// True if we have Line of sight to the current target.
	// NOT IMPLEMENTED
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, SaveGame)
		bool bLosToTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSoftObjectPtr<USoundBase> DeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSoftObjectPtr<UNiagaraSystem> DeathParticleSystem;

	// Resources that will be extracted when the character's health reaches 0.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", TitleProperty = "ResourceType"))
		TArray<FResourceQuantity> ResourcesOnDestroy;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_StasisState)
		ETRStasisState StasisState;

private:

	ATREnemyAIController* TRAIController;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Statis replication function.
	// Stop/Start tick on this actor and stop/start tick and logic in it's AIController
	UFUNCTION(BlueprintNativeEvent)
		void OnRep_StasisState(ETRStasisState OldState);

	//UFUNCTION()
	//	virtual void OnRep_ResourcesByDamageCurrent();

	//UFUNCTION()
	//	virtual void OnRep_ResourcesAfterDestroyCurrent();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void ResetAttributesToMax();

	// Notification called when health reaches minimum value (i.e. = 0)
	// Base class does nothing.
	UFUNCTION(BlueprintNativeEvent)
		void OnHealthDepleted();

	// The minimum range to attack from.
	// Base class implementation returns 100.0.
	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
		float GetMinAttackRange();

	// The maximum range to attack from.
	// Base class implementation returns 1500.0.
	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
		float GetMaxAttackRange();

	// Base class implementation returns true if current health > 0, false otherwise.
	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
		bool IsDead();

	// Helper to get controller cast to TREnemyAIController
	UFUNCTION(BlueprintPure)
		ATREnemyAIController* GetTRAIController();

	// Calculates the amount of resources that should be extracted upon destruction, with the given extraction rates.
	// Returns true if any resources were calculated to be extracted.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		bool GetExtractedResourcesOnDestroy(const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities);
	//virtual bool ExtractedResourcesOnDestroy_Implementation(const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities);

	// IExtractableResource interface functions

	// Gets the current quanties of resources available in this entity.
	virtual TArray<FResourceQuantity> GetResourceQuantities_Implementation();

	// Gets the current quanty of a resource available in this entity.
	virtual float GetResourceQuantity_Implementation(const FResourceType ResourceType);

	// IStasisObject interface functions

	// Gets current statsis state
	virtual ETRStasisState GetStasisState_Implementation() override;
	
	// [Call on Server]
	// Stop tick and AI.
	virtual void StasisSleep_Implementation() override;

	// [Call on Server]
	// Start tick and AI
	virtual void StasisWake_Implementation() override;
	
};
