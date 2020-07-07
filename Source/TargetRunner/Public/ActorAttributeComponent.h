// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "Components/ActorComponent.h"
#include "AttributeData.h"
#include "ActorAttributeComponent.generated.h"

// Event dispatcher for when CurrentValue changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeltaCurrent, float, NewCurrent);
// Event dispatcher for when we hit minimum value
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitAttributeMinimum);
// Event dispatcher for when we hit maximum value
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitAttributeMaximum);


UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TARGETRUNNER_API UActorAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UActorAttributeComponent();

	// The attribute data.
	// Modifying this directly skips notifications and clamping logic.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, SaveGame, Category = "ItemAttributes")
		FAttributeData AttributeData;

	// The name of the attribute. ex: Energy
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemAttributes")
	//	FName Name;

	//// The minimum value allowed for this attribute
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemAttributes")
	//	float MinValue = 0;

	//// The maximum value allowed for this attribute
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemAttributes")
	//	float MaxValue = 0;

	//// The current value of this attribute
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemAttributes")
	//	float CurrentValue = 0;

	// Rate of attribute change (per second).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemAttributes")
		float RechargeRate = 0;

	// If true, attribute recharge will be suspended.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemAttributes")
		bool bRechargePaused = false;

	// Hit minimum value event
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnDeltaCurrent OnDeltaCurrent;
	
	// Hit minimum value event
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnHitAttributeMinimum OnHitMinimum;

	// Hit maximum value event
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnHitAttributeMaximum OnHitMaximum;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Call this to set attribute data.
	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		void SetAttributeData(const FAttributeData& NewData);

	// Call this to set attribute data that matches our current AttributeData.Name. Returns true if matching attribute data was found and set.
	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		bool UpdateFromAttributeDataMap(const TMap<FName, FAttributeData>& DataMap);

	// Fills the given attribute map with an entry for our attribute data, uses AttributeData.Name as key.
	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		void FillAttributeDataMap(UPARAM(ref)TMap<FName, FAttributeData>& DataMap);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category = "ItemAttributes")
		FName GetAttributeName();

	UFUNCTION(BlueprintPure, Category = "ItemAttributes")
		float GetMin();
	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		void SetMin(const float NewMin);

	UFUNCTION(BlueprintPure, Category = "ItemAttributes")
		float GetMax();
	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		void SetMax(const float NewMax);

	UFUNCTION(BlueprintPure, Category = "ItemAttributes")
		float GetCurrent();
	// Sets current value clamped to min/max 
	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		void SetCurrent(const float NewValue);

	// Get remainin capacity of this attribute. (max value - current value)
	UFUNCTION(BlueprintPure, Category = "ItemAttributes")
		float GetRemainingCapacity();

	// Get the current percent of maximum value 0.0 - 1.0
	UFUNCTION(BlueprintPure, Category = "ItemAttributes")
		float GetCurrentPercent();

	// Add the given amount from the current value of this attribute. Value can be negative.
	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		bool DeltaValue(const float ToAdd, const bool bAllowOverspill);

	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		float ResetToMax();
};
