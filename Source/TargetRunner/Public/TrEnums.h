#pragma once

UENUM(BlueprintType)		
enum class EAmmoType : uint8
{
	None 	UMETA(DisplayName = "None"),
	Bomb 	UMETA(DisplayName = "Bomb"),
	Sniper	UMETA(DisplayName = "Sniper")
};
