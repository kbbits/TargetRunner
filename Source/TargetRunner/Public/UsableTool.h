#pragma once


#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Sound/SoundBase.h"
#include "TargetRunner.h"
#include "GoodsQuantity.h"
#include "TRProjectileBase.h"
#include "UsableTool.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UUsableTool : public UInterface
{
	GENERATED_BODY()
};

/**
 * Represents a tool (or weapon) that is usable by the player.
 */
class TARGETRUNNER_API IUsableTool
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.

public:

	// The unique name (or code) of the tool
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
		FName GetName();

	// The display name of the weapon
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
		FText GetDisplayName();

	// The projectile that this weapon fires.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
		TSubclassOf<ATRProjectileBase> GetProjectileClass();

	// Get the current weapon state.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
		ETRWeaponState GetCurrentState();

	// Sets the new weapon state.  Returns the old state.
	//UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
	//	ETRWeaponState SetCurrentState(const ETRWeaponState NewState);

	// Begin Fire Action
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
		void BeginFire();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
		void EndFire();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
		float GetEnergyPerShot();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
		TArray<FGoodsQuantity> GetAmmoPerShot();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
		float GetDamagePerShot();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
		float GetActivationDelay();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
		TAssetPtr<USoundBase> GetFireSound();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
		TAssetPtr<USoundBase> GetFireBusySound();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
		TAssetPtr<USoundBase> GetReloadSound();
};