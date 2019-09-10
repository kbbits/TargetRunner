#pragma once

UENUM(BlueprintType)		
enum class EAmmoType : uint8
{
	None 	UMETA(DisplayName = "None"),
	Bomb 	UMETA(DisplayName = "Bomb"),
	Sniper	UMETA(DisplayName = "Sniper")
};

UENUM(BlueprintType)
enum class ETRAmmoType : uint8
{
	AmmoType_None 	UMETA(DisplayName = "None"),
	AmmoType_Goods 	UMETA(DisplayName = "Goods"),
	AmmoType_Bomb 	UMETA(DisplayName = "Bomb"),
	AmmoType_Sniper	UMETA(DisplayName = "Sniper")
};


UENUM(BlueprintType)
enum class ETRWeaponState : uint8
{
	WeaponState_Idle		UMETA(DisplayName = "Idle"),
	WeaponState_Firing		UMETA(DisplayName = "Firing"),
	WeaponState_FireBusy	UMETA(DisplayName = "FireBusy"),
	WeaponState_Reloading	UMETA(DisplayName = "Reloading"),
	WeaponState_OutOfAmmo	UMETA(DisplayName = "OutOfAmmo")
};
