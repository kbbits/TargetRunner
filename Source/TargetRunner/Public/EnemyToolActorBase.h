// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolActorBase.h"
#include "TREnemyCharacter.h"
#include "EnemyToolActorBase.generated.h"

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API AEnemyToolActorBase : public AToolActorBase
{
	GENERATED_BODY()	

public:

	AEnemyToolActorBase();

	// Tracks count of time between activations.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float FireDelaySum;

	// Tracks count of time between shots in a burst.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float AutoFireDelaySum;

	// Tracks count of activations (shots) during a burst.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float BurstFireCount;

	// If true projectile spawning will be triggered by AnimNotifyEvents calling AnimNotifyFire
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bSpawnProjectileByAnimNotify;

	// Has this actor had it's attribute values adjusted (scaled) by level?
	// This will be false before ScaleToLevel() is called and true afterwards.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
		bool bScaleUpApplied;

public:

	// Get the TREnemyCharacater this weapon is attached to.
	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
		ATREnemyCharacter* GetAttachedCharacter();

	// [Call on Server]
	// Subclasses should override with meaningful implemenation. Base class only sets bScaleUpApplied = true.
	// This should calculate and set the values of the attributes of this character based on ToLevel provided.
	// The default values of each attribute are scaled by level, then the new scaled values are set on the attribute(s).
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void ScaleToLevel(const int32 LevelTo);
};
