// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UsableTool.h"
#include "TRToolBase.generated.h"

UCLASS()
class TARGETRUNNER_API ATRToolBase : public AActor, public IUsableTool
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATRToolBase();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons")
		FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons")
		FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons")
		TSubclassOf<ATRProjectileBase> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons")
		ETRWeaponState WeaponState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons")
		float EnergyPerShot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons")
		FGoodsQuantity AmmoPerShot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons")
		float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons")
		TAssetPtr<USoundBase> FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons")
		TAssetPtr<USoundBase> FireBusySound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons")
		TAssetPtr<USoundBase> ReloadSound;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// IUsableTool interface functions

	// The unique name (or code) of the tool
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
		FName GetName();
	virtual FName GetName_Implementation();

	// The display name of the weapon
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
		FText GetDisplayName();
	virtual FText GetDisplayName_Implementation();

	// The projectile that this weapon fires.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
		TSubclassOf<ATRProjectileBase> GetProjectileClass();
	virtual TSubclassOf<ATRProjectileBase> GetProjectileClass_Implementation();

	// Get the current weapon state.
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
		ETRWeaponState GetCurrentState();
	virtual ETRWeaponState GetCurrentState_Implementation();

	// Sets the new weapon state.  Returns the old state.
	//UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
    //ETRWeaponState SetCurrentState(const ETRWeaponState NewState);
	//virtual ETRWeaponState SetCurrentState_Implementation(const ETRWeaponState NewState);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
		void BeginFire();
	virtual void BeginFire_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
		void EndFire();
	virtual void EndFire_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
		float GetEnergyPerShot();
	virtual float GetEnergyPerShot_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
		FGoodsQuantity GetAmmoPerShot();
	virtual FGoodsQuantity GetAmmoPerShot_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
		float GetDamagePerShot();
	virtual float GetDamagePerShot_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
		TAssetPtr<USoundBase> GetFireSound();
	virtual TAssetPtr<USoundBase> GetFireSound_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
		TAssetPtr<USoundBase> GetFireBusySound();
	virtual TAssetPtr<USoundBase> GetFireBusySound_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
		TAssetPtr<USoundBase> GetReloadSound();
	virtual TAssetPtr<USoundBase> GetReloadSound_Implementation();

};
