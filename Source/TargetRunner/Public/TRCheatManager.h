// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "TRCheatManager.generated.h"

/**
 * UNUSED
 */
UCLASS()
class TARGETRUNNER_API UTRCheatManager : public UCheatManager
{
	GENERATED_BODY()
	
		UFUNCTION(Exec)
		void AddAllKeys();
};
