// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoodsQuantity.h"
#include "InventoryActorComponent.generated.h"

// Event dispatcher for when CurrentValue changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, const TArray<FGoodsQuantity>&, GoodsDeltas, const TArray<FGoodsQuantity>&, ChangedTotals);

/*
* Manages an actor's inventory of goods. 
* Replication of inventory content is managed by all changes going through server. Each of those changes is 
* replicated to owning client via RPCs called from the ServerXxxx functions.
* 
* TODO: Consider changing inventory and snapshot to TMap since we are no longer looking at replicating the inventory array itself.
*/
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TARGETRUNNER_API UInventoryActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryActorComponent();

	// Delegate event when inventory has changed.
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnInventoryChanged OnInventoryChanged;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bEnableClassDebug;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (TitleProperty = "Name"))
		TArray<FGoodsQuantity> Inventory;

	UPROPERTY(BlueprintReadOnly, meta = (TitleProperty = "Name"))
		TArray<FGoodsQuantity> SnapshotInventory;

	// Any goods name that matches one of these strings will be filtered out of saveable goods in GetSaveableGoods()
	// TODO: This should probably be moved to a different location, e.g. GameMode.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FString> UnsaveableGoodsFilters;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Returns true if we are the server authority and:
	//   Either we are owned by a player controller and it is not a local controller
	//  Or
	//   NetMode is not Client and not Standalone. i.e. there is a client to update
	// Returns false otherwise.
	bool ShouldUpdateClient();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// [Any - Should not usually be called directly]
	// Does the work of adjusting local inventory. Called by the replicated AddSubtractGoods functions.
	//Returns true if adjustment could be made, false otherwise(ex: if amount to remove is > current inventory)
	//   bNegateGoodsQuantities - Set this to true to have each goods quantity multiplied by -1.0. (to simplify removing goods using postitive goods quantities)
	UFUNCTION()
		bool AddSubtractGoodsInternal(const FGoodsQuantity& GoodsDelta, const bool bNegateGoodsQuantities, float& NewCurrentQuantity, FGoodsQuantity& NewSnapshotDelta, const bool bAddToSnapshot = false);

	// [Any]
	// Call this one to Add (or subtract) a quantity of goods from inventory. Returns true if adjustment could be made, false otherwise (ex: if amount to remove is > current inventory)
	//  bNegateGoodsQuantities - Set this to true to have each goods quantity multiplied by -1.0. (to simplify removing goods using postitive goods quantities)
	// Note: actual changes to inventory are made by replicated functions that this calls. (ServerAddSubtractGoods, ClientUpdateInventoryQuantity).
	UFUNCTION(BlueprintCallable)
		bool AddSubtractGoods(const FGoodsQuantity& GoodsDelta, const bool bNegateGoodsQuantities, float& CurrentQuantity, const bool bAddToSnapshot = false);

	// [Server]
	// Called from AddSubtractGoods()
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerAddSubtractGoods(const FGoodsQuantity& GoodsDelta, const bool bNegateGoodsQuantities, const bool bAddtoSnapshot = false);

	// [Any - Should not usually be called directly]
	// Does the work of add/subtract quantities from local inventory. The other replicated AddSubtractGoodsArray functions call this.
	// Returns true if all adjustments could be made, false otherwise(ex: if amount to remove is > current inventory)
	//  bNegateGoodsQuantities - Set this to true to have each goods quantity multiplied by -1.0. (to simplify removing goods using postitive goods quantities)
	UFUNCTION()
		bool AddSubtractGoodsArrayInternal(const TArray<FGoodsQuantity>& GoodsDeltas, const bool bNegateGoodsQuantities, TArray<FGoodsQuantity>& CurrentQuantities, TArray<FGoodsQuantity>& NewSnapshotDeltas, const bool bAddToSnapshot = false);

	// [Any]
	// Call this to Add (or subtract) quantities of goods from inventory. Returns true if all adjustments could be made, false otherwise (ex: if amount to remove is > current inventory)
	//  bNegateGoodsQuantities - Set this to true to have each goods quantity multiplied by -1.0. (to simplify removing goods using postitive goods quantities)
	// Note: actual changes to inventory are made by replicated functions that this calls. (ServerAddSubtractGoodsArray, ClientUpdateInventoryQuantity).
	UFUNCTION(BlueprintCallable)
		bool AddSubtractGoodsArray(const TArray<FGoodsQuantity>& GoodsDeltas, const bool bNegateGoodsQuantities, TArray<FGoodsQuantity>& CurrentQuantities, const bool bAddToSnapshot = false);

	// [Server]
	// Called from AddSubtractGoods()
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerAddSubtractGoodsArray(const TArray<FGoodsQuantity>& GoodsDeltas, const bool bNegateGoodsQuantities,  const bool bAddToSnapshot = false);

	// [Any - Not usually called directly]
	// Non-replicated function that does the work of setting the inventory values.
	// The replicated "XxxxSetInventory" functions call this.
	UFUNCTION()
		void SetInventoryInternal(const TArray<FGoodsQuantity>& NewGoods, const TArray<FGoodsQuantity>& NewSnapshotGoods);

	// [Server]
	// Sets the contents of inventory. Usually don't need to call this manually - but is useful to reset state after a load. 
	// This will call client if needed to handle replication.
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void ServerSetInventory(const TArray<FGoodsQuantity>& NewGoods, const TArray<FGoodsQuantity>& NewSnapshotGoods);

	// [Client]
	// Called from ServerSetInventory()
	// Updates client side quantities - sets quantity to new quantity also sets snapshot inventory to new snapshot quantities.
	UFUNCTION(Client, Reliable, WithValidation)
		void ClientSetInventory(const TArray<FGoodsQuantity>& NewGoods, const TArray<FGoodsQuantity>& NewSnapshotGoods);

	// [Client]
	// Called from ServerAddSubtractGoods()
	// Updates client side quantity - sets quantity to new quantity. Snapshot delta is added to existing snapshot quantities.
	UFUNCTION(Client, Reliable, WithValidation)
		void ClientUpdateInventoryQuantity(const FGoodsQuantity NewQuantity, const FGoodsQuantity SnapshotDelta);

	// [Client]
	// Called from ServerAddSubtractGoodsArray()
	// Updates client side quantities - sets quantity to new quantity. Snapshot deltas are added to existing snapshot quantities.
	UFUNCTION(Client, Reliable, WithValidation)
		void ClientUpdateInventoryQuantities(const TArray<FGoodsQuantity>& NewQuantities, const TArray<FGoodsQuantity>& SnapshotDeltas);

	// [Any]
	// Get the current quantity of the given goods.
	UFUNCTION(BlueprintPure)
		float GetGoodsCount(const FName GoodsName);

	// [Any]
	// Get the current quantities of all goods in inventory.
	UFUNCTION(BlueprintPure)
		void GetAllGoods(TArray<FGoodsQuantity>& AllGoods);

	// [Any]
	// Get all goods that pass the save block filters. See UnsaveableGoodsFilters.
	UFUNCTION(BlueprintPure)
		void GetSaveableGoods(TArray<FGoodsQuantity>& AllSaveableGoods);

	// [Any]
	// Check if the inventory contains the given goods
	UFUNCTION(BlueprintPure)
		bool HasGoods(const FGoodsQuantity Goods, float& CurrrentQuantity);

	// [Any]
	// Check that the inventory contains all of the given goods
	UFUNCTION(BlueprintPure)
		bool HasAllGoods(const TArray<FGoodsQuantity> Goods, TArray<FGoodsQuantity>& CurrrentQuantities);

	// [Any]
	// Returns true if inventory is empty.
	UFUNCTION(BlueprintPure)
		bool IsEmpty();
	
	// [Server]
	// Sets the contents of snapshot inventory. Usually don't need to call this manually - useful to reset state after a load. 
	// This will call client if needed to handle replication.
	//UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
	//	void ServerSetSnapshotInventory(const TArray<FGoodsQuantity>& NewSnapshotGoods);

	// [Client]
	// Should not need to call directly. Called by ServerSetSnapshotInventory.
	//UFUNCTION(Client, Reliable, WithValidation)
	//	void ClientSetSnapshotInventory(const TArray<FGoodsQuantity>& NewSnapshotGoods);

	// [Server]
	// Empties the snapshot inventory. 
	// This will call client if needed to handle replication.
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void ServerClearSnapshotInventory();

	// [Client]
	// Should not need to call directly. Called by ServerClearSnapshotInventory.
	UFUNCTION(Client, Reliable, WithValidation)
		void ClientClearSnapshotInventory();

	// [Any]
	// Get goods quantities since last snapshot was started.
	UFUNCTION(BlueprintPure)
		void GetSnapshotGoods(TArray<FGoodsQuantity>& AllSnapshotGoods);

	// [Any]
	// Returns true if snapshot inventory is empty.
	UFUNCTION(BlueprintPure)
		bool SnapshotInventoryIsEmpty();

};
