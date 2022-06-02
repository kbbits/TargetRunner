// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnrealNetwork.h"
#include "CollectableResource.h"
#include "CollectablePickup.h"
#include "InspectableItem.h"
#include "TRPickupBase.generated.h"

UCLASS()
class TARGETRUNNER_API ATRPickupBase : public AActor, public ICollectablePickup, public IInspectableItem
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATRPickupBase(const FObjectInitializer& OI);

	// All goods, energy, etc. collected from this pickup.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_PickupAwards, meta = (ExposeOnSpawn = "true"))
		FPickupAwards PickupAwards;

	// DEPRECATED - Use PickupAwards instead. Goods collected from this pickup
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	//	TArray<FGoodsQuantity> PickupGoods;

	// If this pickup has already been collected - but still exists for some reason.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_Collected)
		bool bCollected;

protected:
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	//	USceneComponent* RootScene

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Replication notification of collected state
	UFUNCTION()
		virtual void OnRep_Collected();

	UFUNCTION(BlueprintNativeEvent)
		void OnRep_PickupAwards();
	void OnRep_PickupAwards_Implementation();

	//========= ICollectablePickup interface functions

	/** Get the PickupAwards to collect. 
	 *  Default implementation : if !bCollected it returns all PickupAwards otherwise returns empty set. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickup Collecting")
		void GetPickupAwards(FPickupAwards& PickupsAwarded);
	virtual void GetPickupAwards_Implementation(FPickupAwards& PickupsAwarded);

	/** Get the goods portion of the PickupAwards. 
	 *  Default implementation: if !bCollected it returns all PickupAwards.PickupGoods, otherwise returns an empty array. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickup Collecting")
		void GetPickupGoods(TArray<FGoodsQuantity>& PickupGoods);
	virtual void GetPickupGoods_Implementation(TArray<FGoodsQuantity>& PickupGoods);

	/** Call this to notify this entity that it has been collected.
	 *  Default implementation sets bCollected = true then destroys self. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickup Collecting")
		void NotifyPickupCollected();
	virtual void NotifyPickupCollected_Implementation();

	//======== IInspectableItem interface functions

	// Base class does nothing
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inspectable Item")
		void OnBeginInspect();

	// Base class does nothing.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inspectable Item")
		void OnEndInspect();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inspectable Item")
		FText GetItemDisplayName();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inspectable Item")
		FInspectInfo GetInspectInfo();

	//======== ICollectableResource interface functions

	// Get the resource goods to collect. Default implementation: if !bCollected it returns all PickupAwards.PickupGoods otherwise returns empty array.
	//UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Resource Collecting")
	//	void GetResourceGoods(TArray<FGoodsQuantity>& CollectedGoods);
	//virtual void GetResourceGoods_Implementation(TArray<FGoodsQuantity>& CollectedGoods);

	// Call this to notify this entity that it has been collected.
	// Default implementation sets bCollected = true then destroys self.
	//UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Resource Collecting")
	//	void NotifyCollected();
	//virtual void NotifyCollected_Implementation();
};
