#pragma once

//#include "Engine/DataTable.h"
#include "TREnemyCharacter.h"
#include "EnemyCharacterClassSet.generated.h"

USTRUCT(BlueprintType)
struct FEnemyCharacterClassSet : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<TSubclassOf<ATREnemyCharacter>> EnemyClasses;
};