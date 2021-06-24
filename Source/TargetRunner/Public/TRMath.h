// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TargetRunner.h"
#include "TRMath.generated.h"

/**
 *
 */
UCLASS()
class TARGETRUNNER_API UTRMath : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintPure)
        static FVector2D DirectionToOffsetVector(const ETRDirection Direction);

    UFUNCTION(BlueprintPure)
        static FIntPoint DirectionToOffset(const ETRDirection Direction);

    UFUNCTION(BlueprintPure)
        static ETRDirection OppositeDirection(const ETRDirection Direction);
};