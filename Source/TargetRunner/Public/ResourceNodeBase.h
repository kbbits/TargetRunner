// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Delegates/Delegate.h"
#include "UnrealNetwork.h"
#include "ResourceType.h"
#include "ResourceQuantity.h"
#include "ResourceRateFilter.h"
#include "ExtractableResource.h"
#include "TargetRunner.h"
#include "ResourceNodeBase.generated.h"

// Event dispatcher for when we are destroyed
// Only called on server.
UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNodeDestroyed);

// A harvestable resource. 
UCLASS()
class TARGETRUNNER_API AResourceNodeBase : public AActor, public IExtractableResource
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AResourceNodeBase();

	
	// Starting health for this node
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
		float BaseHealth;

	// Current health of this node.  When health reaches 0 ResourcesOnDestroy will be extracted if damager implements IExtractsResources.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, ReplicatedUsing=OnRep_CurrentHealth)
		float CurrentHealth;

	// The ResourceType of this node. This affects, for example, the damage done by sources that implement IDoesDamageByType interface.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
		FResourceType NodeResourceType;

	// Total resources that will be extracted as the node is damaged. 
	// Amount extracted during each damage event is proportional to damage dealt as percent of node base health.
	// ex: if a damage event does damage equal to 20% of the node's base health then 20% of the node's ResourcesByDamage will be extracted.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
		TArray<FResourceQuantity> ResourcesByDamage;

	// Tracks remaining ResourceByDamage quantities.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_ResourcesByDamageCurrent)
		TArray<FResourceQuantity> ResourcesByDamageCurrent;

	// Resources that will be extracted when the node's health reaches 0.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
		TArray<FResourceQuantity> ResourcesOnDestroy;

protected:
	// Root scene for the node
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USceneComponent* RootScene;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Calculates the amount of resources that should be extracted for a given amount of damage and extraction rates.
	// Makes no changes to current resource quantities.
	// Returns true if any resources were calculated to be extracted.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		bool ExtractedResourcesForDamage(const float Damage, const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities);
	virtual bool ExtractedResourcesForDamage_Implementation(const float Damage, const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities);

	// Reduces the current values in ResourcesForDamageCurrent.
	UFUNCTION(Server, Reliable, BlueprintCallable)
		void ServerExtractResources(const TArray<FResourceQuantity>& ExtractQuantities);
	virtual void ServerExtractResources_Implementation(const TArray<FResourceQuantity>& ExtractQuantities);

	// Calculates the amount of resources that should be extracted upon destruction with the given extraction rates.
	// Returns true if any resources were calculated to be extracted.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		bool ExtractedResourcesOnDestroy(const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities);
	virtual bool ExtractedResourcesOnDestroy_Implementation(const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities);

	// Replication notification
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void OnRep_CurrentHealth();
	void OnRep_CurrentHealth_Implementation();

	UFUNCTION()
		virtual void OnRep_ResourcesByDamageCurrent();

	// Use this function to set current health at runtime. Calls OnRep_CurrentHealth for server too.
	UFUNCTION(Server, Reliable, BlueprintCallable)
		void ServerSetCurrentHealth(const float NewCurrentHealth);
	virtual void ServerSetCurrentHealth_Implementation(const float NewCurrentHealth);

	// Delegates

	// Hit minimum value event
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnNodeDestroyed OnNodeDestroyed;

	// IExtractableResource interface functions

	// Gets the current quanties of resources available in this entity.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Extractable Resource")
		TArray<FResourceQuantity> GetResourceQuantities();
	virtual TArray<FResourceQuantity> GetResourceQuantities_Implementation();

	// Gets the current quanty of a resource available in this entity.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Extractable Resource")
		float GetResourceQuantity(const FResourceType ResourceType);
	virtual float GetResourceQuantity_Implementation(const FResourceType ResourceType);

	
	
};
