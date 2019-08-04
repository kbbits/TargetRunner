#pragma once
#pragma once

#include "Engine/DataTable.h"
#include "PlatformBase.h"
#include "PlatformGridRow.generated.h"

USTRUCT(BlueprintType)
struct FPlatformGridRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	//	TArray<APlatformBase*> Platforms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TMap<int, APlatformBase*> RowPlatforms;
	
};