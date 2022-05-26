#pragma once

#include "TRGameOptions.generated.h"

USTRUCT(BlueprintType)
struct FTRGameOptions
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float MasterVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float SFXVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float MusicVolume;
};