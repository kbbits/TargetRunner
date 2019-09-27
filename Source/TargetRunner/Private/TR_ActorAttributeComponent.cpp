// Fill out your copyright notice in the Description page of Project Settings.


#include "TR_ActorAttributeComponent.h"

// Sets default values for this component's properties
UTR_ActorAttributeComponent::UTR_ActorAttributeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UTR_ActorAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

}


// Called every frame
void UTR_ActorAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Don't do anything when we're not active
	if (!IsActive()){ return; }

	float oldCurrentValue = CurrentValue;

	// If we recharge and we're not at one of the min/max
	if ((RechargeRate > 0.0f && CurrentValue < MaxValue) || (RechargeRate < 0.0f && CurrentValue > MinValue))
	{
		// Update the new value based on recharge
		CurrentValue = FMath::Clamp((RechargeRate * DeltaTime) + CurrentValue, MinValue, MaxValue);
	}

	// If the value hasn't changed just return, we have nothing else to do.
	// With this check here we don't need to use flags for our events.
	if (CurrentValue == oldCurrentValue) { return; }

	// Check max and fire events if we just hit max
	if (CurrentValue == MaxValue)
	{
		// Reset OnHitMin flag since we're not at min
		// if (bFiredOnMin) { bFiredOnMin = false; }
		// Call event if we haven't already
		//if (!bFiredOnMax)
		//{
		//	OnHitMaximum.Broadcast();
		//	bFiredOnMax = true;
		//}
		OnHitMaximum.Broadcast();
	}
	else if (CurrentValue == MinValue) // We are at minimum
	{
		// Reset OnHitMax flag since we're not at max
		//if (bFiredOnMax) { bFiredOnMax = false; }
		// Call event if we haven't already
		//if (!bFiredOnMin)
		//{
		//	OnHitMinimum.Broadcast();
		//	bFiredOnMin = true;
		//}
		OnHitMinimum.Broadcast();
	}
	//else // Not at min or max
	//{
	//	// Reset OnHitMin flag since we're not at min
	//	if (bFiredOnMin) { bFiredOnMin = false; }
	//	// Reset OnHitMax flag since we're not at max
	//	if (bFiredOnMax) { bFiredOnMax = false; }
	//}
}

// Get remainin capacity of this attribute. (max value - current value)
float UTR_ActorAttributeComponent::GetRemainingCapacity()
{
	return MaxValue - CurrentValue;
}

// Get the current percent of maximum value 0.0 - 1.0
float UTR_ActorAttributeComponent::GetCurrentPercent()
{
	if (MaxValue == 0.0f)
	{
		return 0.0f;
	}	
	else 
	{
		return CurrentValue / MaxValue;
	}
}

// Add the given amount from the current value of this attribute. Value can be negative.
//		bAllowOverspill -	if true this will allow a delta adjustment that would push the value outside of 
//							the min/max range to succeed and just set the value to the clamped range.
//							If this is false a delta that would result in exceeding min/max range will return false and make no change to the current value.
// Returns true if successful, false if delta was not applied to value.
// As a convenience, this provides the current value after deducting in the RemainingValue out param.
bool UTR_ActorAttributeComponent::DeltaValue(const float ToDeduct = 1, const bool bAllowOverspill = false)
{
	float NewValue = CurrentValue + ToDeduct;

	// If we are outside allowed range and we don't allow overspill, just return false
	if ((NewValue > MaxValue || NewValue < MinValue) && !bAllowOverspill)
	{
		return false;
	}

	// Set the current value to the new clamped value
	CurrentValue = FMath::Clamp(NewValue, MinValue, MaxValue);
	return true;
}

