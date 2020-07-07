// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorAttributeComponent.h"
#include "..\Public\ActorAttributeComponent.h"
#include "UnrealNetwork.h"

// Sets default values for this component's properties
UActorAttributeComponent::UActorAttributeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


void UActorAttributeComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UActorAttributeComponent, AttributeData);
}


// Called when the game starts
void UActorAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

}


void UActorAttributeComponent::SetAttributeData(const FAttributeData& NewData)
{
	AttributeData = NewData;
}


bool UActorAttributeComponent::UpdateFromAttributeDataMap(const TMap<FName, FAttributeData>& DataMap)
{
	if (DataMap.Contains(AttributeData.Name))
	{
		SetAttributeData(DataMap[AttributeData.Name]);
		return true;
	}
	return false;
}


void UActorAttributeComponent::FillAttributeDataMap(TMap<FName, FAttributeData>& DataMap)
{
	if (!AttributeData.Name.IsNone())
	{
		DataMap.Add(AttributeData.Name, AttributeData);
	}
}


// Called every frame
void UActorAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Don't do anything when we're not active
	if (!IsActive()){ return; }
	// Don't do anything if recharge is paused
	if (bRechargePaused) { return; }

	float NewValue = AttributeData.CurrentValue;
	// If we recharge and we're not at one of the min/max
	if ((RechargeRate > 0.0f && AttributeData.CurrentValue < AttributeData.MaxValue) || (RechargeRate < 0.0f && AttributeData.CurrentValue > AttributeData.MinValue))
	{
		// Update the new value based on recharge
		NewValue = FMath::Clamp((RechargeRate * DeltaTime) + AttributeData.CurrentValue, AttributeData.MinValue, AttributeData.MaxValue);
	}
	// If it's a new value, set it.
	if (AttributeData.CurrentValue != NewValue) 
	{ 
		SetCurrent(NewValue);
	}
}

FName UActorAttributeComponent::GetAttributeName()
{
	return AttributeData.Name;
}

float UActorAttributeComponent::GetMin()
{
	return AttributeData.MinValue;
}

void UActorAttributeComponent::SetMin(const float NewMin)
{
	AttributeData.MinValue = NewMin;
}

float UActorAttributeComponent::GetMax()
{
	return AttributeData.MaxValue;
}

void UActorAttributeComponent::SetMax(const float NewMax)
{
	AttributeData.MaxValue = NewMax;
}

float UActorAttributeComponent::GetCurrent()
{
	return AttributeData.CurrentValue;
}

void UActorAttributeComponent::SetCurrent(const float NewValue)
{
	float NewValueClamped = FMath::Clamp(NewValue, AttributeData.MinValue, AttributeData.MaxValue);
	if (AttributeData.CurrentValue != NewValueClamped)
	{
		AttributeData.CurrentValue = NewValueClamped;
		OnDeltaCurrent.Broadcast(AttributeData.CurrentValue);
		if (AttributeData.CurrentValue == AttributeData.MaxValue)
		{
			OnHitMaximum.Broadcast();
		}
		else if (AttributeData.CurrentValue == AttributeData.MinValue)
		{
			OnHitMinimum.Broadcast();
		}
	}	
}

// Get remainin capacity of this attribute. (max value - current value)
float UActorAttributeComponent::GetRemainingCapacity()
{
	return AttributeData.MaxValue - AttributeData.CurrentValue;
}

// Get the current percent of maximum value 0.0 - 1.0
float UActorAttributeComponent::GetCurrentPercent()
{
	if (AttributeData.MaxValue == 0.0f)
	{
		return 0.0f;
	}	
	else 
	{
		return AttributeData.CurrentValue / AttributeData.MaxValue;
	}
}

// Add the given amount from the current value of this attribute. Value can be negative.
//		bAllowOverspill -	if true this will allow a delta adjustment that would push the value outside of 
//							the min/max range to succeed and just set the value to the clamped range.
//							If this is false a delta that would result in exceeding min/max range will return false and make no change to the current value.
// Returns true if successful, false if delta was not applied to value.
bool UActorAttributeComponent::DeltaValue(const float ToAdd = 1, const bool bAllowOverspill = false)
{
	float NewValue = AttributeData.CurrentValue + ToAdd;
	// If we are outside allowed range and we don't allow overspill, just return false
	if ((NewValue > AttributeData.MaxValue || NewValue < AttributeData.MinValue) && !bAllowOverspill)
	{
		return false;
	}
	// Set the current value to the new clamped value
	SetCurrent(NewValue);
	return true;
}

// Sets the current value to max value and returns this value.
float UActorAttributeComponent::ResetToMax()
{
	SetCurrent(AttributeData.MaxValue);
	return AttributeData.CurrentValue;
}

