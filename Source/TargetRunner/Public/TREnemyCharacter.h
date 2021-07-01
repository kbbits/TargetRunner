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
#include "ToolData.h"
#include "TREnemyCharacter.generated.h"

UCLASS()
class TARGETRUNNER_API ATREnemyCharacter : public ACharacter, public IExtractableResource, public IStasisObject
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATREnemyCharacter();

	// Determines the scaled difficulty of the enemy
	// Affects the starting health, speed (set on movement component) and other attributes of the enemy.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, meta = (ExposeOnSpawn = "true"))
		int32 Level;

	// Has this character had it's attribute values adjusted (scaled) by level?
	// This will be false before ScaleToLevel() is called and true afterwards.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
		bool bScaleUpApplied;

	// Class of weapon this character will use. 
	// Will be scaled up and applied as appropriate by subclasses.
	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	//	TSubclassOf<class UEnemyToolWeaponBase> WeaponClass;

	// The base walk speed for a Level=1 character
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	//	float BaseWalkSpeed;

	// Health of this character. This is scaled as enemy Level goes up.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		UActorAttributeComponent* HealthAttribute;

	// Determines the accuracy of (projectile) attacks.
	// Valid range is >= 0.0.  1.0 represents "perfect" accuracy. 
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	//	FNamedFloat Accuracy;

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

	// Sound to play on death
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSoftObjectPtr<USoundBase> DeathSound;

	// Particle system to use on death effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSoftObjectPtr<UNiagaraSystem> DeathParticleSystem;

	// Can be set to a valid ResourceType. 
	// If so, damage dealt to this actor by sources that implement DoesDamageByType (i.e. all player weapons)
	// will have damage dealt accordingly.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FResourceType TakesDamageAs;

	// Resources that will be extracted (dropped) when this character's health reaches 0.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", TitleProperty = "ResourceType"))
		TArray<FResourceQuantity> ResourcesOnDestroy;

	// Goods that will be extracted (dropped) when this character's health reaches 0.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", TitleProperty = "Name"))
		TArray<FGoodsQuantity> GoodsOnDestroy;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_StasisState)
		ETRStasisState StasisState;

private:

	ATREnemyAIController* TRAIController;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame.
	// Base class implementation calculates and sets bTargetInRange. Sets bAimOnTarget=false if there is no current target.
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// [Call on Server]
	// Subclasses should override with meaningful implemenation. Base class only sets bScaleUpApplied = true.
	// This should calculate and set the values of the attributes of this character based on it's Level.
	// The default values of each attribute are scaled by level, then the new scaled values are set on the attribute(s).
	// ex: HealthAttribtue, walk/run speed, attack damage, dropped resources & goods
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable )
		void ScaleToLevel();

	// Statis replication function.
	// Stop/Start tick on this actor and stop/start tick and logic in it's AIController
	UFUNCTION(BlueprintNativeEvent)
		void OnRep_StasisState(ETRStasisState OldState);

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

	// Base class implementation returns true if current health <= 0, false otherwise.
	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
		bool IsDead();

	// [Only valid on Server]
	// Helper to get controller cast to TREnemyAIController
	UFUNCTION(BlueprintPure)
		ATREnemyAIController* GetTRAIController();

	// Calculates the amount of resources that should be extracted upon destruction, with the given extraction rates.
	// Returns true if any resources were calculated to be extracted.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		bool GetExtractedResourcesOnDestroy(const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities);
	
	// IExtractableResource interface functions

	// Gets the current quanties of resources available in this entity.
	virtual TArray<FResourceQuantity> GetResourceQuantities_Implementation();

	// Gets the current quanty of a specific resource type available in this entity.
	virtual float GetResourceQuantity_Implementation(const FResourceType& ResourceType);

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
