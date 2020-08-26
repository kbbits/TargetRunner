// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoodsQuantity.h"
#include "InventoryActorComponent.generated.h"

// Event dispatcher for when CurrentValue changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryChanged, const TArray<FGoodsQuantity>&, ChangedItems);


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

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<FGoodsQuantity> Inventory;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	bool ShouldUpdateClient();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// [Any]
	// Call this one to Add (or subtract) a quantity of goods from inventory. Returns true if adjustment could be made, false otherwise (ex: if amount to remove is > current inventory)
	//  bNegateGoodsQuantities - Set this to true to have each goods quantity multiplied by -1.0. (to simplify removing goods using postitive goods quantities)
	// Note: actual changes to inventory are made by replicated functions that this calls. (ServerAddSubtractGoods, ClientUpdateInventoryQuantity).
	UFUNCTION(BlueprintCallable)
		bool AddSubtractGoods(const FGoodsQuantity& GoodsDelta, const bool bNegateGoodsQuantities, float& CurrentQuantity);

	// [Server]
	// Called from AddSubtractGoods()
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerAddSubtractGoods(const FGoodsQuantity& GoodsDelta, const bool bNegateGoodsQuantities);

	// [Any]
	// Call this to Add (or subtract) quantities of goods from inventory. Returns true if all adjustments could be made, false otherwise (ex: if amount to remove is > current inventory)
	//  bNegateGoodsQuantities - Set this to true to have each goods quantity multiplied by -1.0. (to simplify removing goods using postitive goods quantities)
	// Note: actual changes to inventory are made by replicated functions that this calls. (ServerAddSubtractGoodsArray, ClientUpdateInventoryQuantity).
	UFUNCTION(BlueprintCallable)
		bool AddSubtractGoodsArray(const TArray<FGoodsQuantity>& GoodsDeltas, const bool bNegateGoodsQuantities, TArray<FGoodsQuantity>& CurrentQuantities);

	// [Server]
	// Called from AddSubtractGoods()
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerAddSubtractGoodsArray(const TArray<FGoodsQuantity>& GoodsDeltas, const bool bNegateGoodsQuantities);

	// [Client]
	// Called from ServerAddSubtractGoods()
	// Updates client side quantity in client inventory.
	UFUNCTION(Client, Reliable, WithValidation)
		void ClientUpdateInventoryQuantity(const FGoodsQuantity NewQuantity);

	// [Client]
	// Called from ServerAddSubtractGoodsArray()
	// Updates client side quantities in client inventory.
	UFUNCTION(Client, Reliable, WithValidation)
		void ClientUpdateInventoryQuantities(const TArray<FGoodsQuantity>& NewQuantities);

	// [Any]
	// Get the current quantity of the given goods.
	UFUNCTION(BlueprintPure)
		float GetGoodsCount(const FName GoodsName);

	// [Any]
	// Get the current quantities of all goods in inventory.
	UFUNCTION(BlueprintPure)
		void GetAllGoods(TArray<FGoodsQuantity>& AllGoods);

	// [Any]
	// Check if the inventory contains the given goods
	UFUNCTION(BlueprintPure)
		bool HasGoods(const FGoodsQuantity Goods, float& CurrrentQuantity);

	// [Any]
	// Check that the inventory contains all of the given goods
	UFUNCTION(BlueprintPure)
		bool HasAllGoods(const TArray<FGoodsQuantity> Goods, TArray<FGoodsQuantity>& CurrrentQuantities);

};
