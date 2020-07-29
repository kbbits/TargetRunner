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

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, SaveGame)
		FGuid PlayerGuid;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, SaveGame)
		FName ProfileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float MaxTierCompleted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float TotalRunsPlayed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float TotalPlaytimeInRuns;
	
public:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void GetPlayerSaveData(FPlayerSaveData& SaveData);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void UpdateFromPlayerSaveData(const FPlayerSaveData& SaveData);
};
