#pragma once

#include "Engine/DataTable.h"
#include "LevelTemplate.h"
#include "PlayerLevelRecord.h"
#include "LevelTemplateContextStruct.generated.h"

// Describes a playable level. Instances of these will be generated.
// The instances will be made available to players. This data is then used to generate the playable level map.
USTRUCT(BlueprintType)
struct FLevelTemplateContextStruct 
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FLevelTemplate LevelTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FPlayerLevelRecord> PlayerRecords;
};