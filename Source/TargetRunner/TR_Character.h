// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "CollectsResources.h"
#include "PickupAwards.h"
#include "TR_Character.generated.h"

UCLASS()
class TARGETRUNNER_API ATR_Character : public ACharacter, public ICollectsResources
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATR_Character(const FObjectInitializer& OI);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UCapsuleComponent* ResourceCollectionVolume;
	
protected:

	// Adjusts ResourceCollectionVolume component size.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SetupCollectionVolume();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	// Override this in BP and return the scene component that we want to attach the ResourceCollectionVolume to.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		USceneComponent* GetCollectorParentComponent();
	virtual USceneComponent* GetCollectorParentComponent_Implementation();

	// To get BP values
	void PostInitProperties() override;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Collector overlap begin
	// Default implementation handles collecting ICollectablResource(s).
	UFUNCTION(BlueprintNativeEvent)
		void OnCollectorOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	virtual void OnCollectorOverlapBegin_Implementation(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Collector overlap end function
	UFUNCTION(BlueprintImplementableEvent)
		void OnCollectorOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Handle collecting PickupAwards. Base class does nothing.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Pickup Collecting")
		void OnCollectedPickupAwards(const FPickupAwards PickupAwards);

	void FellOutOfWorld(const class UDamageType& DmgType);

	UFUNCTION(BlueprintImplementableEvent)
		void OnFellOutOfWorld();

	// ICollectsResources interface functions

	// Returns true if this resource collector has a homing target, false otherwise.
	// If it does have a homing target, the location is set in TargetLocation.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Resource Collecting")
		bool GetCollectionTargetLocation(FVector& TargetLocation);
	virtual bool GetCollectionTargetLocation_Implementation(FVector& TargetLocation) override;

	// Give this entity resource goods to collect.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Resource Collecting")
		void CollectResourceGoods(const TArray<FGoodsQuantity>& CollectedGoods);
	//virtual void CollectResourceGoods_Implementation(const TArray<FGoodsQuantity>& CollectedGoods) override;

};
