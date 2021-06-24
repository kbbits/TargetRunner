#pragma once

template<typename TEnum>
static FORCEINLINE FString GetEnumValueAsString(/*const FString& Name,*/ TEnum Value) {
	//const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
	//if (!enumPtr) return FString("Invalid");
	//return enumPtr->GetNameByValue((int64)Value).ToString();
	static_assert(TIsEnum<TEnum>::Value, "Should only call this with enum types");
	UEnum* EnumClass = StaticEnum<TEnum>();
	check(EnumClass != nullptr);
	return EnumClass->GetNameByValue((int64)Value).ToString();;
}

UENUM(BlueprintType)
enum class ETRFaction : uint8
{
	None = 0			UMETA(DisplayName = "None"),
	Player = 1			UMETA(DisplayName = "Player"),
	Enemy = 8			UMETA(DisplayName = "Enemy"),
	AllNeutral = 255	UMETA(DisplayName = "AllNeutral")
};

UENUM(BlueprintType)
enum class ETRStasisState : uint8
{
	Awake		UMETA(DispalyName = "Awake"),
	InStasis	UMETA(DispalyName = "InStasis")
};

UENUM(BlueprintType)
enum class ETRDirection : uint8
{
	North		UMETA(DisplayName = "North"),
	NorthEast	UMETA(DispalyName = "NorthEast"),
	East		UMETA(DisplayName = "East"),
	SouthEast	UMETA(DispalyName = "SouthEast"),
	South		UMETA(DisplayName = "South"),
	SouthWest	UMETA(DispalyName = "SouthWest"),
	West		UMETA(DisplayName = "West"),
	NorthWest	UMETA(DispalyName = "NorthWest")
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
	Aiming		UMETA(DisplayName = "Aming"),
	ReadyToFire	UMETA(DisplayName = "ReadyToFire"),
	Firing		UMETA(DisplayName = "Firing"),
	FireBusy	UMETA(DisplayName = "FireBusy"),
	Reloading	UMETA(DisplayName = "Reloading"),
	OutOfAmmo	UMETA(DisplayName = "OutOfAmmo")
};

UENUM(BlueprintType)
enum class ETRToolUpgrade : uint8
{
	DamageRate		UMETA(DisplayName = "Damage Rate"),
	ExtractionRate	UMETA(DisplayName = "Extraction Rate")
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
enum class ETRRoomExitLayout : uint8
{
	// Room has no exits
	None		UMETA(DisplayName = "None"),
	// Room has one exit. At default orientation exit is on north wall.
	One			UMETA(DisplayName = "OneExit"),
	// Room has two exits on adjacent walls. At default orientation exits are on north and east walls.
	TwoAdjacent	UMETA(DisplayName = "TwoAdjacent"),
	// Room has two exits on opposite walls. At defalt orientation exits are on north and south walls.
	TwoOpposite	UMETA(DisplayName = "TwoOpposite"),
	// Room has three exits. At default orientation exits are on west, north and east walls.
	Three		UMETA(DisplayName = "Three"),
	// Room has four exits -- one on each wall.
	Four		UMETA(DisplayName = "Four")
};

UENUM(BlueprintType)
enum class ETRGridCellState : uint8
{
	Blocked		UMETA(DisplayName = "Blocked"),
	Open		UMETA(DisplayName = "Open")
};

UENUM(BlueprintType)
enum class ETRResourceMatch : uint8
{
	None = 0		UMETA(DispalyName = "None"),
	Category = 1	UMETA(DisplayName = "Category"),
	Type = 2		UMETA(DisplayName = "Type"),
	SubType	= 4		UMETA(DisplayName = "SubType"),
	Exact = 255		UMETA(DisplayName = "Exact Match")
};

//UENUM(BlueprintType)
//enum class ETRResourceCategory : uint8
//{
//	Mineral		UMETA(DisplayName = "Mineral"),
//	Metal		UMETA(DisplayName = "Metal"),
//	Vegetable	UMETA(DisplayName = "Vegetable"),
//	Energy		UMETA(DisplayName = "Energy")
//};