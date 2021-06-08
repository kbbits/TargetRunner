// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ActorAttributeComponent.h"
#include "PlayerSaveData.h"
#include "TRPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API ATRPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ATRPlayerState();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		UActorAttributeComponent* EnergyAttribute;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		UActorAttributeComponent* AnimusAttribute;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		UActorAttributeComponent* HealthAttribute;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Replicated, SaveGame)
		FGuid PlayerGuid;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Replicated, SaveGame)
		FName ProfileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, SaveGame)
		FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, SaveGame)
		int32 ExperienceLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, SaveGame)
		FNamedGoodsQuantitySet LevelUpGoodsProgress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float MaxTierCompleted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float TotalRunsPlayed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float TotalPlaytimeInRuns;
		
public:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// For seamless travel property migration to new PlayerState
	virtual void CopyProperties(APlayerState* NewPlayerState) override;
	
	// This should typically be called through the TRPlayerController.ApplyAttributeModifiers(). That function handles replication.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void ApplyAttributeModifiers(const TArray<FAttributeModifier>& NewModifiers);

	// This should typically be called through the TRPlayerController.RemoveAttributeModifiers(). That function handles replication.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void RemoveAttributeModifiers(const TArray<FAttributeModifier>& ModifiersToRemove);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void GetPlayerSaveData(FPlayerSaveData& SaveData);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void UpdateFromPlayerSaveData(const FPlayerSaveData& SaveData);
};
