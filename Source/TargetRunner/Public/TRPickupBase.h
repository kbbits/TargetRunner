// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnrealNetwork.h"
#include "CollectableResource.h"
#include "TRPickupBase.generated.h"

UCLASS()
class TARGETRUNNER_API ATRPickupBase : public AActor, public ICollectableResource
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATRPickupBase(const FObjectInitializer& OI);

	// Goods collected from this pickup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
		TArray<FGoodsQuantity> PickupGoods;

	// If this pickup has already been collected - but still exists for some reason.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_Collected)
		bool bCollected;

protected:
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	//	USceneComponent* RootScene;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Replication notification of collected state
	UFUNCTION()
	virtual void OnRep_Collected();

	// ICollectableResource interface functions

	// Get the resource goods to collect.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Resource Collecting")
		void GetResourceGoods(TArray<FGoodsQuantity>& CollectedGoods);
	virtual void GetResourceGoods_Implementation(TArray<FGoodsQuantity>& CollectedGoods);

	// Call this to notify this entity that it has been collected.
	// Generally this entity will then destroy itself.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Resource Collecting")
		void NotifyCollected();
	virtual void NotifyCollected_Implementation();

};
