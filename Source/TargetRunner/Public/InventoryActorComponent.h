// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoodsQuantity.h"
#include "InventoryActorComponent.generated.h"

// Event dispatcher for when CurrentValue changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryChanged, const TArray<FGoodsQuantity>&, ChangedItems);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TARGETRUNNER_API UInventoryActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryActorComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<FGoodsQuantity> Inventory;

	// Delegate event when inventory has changed.
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnInventoryChanged OnInventoryChanged;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	bool ShouldUpdateClient();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// [Any]
	// Adds (or subtracts) a quantity of goods from inventory. Returns true if adjustment could be made, false otherwise (ex: if amount to remove is > current inventory)
	// Note: actual changes to inventory are made by replicated functions (ServerAddSubtractGoods, ClientUpdateInventoryQuantity).
	UFUNCTION(BlueprintCallable)
		bool AddSubtractGoods(const FGoodsQuantity& GoodsDelta, float& CurrentQuantity);

	// [Server]
	// Called from AddSubtractGoods()
	// 
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerAddSubtractGoods(const FGoodsQuantity& GoodsDelta);

	// [Any]
	// Adds (or subtracts) quantities of goods from inventory. Returns true if all adjustments could be made, false otherwise (ex: if amount to remove is > current inventory)
	// Note: actual changes to inventory are made by replicated functions (ServerAddSubtractGoodsArray, ClientUpdateInventoryQuantity).
	UFUNCTION(BlueprintCallable)
		bool AddSubtractGoodsArray(const TArray<FGoodsQuantity>& GoodsDeltas, TArray<FGoodsQuantity>& CurrentQuantities);

	// [Server]
	// Called from AddSubtractGoods()
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerAddSubtractGoodsArray(const TArray<FGoodsQuantity>& GoodsDeltas);

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
};
