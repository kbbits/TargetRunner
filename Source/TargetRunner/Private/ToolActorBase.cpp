// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolActorBase.h"
#include "..\Public\ToolActorBase.h"
#include "ResourceFunctionLibrary.h"

// Sets default values
AToolActorBase::AToolActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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

void AToolActorBase::BeginFire_Implementation()
{
	// Base class does nothing but set state.
	WeaponState = ETRWeaponState::Firing;
}

void AToolActorBase::EndFire_Implementation()
{
	// Base class does nothing but set state.
	WeaponState = ETRWeaponState::Idle;
}

float AToolActorBase::GetEnergyPerShot_Implementation()
{
	if (Tool != nullptr) { return Tool->EnergyPerShot.CurrentValue; }
	return 0.0f;
}

FGoodsQuantity AToolActorBase::GetAmmoPerShot_Implementation()
{
	if (Tool != nullptr) { return Tool->AmmoPerShot; }
	return FGoodsQuantity();
}

float AToolActorBase::GetDamagePerShot_Implementation()
{
	if (Tool != nullptr) { return Tool->BaseDamage.CurrentValue; }
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


