// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorAttributeComponent.h"
#include "TargetRunner.h"
#include "NamedPrimitiveTypes.h"
#include "UnrealNetwork.h"
// for intellisense
#include "..\Public\ActorAttributeComponent.h"

// Sets default values for this component's properties
UActorAttributeComponent::UActorAttributeComponent()
{
	ModifiedAttributeData = AttributeData;
	//SetIsReplicated(true);
	SetIsReplicatedByDefault(true);
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


void UActorAttributeComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UActorAttributeComponent, AttributeData);
	DOREPLIFETIME(UActorAttributeComponent, ModifiedAttributeData);
	DOREPLIFETIME(UActorAttributeComponent, Modifiers);
}


// Called when the game starts
void UActorAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	CalculateModifiedAttributeValues();
}

void UActorAttributeComponent::CalculateModifiedAttributeValues()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		FAttributeData MultiplierAttrData;
		FAttributeData ScalarAttrData;

		for (FAttributeModifier Mod : Modifiers)
		{
			switch (Mod.Modifier.Type)
			{
			case EModifierType::Multiplier:
				switch (Mod.EffectsValue)
				{
				case EAttributeModifierType::ValueMin:
					MultiplierAttrData.MinValue += Mod.Modifier.Value;
					break;
				case EAttributeModifierType::ValueMax:
					MultiplierAttrData.MaxValue += Mod.Modifier.Value;
					break;
				case EAttributeModifierType::ValueMaxAndCurrent:
					MultiplierAttrData.MaxValue += Mod.Modifier.Value;
					ResetToMax();
					break;
				case EAttributeModifierType::Rate:
					MultiplierAttrData.DeltaRate += Mod.Modifier.Value;
					break;
				}
				break;
			case EModifierType::Scalar:
				switch (Mod.EffectsValue)
				{
				case EAttributeModifierType::ValueMin:
					ScalarAttrData.MinValue += Mod.Modifier.Value;
					break;
				case EAttributeModifierType::ValueMax:
					ScalarAttrData.MaxValue += Mod.Modifier.Value;
					break;
				case EAttributeModifierType::ValueMaxAndCurrent:
					MultiplierAttrData.MaxValue += Mod.Modifier.Value;
					ResetToMax();
					break;
				case EAttributeModifierType::Rate:
					ScalarAttrData.DeltaRate += Mod.Modifier.Value;
					break;
				}
				break;
			}
		}
		ModifiedAttributeData.MinValue = ScalarAttrData.MinValue + (AttributeData.MinValue * (1.0 + MultiplierAttrData.MinValue));
		ModifiedAttributeData.MaxValue = ScalarAttrData.MaxValue + (AttributeData.MaxValue * (1.0 + MultiplierAttrData.MaxValue));
		ModifiedAttributeData.DeltaRate = ScalarAttrData.DeltaRate + (AttributeData.DeltaRate * (1.0 + MultiplierAttrData.DeltaRate));
		SetCurrent(ModifiedAttributeData.CurrentValue);
	}
}


void UActorAttributeComponent::PostInitProperties()
{
	Super::PostInitProperties();
	ModifiedAttributeData.CurrentValue = AttributeData.CurrentValue;
	CalculateModifiedAttributeValues();
}


