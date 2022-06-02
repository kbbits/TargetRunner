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
#include "InspectableItem.h"
#include "TargetRunner.h"
#include "ResourceNodeBase.generated.h"

// Event dispatcher for when we are destroyed
// Only called on server.
UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNodeDestroyed);

/*
* A harvestable resource.
*/ 
UCLASS()
class TARGETRUNNER_API AResourceNodeBase : public AActor, public IExtractableResource, public IInspectableItem
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AResourceNodeBase();
		
	// Starting health for this node
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
		float BaseHealth;

	// The ResourceType of this node. This affects, for example, the damage done by sources that implement IDoesDamageByType interface.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_NodeResourceType, meta = (ExposeOnSpawn = "true"))
		FResourceType NodeResourceType;

	// The total resources contained in the node.
	// = ResourcesByDamage + ResourcesOnDestroy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_TotalResources, meta = (TitleProperty = "ResourceType"))
		TArray<FResourceQuantity> TotalResources;

	// If this is >= 0, then ResourcesByDamage and ResourcesOnDestroy will be calculated from TotalResources.
	// This is usually set by calling SetResources(). Kept here so we can refer to it later.
	// The percent (expressed 0.0 - 1.0) of TotalResources that will be allocated to ResourcesByDamge.
	// The remaining resources will be allocated to ResourcesOnDestroy.
	// Default = 0.0f ==  All resources allocated to ResourcesOnDestroy
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ResourcesOnDamageSplit;

	// Resources that will be extracted as the node is damaged. 
	// Amount extracted during each damage event is proportional to damage dealt as percent of node base health.
	// ex: if a damage event does damage equal to 20% of the node's base health then 20% of the node's ResourcesByDamage will be extracted.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", TitleProperty = "ResourceType"))
		TArray<FResourceQuantity> ResourcesByDamage;

	// Tracks remaining ResourceByDamage quantities.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_ResourcesByDamageCurrent, meta = (TitleProperty = "ResourceType"))
		TArray<FResourceQuantity> ResourcesByDamageCurrent;

	// Resources that will be extracted when the node's health reaches 0.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", TitleProperty = "ResourceType"))
		TArray<FResourceQuantity> ResourcesOnDestroy;

	// Sound when this node is hit with damage but not destroyed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USoundBase* HitSound;

	// Sound when this node is destroyed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USoundBase* DestroyedSound;


protected:
	// Root scene for the node
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USceneComponent* RootScene;

	// Current health of this node.  When health reaches 0 ResourcesOnDestroy will be extracted if damager implements IExtractsResources.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentHealth)
		float CurrentHealth;

	// Should only be set at spawn or before SetResourceType is called.
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
		int32 NodeRandSeed;

	UPROPERTY(BlueprintReadWrite)
		FRandomStream NodeRandStream;

protected:

	virtual void OnConstruction(const FTransform& Transform) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
		void InitRandStream();

	// Set (or initialize) the resource type and total resources for this node. 
	// Will also calculate and assign the resources by damage and resources on destroy.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void SetResources(const FResourceType& NewNodeResourceType, const TArray<FResourceQuantity>& NewTotalResources, const float PercentResourcesByDamage);

	// Calculates the amount of resources that should be extracted for a given amount of damage and extraction rates.
	// Makes no changes to current resource quantities.
	// Returns true if any resources were calculated to be extracted.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		bool ExtractedResourcesForDamage(const float Damage, const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities);
	virtual bool ExtractedResourcesForDamage_Implementation(const float Damage, const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities);

	// Calculates the amount of resources that should be extracted upon destruction with the given extraction rates.
	// Returns true if any resources were calculated to be extracted.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		bool ExtractedResourcesOnDestroy(const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities);
	virtual bool ExtractedResourcesOnDestroy_Implementation(const TArray<FResourceRateFilter>& ExtractionRates, TArray<FResourceQuantity>& ExtractedQuantities);

	// Reduces the current values in ResourcesForDamageCurrent.
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void ServerExtractResources(const TArray<FResourceQuantity>& ExtractQuantities);
	virtual void ServerExtractResources_Implementation(const TArray<FResourceQuantity>& ExtractQuantities);


	// Replication notification
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void OnRep_CurrentHealth();
	void OnRep_CurrentHealth_Implementation();

	// This also calls InitRandStream
	UFUNCTION(BlueprintNativeEvent)
		void OnRep_NodeResourceType();
	virtual void OnRep_NodeResourceType_Implementation();

	UFUNCTION(BlueprintNativeEvent)
		void OnRep_TotalResources();
	virtual void OnRep_TotalResources_Implementation();

	UFUNCTION()
		virtual void OnRep_ResourcesByDamageCurrent();

	// Use this function to set current health at runtime. Calls OnRep_CurrentHealth for server too.
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void ServerSetCurrentHealth(const float NewCurrentHealth);
	virtual void ServerSetCurrentHealth_Implementation(const float NewCurrentHealth);

	// Use this function to add or subtract to/from current health at runtime. Calls OnRep_CurrentHealth for server too.
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void ServerDeltaCurrentHealth(const float CurrentHealthDelta);
	virtual void ServerDeltaCurrentHealth_Implementation(const float NewCurrentHealth);

	// Delegates

	// Hit minimum health value event
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnNodeDestroyed OnNodeDestroyed;

	//======= IExtractableResource interface functions

	// Gets the current quanties of resources available in this entity.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Extractable Resource")
		TArray<FResourceQuantity> GetResourceQuantities();
	virtual TArray<FResourceQuantity> GetResourceQuantities_Implementation();

	// Gets the current quanty of a resource available in this entity.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Extractable Resource")
		float GetResourceQuantity(const FResourceType& ResourceType);
	virtual float GetResourceQuantity_Implementation(const FResourceType& ResourceType);

	
	//=======  IInspectableItem interface functions

	// Base class does nothing
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inspectable Item")
		void OnBeginInspect();

	// Base class does nothing.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inspectable Item")
		void OnEndInspect();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inspectable Item")
		FText GetItemDisplayName();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inspectable Item")
		FInspectInfo GetInspectInfo();
};
