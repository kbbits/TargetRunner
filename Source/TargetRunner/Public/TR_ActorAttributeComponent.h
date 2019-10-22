// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "Components/ActorComponent.h"
#include "TR_ActorAttributeComponent.generated.h"


// Event dispatcher for when we hit minimum value
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitAttrMinimum);
//DECLARE_EVENT(UTR_ActorAttributeComponent, FOnHitAttrMinimum);
// Event dispatcher for when we hit maximum value
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitAttrMaximum);
//DECLARE_EVENT(UTR_ActorAttributeComponent, FOnHitAttrMaximum);

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TARGETRUNNER_API UTR_ActorAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTR_ActorAttributeComponent();
		

	// The name of the attribute. ex: Energy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorAttributes")
		FName AttributeName;

	// The minimum value allowed for this attribute
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorAttributes")
		float MinValue = 0;

	// The maximum value allowed for this attribute
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorAttributes")
		float MaxValue = 0;

	// The current value of this attribute
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorAttributes")
		float CurrentValue = 0;

	// Rate of attribute change (per second).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorAttributes")
		float RechargeRate = 0;

	// Rate of attribute change (per second).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorAttributes")
		bool bRechargePaused = false;

	// Hit minimum value event
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnHitAttrMinimum OnHitMinimum;

	// Hit maximum value event
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnHitAttrMaximum OnHitMaximum;

private:
	// flags so we only call event once when we hit a min or max.
	//bool bFiredOnMax = false;
	//bool bFiredOnMin = false;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Get remainin capacity of this attribute. (max value - current value)
	UFUNCTION(BlueprintPure, Category="ActorAttributes")
		float GetRemainingCapacity();

	// Get the current percent of maximum value 0.0 - 1.0
	UFUNCTION(BlueprintPure, Category = "ActorAttributes")
		float GetCurrentPercent();

	// Add the given amount from the current value of this attribute. Value can be negative.
	UFUNCTION(BlueprintCallable, Category = "ActorAttributes")
		bool DeltaValue(const float ToDeduct, const bool bAllowOverspill);

	UFUNCTION(BlueprintCallable, Category = "ActorAttributes")
		float ResetToMax();
};
