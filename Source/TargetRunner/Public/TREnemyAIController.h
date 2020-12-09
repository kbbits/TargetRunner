// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GenericTeamAgentInterface.h"
#include "Perception/AIPerceptionComponent.h"
#include "TargetRunner.h"
#include "TREnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API ATREnemyAIController : public AAIController
{
	GENERATED_BODY()

public:

	ATREnemyAIController();

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	//	UAIPerceptionComponent* AIPerception;

public:

	// [Server only]
	// Must be implemented by sublclasses.
	// Current base class implementation returns nullptr.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		AActor* GetCurrentTarget();

	// Implementation of IGenericTeamAgentInterface
	ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

	UFUNCTION(BlueprintPure)
		ETeamAttitude::Type GetTeamAttitudeTowardsBP(const AActor* Other) const;
};
