// Fill out your copyright notice in the Description page of Project Settings.


#include "TREnemyAIController.h"

ATREnemyAIController::ATREnemyAIController()
	: Super()
{
	SetGenericTeamId(FGenericTeamId(static_cast<uint8>(ETRFaction::Enemy)));
    SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception")));    
}


ETeamAttitude::Type ATREnemyAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
    if (const APawn* OtherPawn = Cast<APawn>(&Other)) 
    {
        if (const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController()))
        {
            return Super::GetTeamAttitudeTowards(*OtherPawn->GetController());
        }
    }
    return ETeamAttitude::Neutral;
}


ETeamAttitude::Type ATREnemyAIController::GetTeamAttitudeTowardsBP(const AActor* Other) const
{
    return GetTeamAttitudeTowards(*Other);
}

