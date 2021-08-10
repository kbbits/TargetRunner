// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "Components/ActorComponent.h"
#include "AttributeData.h"
#include "ModifiableAttributes.h"
#include "ActorAttributeComponent.generated.h"

// Event dispatcher for when CurrentValue changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeltaCurrent, float, NewCurrent);
// Event dispatcher for when we hit minimum value
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitAttributeMinimum);
// Event dispatcher for when we hit maximum value
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitAttributeMaximum);
// Event dispatcher for when min or max value has changed
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeltaMinMax);

// Each instance of an ActorAttributeComponent represents a single attribute for the actor. ex: Health, Energy, etc.
// This component provides support for applying modifiers to the attribute values.
// Changes made on server are replicated to owning client.
// Replication is handled via the replicated AttributeData and ModifiedAttributeData properties. 
// Notification delegates are called in the RepNotify handler.
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TARGETRUNNER_API UActorAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UActorAttributeComponent();

	// The base attribute data. Use accessors to get current values.
	// Modifying this directly skips notifications and clamping logic.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_AttributeDataChanged, SaveGame, Category = "ItemAttributes")
		FAttributeData AttributeData;

	// The current state of the attribute, including any mods applied. Current Value is pulled from here.
	// Use accessors to ensure correct getting/setting of values.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, ReplicatedUsing = OnRep_ModifiedAttributeDataChanged, Category = "ItemAttributes")
		FAttributeData ModifiedAttributeData;
			
	// If true, attribute recharge will be suspended.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemAttributes")
		bool bRechargePaused = false;

	// Even fired when the current value of the attribute has changed.
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnDeltaCurrent OnDeltaCurrent;
	
	// Hit minimum value event
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnHitAttributeMinimum OnHitMinimum;

	// Hit maximum value event
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnHitAttributeMaximum OnHitMaximum;

	// Changed min or max value
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnDeltaMinMax OnDeltaMinMax;

protected:

	// These are only tracked on the server and should be re-applied after construction.
	UPROPERTY(EditInstanceOnly, Replicated, Transient, Category = "ItemAttributes")
		TArray<FAttributeModifier> Modifiers;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void CalculateModifiedAttributeValues();

public:	

	void PostInitProperties() override;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Helper to copy properties from another component to this one. Used in PlayerState.CopyProperties and PlayerController.SeamlessTravelFrom
	void CopyPropertiesFromOther(UActorAttributeComponent* OtherComponent);

	// [Client]
	// Replication notification
	UFUNCTION()
		void OnRep_AttributeDataChanged(FAttributeData OldAttributeData);

	// [Client]
	// Replication notification
	UFUNCTION()
		void OnRep_ModifiedAttributeDataChanged(FAttributeData OldModifiedAttributeData);

	// [Server]
	UFUNCTION(BlueprintNativeEvent)
		void AddModifiers(const TArray<FAttributeModifier>& NewModifiers);

	// [Server]
	UFUNCTION(BlueprintNativeEvent)
		void RemoveModifiers(const TArray<FAttributeModifier>& NewModifiers);

	// Call this to set base attribute data. If called on server, data will replicate to client normally.
	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		void SetAttributeData(const FAttributeData& NewData);

	// Call this to set base attribute data that matches our current AttributeData.Name. Returns true if matching attribute data was found and set.
	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		bool UpdateFromAttributeDataMap(const TMap<FName, FAttributeData>& DataMap);

	// Call this to set base attribute data that matches our current AttributeData.Name. Returns true if matching attribute data was found and set.
	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		bool UpdateFromAttributeDataArray(const TArray<FAttributeData>& DataArray);

	// Fills the given attribute map with an entry for our base attribute data, uses AttributeData.Name as key.
	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		void FillAttributeDataMap(UPARAM(ref)TMap<FName, FAttributeData>& DataMap);

	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		void FillAttributeDataArray(UPARAM(ref)TArray<FAttributeData>& DataArray);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category = "ItemAttributes")
		FName GetAttributeName();

	// Current minimum value including modifiers
	UFUNCTION(BlueprintPure, Category = "ItemAttributes")
		float GetMin();

	// Get the minimum value without including modifiers
	UFUNCTION(BlueprintPure, Category = "ItemAttributes")
		float GetMinBase();
	
	// Set the minimum value, before modifiers are calculated
	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		void SetMinBase(const float NewMin);

	// Current maximum value including modifiers
	UFUNCTION(BlueprintPure, Category = "ItemAttributes")
		float GetMax();

	// Get the maximum value without including modifiers
	UFUNCTION(BlueprintPure, Category = "ItemAttributes")
		float GetMaxBase();

	// Set the maximum value, before modifiers are calculated
	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		void SetMaxBase(const float NewMax);

	// Get the current value including modifiers
	UFUNCTION(BlueprintPure, Category = "ItemAttributes")
		float GetCurrent();

	// Sets current value clamped to min/max of modified attribute data 
	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		void SetCurrent(const float NewValue);

	// Sets current value of the base attribute clamped to min/max of base attribute data.
	// Should never need to call this, except for data loading or init.
	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		void SetCurrentBase(const float NewValue);

	UFUNCTION(BlueprintPure, Category = "ItemAttributes")
		float GetDeltaRate();
	
	// Get remainin capacity of this attribute. (max value - current value)
	UFUNCTION(BlueprintPure, Category = "ItemAttributes")
		float GetRemainingCapacity();

	// Get the current percent of maximum value 0.0 - 1.0
	UFUNCTION(BlueprintPure, Category = "ItemAttributes")
		float GetCurrentPercent();

	// Add the given amount to the current value of this attribute. Value can be negative.
	//		bAllowOverspill -	if true this will allow a delta adjustment that would push the value outside of 
	//							the min/max range to succeed and will set the value to the clamped range.
	//							If this is false a delta that would result in exceeding min/max range will return false and make no change to the current value.
	// Returns true if successful and current value was changed, false if current value was not changed.
	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		bool DeltaValue(const float ToAdd, const bool bAllowOverspill);

	UFUNCTION(BlueprintCallable, Category = "ItemAttributes")
		float ResetToMax();
};
