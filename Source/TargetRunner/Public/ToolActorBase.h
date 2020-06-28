// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UsableTool.h"
#include "ResourceRateFilter.h"
#include "ToolActorBase.generated.h"

UCLASS()
class TARGETRUNNER_API AToolActorBase : public AActor, public IUsableTool
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AToolActorBase();

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		FName Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		TSubclassOf<ATRProjectileBase> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons")
		ETRWeaponState WeaponState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		float EnergyPerShot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		FGoodsQuantity AmmoPerShot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		float BaseDamage;

	// The effective damage rates of this tool against given resource types. Expressed as a percent of base damage (0.0 - 1.0).
	// Values > 1.0 are allowed. Calculated damage against a resource type not matching any filters will = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons", meta = (ExposeOnSpawn = "true"))
		TArray<FResourceRateFilter> BaseDamageRates;

	// The effective extraction rates of this tool against given resource types. Expressed as a percent (0.0 - 1.0).
	// Values > 1.0 are allowed. Calculated extracted amounts against a resource type not matching any filters will = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Weapons", meta = (ExposeOnSpawn = "true"))
		TArray<FResourceRateFilter> BaseResourceExtractionRates;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		TAssetPtr<USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		TAssetPtr<USoundBase> FireBusySound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
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
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
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
