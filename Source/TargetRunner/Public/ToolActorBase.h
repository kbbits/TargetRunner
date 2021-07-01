// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ToolData.h"
#include "UsableTool.h"
#include "ResourceRateFilter.h"
#include "AttributeDataSet.h"
//#include "ToolBase.h"
#include "ToolActorBase.generated.h"

class UToolBase;

UCLASS()
class TARGETRUNNER_API AToolActorBase : public AActor, public IUsableTool
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AToolActorBase();

	// When this actor is spawned with no UTool object, this is the default to create.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<UToolBase> DefaultToolClass;

	UPROPERTY(ReplicatedUsing = OnRep_ToolData)
		FToolData ToolData;

	UPROPERTY(BlueprintReadOnly, Category = "Player Weapons", meta = (ExposeOnSpawn = "true"))
		class UToolBase* Tool;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_WeaponState, Category = "Player Weapons")
		ETRWeaponState WeaponState;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Weapons")
		FName Name;

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

	// [Client]
	// Replication notification
	UFUNCTION()
		void OnRep_ToolData();

	UFUNCTION(BlueprintNativeEvent)
		void OnRep_WeaponState();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// If the underlying UTool instance is null, this will create an instance of the DefualtToolClass.
	UFUNCTION(BlueprintCallable, Category = "Player Weapons")
		void InitToolObject();

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

	// Base class does nothing
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
		void BeginFire();
	virtual void BeginFire_Implementation();

	// Base class does nothing
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Weapons")
		void EndFire();
	virtual void EndFire_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
		float GetEnergyPerShot();
	virtual float GetEnergyPerShot_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
		TArray<FGoodsQuantity> GetAmmoPerShot();
	virtual TArray<FGoodsQuantity> GetAmmoPerShot_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
		FGoodsQuantitySet GetAmmoPerShotAsSet();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
		float GetDamagePerShot();
	virtual float GetDamagePerShot_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
		float GetActivationDelay();
	virtual float GetActivationDelay_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
		TAssetPtr<USoundBase> GetFireSound();
	virtual TAssetPtr<USoundBase> GetFireSound_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
		TAssetPtr<USoundBase> GetFireBusySound();
	virtual TAssetPtr<USoundBase> GetFireBusySound_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Player Weapons")
		TAssetPtr<USoundBase> GetReloadSound();
	virtual TAssetPtr<USoundBase> GetReloadSound_Implementation();



	UFUNCTION(BlueprintPure, Category = "Player Weapons")
		TArray<FResourceRateFilter> GetDamageRates();

	UFUNCTION(BlueprintPure, Category = "Player Weapons")
		TArray<FResourceRateFilter> GetResourceExtractionRates();

};
