#pragma once

UENUM(BlueprintType)
enum class ETRDirection : uint8
{
	North	UMETA(DisplayName = "North"),
	East	UMETA(DisplayName = "East"),
	South	UMETA(DisplayName = "South"),
	West	UMETA(DisplayName = "West")
};

UENUM(BlueprintType)
enum class ETRAmmoType : uint8
{
	None 	UMETA(DisplayName = "None"),
	Goods 	UMETA(DisplayName = "Goods"),
	Bomb 	UMETA(DisplayName = "Bomb"),
	Sniper	UMETA(DisplayName = "Sniper")
};


UENUM(BlueprintType)
enum class ETRWeaponState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Firing		UMETA(DisplayName = "Firing"),
	FireBusy	UMETA(DisplayName = "FireBusy"),
	Reloading	UMETA(DisplayName = "Reloading"),
	OutOfAmmo	UMETA(DisplayName = "OutOfAmmo")
};

UENUM(BlueprintType)
enum class ETRWallState : uint8
{
	Unknown		UMETA(DisplayName = "Unknown"),
	Empty		UMETA(DisplayName = "Empty"),
	Blocked		UMETA(DisplayName = "Blocked"),
	Open		UMETA(DisplayName = "Open"),
	Door		UMETA(DisplayName = "Door"),
	Random		UMETA(DisplayName = "Random")
};

UENUM(BlueprintType)
enum class ETRFloorState : uint8
{
	Unknown		UMETA(DisplayName = "Unknown"),
	Open		UMETA(DisplayName = "Open"),
	Blocked		UMETA(DisplayName = "Blocked")
};

UENUM(BlueprintType)
enum class ETRGridCellState : uint8
{
	Blocked		UMETA(DisplayName = "Blocked"),
	Open		UMETA(DisplayName = "Open")
};