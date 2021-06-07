// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolActorBase.h"
#include "..\Public\ToolActorBase.h"
#include "ToolBase.h"
#include "ResourceFunctionLibrary.h"

// Sets default values
AToolActorBase::AToolActorBase()
{
	SetReplicates(true);
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponState = ETRWeaponState::Idle;
}


void AToolActorBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AToolActorBase, ToolData);
}


void AToolActorBase::OnRep_ToolData()
{
	if (ToolData.IsValid()) {
		Tool = UToolBase::CreateToolFromToolData(ToolData, this);
	}
}


// Called when the game starts or when spawned
void AToolActorBase::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void AToolActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AToolActorBase::InitToolObject()
{
	FToolData TmpToolData = ToolData;
	if (Tool == nullptr || !Tool->IsValidLowLevel())
	{		
		UToolBase* TmpTool = DefaultToolClass.GetDefaultObject();
		if (TmpTool)
		{
			TmpTool->ToToolData(TmpToolData);
			Tool = UToolBase::CreateToolFromToolData(TmpToolData, this);
		}
	}
	else
	{
		Tool->ToToolData(TmpToolData);
	}
	ToolData = TmpToolData;
}


FName AToolActorBase::GetName_Implementation()
{
	return Name;
}


FText AToolActorBase::GetDisplayName_Implementation()
{
	if (Tool != nullptr) { return Tool->DisplayName;  }
	return FText();
}


TSubclassOf<ATRProjectileBase> AToolActorBase::GetProjectileClass_Implementation()
{
	if (Tool != nullptr) { return Tool->ProjectileClass; }
	return ATRProjectileBase::StaticClass();
}


ETRWeaponState AToolActorBase::GetCurrentState_Implementation()
{
	return WeaponState;
}


// Base class does nothing
void AToolActorBase::BeginFire_Implementation()
{
	//if (Tool && Tool->bAllowsActivation && WeaponState == ETRWeaponState::Idle)
	//{
	//	WeaponState = ETRWeaponState::Firing;
	//}
}


// Base class does nothing
void AToolActorBase::EndFire_Implementation()
{
	//if (GetActivationDelay() <= 0.0f)
	//{
	//	WeaponState = ETRWeaponState::Idle;
	//}
	//else 
	//{
	//	if (WeaponState == ETRWeaponState::Firing)
	//	{
	//		WeaponState = ETRWeaponState::FireBusy;
	//	}
	//}
}


float AToolActorBase::GetEnergyPerShot_Implementation()
{
	if (Tool != nullptr) { return Tool->GetEnergyPerShot(); }
	return 0.0f;
}


TArray<FGoodsQuantity> AToolActorBase::GetAmmoPerShot_Implementation()
{
	if (Tool != nullptr) { return Tool->GetAmmoPerShot(); }
	return TArray<FGoodsQuantity>();
}


FGoodsQuantitySet AToolActorBase::GetAmmoPerShotAsSet_Implementation()
{
	if (Tool != nullptr) { return Tool->AmmoPerShot.GoodsQuantitySet; }
	return FGoodsQuantitySet();
}


float AToolActorBase::GetDamagePerShot_Implementation()
{
	if (Tool != nullptr) { return Tool->BaseDamage.CurrentValue; }
	return 0.0f;
}


float AToolActorBase::GetActivationDelay_Implementation()
{
	if (Tool != nullptr) { return Tool->ActivationDelay.CurrentValue; }
	return 0.0f;
}


TAssetPtr<USoundBase> AToolActorBase::GetFireSound_Implementation()
{
	return FireSound;
}


TAssetPtr<USoundBase> AToolActorBase::GetFireBusySound_Implementation()
{
	return FireBusySound;
}


TAssetPtr<USoundBase> AToolActorBase::GetReloadSound_Implementation()
{
	return ReloadSound;
}


TArray<FResourceRateFilter> AToolActorBase::GetDamageRates()
{
	if (Tool != nullptr) { return Tool->BaseDamageRates; }
	return TArray<FResourceRateFilter>();
}


TArray<FResourceRateFilter> AToolActorBase::GetResourceExtractionRates()
{
	if (Tool != nullptr) { return Tool->BaseResourceExtractionRates; }
	return TArray<FResourceRateFilter>();
}