#if WITH_EDITOR
void UActorAttributeComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = PropertyChangedEvent.MemberProperty == nullptr ? PropertyChangedEvent.GetPropertyName() : PropertyChangedEvent.MemberProperty->GetFName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UActorAttributeComponent, AttributeData))
	{
		ModifiedAttributeData.CurrentValue = AttributeData.CurrentValue;
		CalculateModifiedAttributeValues();
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif


void UActorAttributeComponent::CopyPropertiesFromOther(UActorAttributeComponent* OtherComponent)
{
	if (OtherComponent)
	{
		// First, set the modifiers
		Modifiers.Empty(OtherComponent->Modifiers.Num());
		Modifiers.Append(OtherComponent->Modifiers);
		// Call SetAttributeData to set values. It will recalculate modified values which will replicate.
		SetAttributeData(OtherComponent->AttributeData);
	}
}


void UActorAttributeComponent::OnRep_AttributeDataChanged(FAttributeData OldAttributeData)
{
	/*
	if (OldAttributeData.MinValue != AttributeData.MinValue || OldAttributeData.MaxValue != AttributeData.MaxValue)
	{
		OnDeltaMinMax.Broadcast();
	}
	if (OldAttributeData.CurrentValue != AttributeData.CurrentValue) 
	{ 
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
	*/
}


void UActorAttributeComponent::OnRep_ModifiedAttributeDataChanged(FAttributeData OldModifiedAttributeData)
{
	if (OldModifiedAttributeData.MinValue != ModifiedAttributeData.MinValue || OldModifiedAttributeData.MaxValue != ModifiedAttributeData.MaxValue)
	{
		OnDeltaMinMax.Broadcast();
	}
	if (OldModifiedAttributeData.CurrentValue != ModifiedAttributeData.CurrentValue)
	{
		OnDeltaCurrent.Broadcast(GetCurrent());
		if (GetCurrent() == GetMax())
		{
			OnHitMaximum.Broadcast();
		}
		else if (GetCurrent() == GetMin())
		{
			OnHitMinimum.Broadcast();
		}
	}
}

void UActorAttributeComponent::AddModifiers_Implementation(const TArray<FAttributeModifier>& NewModifiers)
{
	bool bChanged = false;
	for (FAttributeModifier Mod : NewModifiers)
	{
		if (Mod.AttributeCode == GetAttributeName())
		{
			Modifiers.Add(Mod);
			bChanged = true;

			//UE_LOG(LogTRGame, Log, TEXT("AttributeModifier added: %s, %s %.1f"), *Mod.AttributeCode.ToString(), *GetEnumValueAsString<EAttributeModifierType>(Mod.EffectsValue), Mod.Modifier.Value);
		}
	}
	if (bChanged)
	{
		CalculateModifiedAttributeValues();
	}
}

//bool UActorAttributeComponent::AddModifiers_Validate(const TArray<FAttributeModifier>& NewModifiers)
//{
//	return true;
//}


void UActorAttributeComponent::RemoveModifiers_Implementation(const TArray<FAttributeModifier>& NewModifiers)
{
	bool bChanged = false;
	for (FAttributeModifier Mod : NewModifiers)
	{
		if (Mod.AttributeCode == GetAttributeName())
		{
			if (Modifiers.RemoveSingle(Mod) > 0)
			{
				bChanged = true;
				//UE_LOG(LogTRGame, Log, TEXT("AttributeModifier Removed: %s, %s %.1f"), *Mod.AttributeCode.ToString(), *GetEnumValueAsString<EAttributeModifierType>(Mod.EffectsValue), Mod.Modifier.Value);
			}
		}
	}
	if (bChanged)
	{
		CalculateModifiedAttributeValues();
	}
}

//bool UActorAttributeComponent::RemoveModifiers_Validate(const TArray<FAttributeModifier>& NewModifiers)
//{
//	return true;
//}

void UActorAttributeComponent::SetAttributeData(const FAttributeData& NewData)
{
	AttributeData = NewData;
	ModifiedAttributeData = AttributeData;
	CalculateModifiedAttributeValues();
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


bool UActorAttributeComponent::UpdateFromAttributeDataArray(const TArray<FAttributeData>& DataArray)
{
	const FAttributeData* TmpAttrData = FindInNamedArray(DataArray, AttributeData.Name);
	if (TmpAttrData)
	{
		SetAttributeData(*TmpAttrData);
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

void UActorAttributeComponent::FillAttributeDataArray(TArray<FAttributeData>& DataArray)
{
	if (!AttributeData.Name.IsNone())
	{
		DataArray.Add(AttributeData);
	}
}


// Called every frame
void UActorAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

		// Don't do anything when we're not active
		if (!IsActive()) { return; }
		// Don't do anything if recharge is paused
		if (bRechargePaused || GetDeltaRate() == 0.0f) { return; }

		float NewValue = ModifiedAttributeData.CurrentValue;
		// If we recharge and we're not at one of the min/max
		if ((GetDeltaRate() > 0.0f && GetCurrent() < GetMax()) || (GetDeltaRate() < 0.0f && GetCurrent() > GetMin()))
		{
			// Update the new value based on recharge
			NewValue = FMath::Clamp((GetDeltaRate() * DeltaTime) + GetCurrent(), GetMin(), GetMax());
		}
		// If it's a new value, set it.
		if (GetCurrent() != NewValue)
		{
			SetCurrent(NewValue);
		}
	}
}

FName UActorAttributeComponent::GetAttributeName()
{
	return AttributeData.Name;
}

float UActorAttributeComponent::GetMin()
{
	return ModifiedAttributeData.MinValue;
}

float UActorAttributeComponent::GetMinBase() 
{ 
	return AttributeData.MinValue;
}

void UActorAttributeComponent::SetMinBase(const float NewMin)
{
	float OldMin = AttributeData.MinValue;
	AttributeData.MinValue = NewMin;
	if (NewMin != OldMin) 
	{ 
		CalculateModifiedAttributeValues();
		OnDeltaMinMax.Broadcast(); 
	}
}

float UActorAttributeComponent::GetMax()
{
	return ModifiedAttributeData.MaxValue;
}

float UActorAttributeComponent::GetMaxBase()
{
	return AttributeData.MaxValue;
}

void UActorAttributeComponent::SetMaxBase(const float NewMax)
{
	float OldMax = AttributeData.MaxValue;
	AttributeData.MaxValue = NewMax;
	if (NewMax != OldMax) 
	{ 
		CalculateModifiedAttributeValues();
		OnDeltaMinMax.Broadcast(); 
	}
}

float UActorAttributeComponent::GetCurrent()
{
	return ModifiedAttributeData.CurrentValue;
}

void UActorAttributeComponent::SetCurrent(const float NewValue)
{
	float NewValueClamped = FMath::Clamp(NewValue, ModifiedAttributeData.MinValue, ModifiedAttributeData.MaxValue);
	if (ModifiedAttributeData.CurrentValue != NewValueClamped)
	{
		ModifiedAttributeData.CurrentValue = NewValueClamped;
		OnDeltaCurrent.Broadcast(ModifiedAttributeData.CurrentValue);
		if (ModifiedAttributeData.CurrentValue == ModifiedAttributeData.MaxValue)
		{
			OnHitMaximum.Broadcast();
		}
		else if (ModifiedAttributeData.CurrentValue == ModifiedAttributeData.MinValue)
		{
			OnHitMinimum.Broadcast();
		}
	}	
}

void UActorAttributeComponent::SetCurrentBase(const float NewValue)
{
	float NewValueClamped = FMath::Clamp(NewValue, AttributeData.MinValue, AttributeData.MaxValue);
	if (AttributeData.CurrentValue != NewValueClamped)
	{
		AttributeData.CurrentValue = NewValueClamped;
		ModifiedAttributeData.CurrentValue = AttributeData.CurrentValue;
		CalculateModifiedAttributeValues();
		/*
		OnDeltaCurrent.Broadcast(AttributeData.CurrentValue);
		if (AttributeData.CurrentValue == AttributeData.MaxValue)
		{
			OnHitMaximum.Broadcast();
		}
		else if (AttributeData.CurrentValue == AttributeData.MinValue)
		{
			OnHitMinimum.Broadcast();
		}
		*/
	}
}

float UActorAttributeComponent::GetDeltaRate()
{
	return ModifiedAttributeData.DeltaRate;
}

// Get remainin capacity of this attribute. (max value - current value)
float UActorAttributeComponent::GetRemainingCapacity()
{
	return GetMax() - GetCurrent();
}

// Get the current percent of maximum value 0.0 - 1.0
float UActorAttributeComponent::GetCurrentPercent()
{
	if (GetMax() == 0.0f)
	{
		return 0.0f;
	}	
	else 
	{
		return GetCurrent() / GetMax();
	}
}


bool UActorAttributeComponent::DeltaValue(const float ToAdd = 1, const bool bAllowOverspill = false)
{
	float NewValue = GetCurrent() + ToAdd;
	// If we are outside allowed range and we don't allow overspill, just return false
	if (!bAllowOverspill && (NewValue > GetMax() || NewValue < GetMin()))
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
	SetCurrent(GetMax());
	return GetCurrent();
}

