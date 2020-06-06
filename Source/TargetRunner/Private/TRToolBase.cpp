// Fill out your copyright notice in the Description page of Project Settings.


#include "TRToolBase.h"
#include "..\Public\TRToolBase.h"

// Sets default values
ATRToolBase::ATRToolBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATRToolBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATRToolBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FName ATRToolBase::GetName_Implementation()
{
	return Name;
}

FText ATRToolBase::GetDisplayName_Implementation()
{
	return DisplayName;
}

TSubclassOf<ATRProjectileBase> ATRToolBase::GetProjectileClass_Implementation()
{
	return ProjectileClass;
}

ETRWeaponState ATRToolBase::GetCurrentState_Implementation()
{
	return WeaponState;
}

void ATRToolBase::BeginFire_Implementation()
{
	// Base class does nothing but set state.
	WeaponState = ETRWeaponState::Firing;
}

void ATRToolBase::EndFire_Implementation()
{
	// Base class does nothing
	WeaponState = ETRWeaponState::Idle;
}

float ATRToolBase::GetEnergyPerShot_Implementation()
{
	return EnergyPerShot;
}

FGoodsQuantity ATRToolBase::GetAmmoPerShot_Implementation()
{
	return AmmoPerShot;
}

float ATRToolBase::GetDamagePerShot_Implementation()
{
	return Damage;
}

TAssetPtr<USoundBase> ATRToolBase::GetFireSound_Implementation()
{
	return FireSound;
}

TAssetPtr<USoundBase> ATRToolBase::GetFireBusySound_Implementation()
{
	return FireBusySound;
}

TAssetPtr<USoundBase> ATRToolBase::GetReloadSound_Implementation()
{
	return ReloadSound;
}


