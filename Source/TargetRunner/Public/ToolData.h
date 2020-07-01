#pragma once

//#include "ToolBase.h"
#include "AttributeDataSet.h"
#include "ToolData.generated.h"

USTRUCT(BlueprintType)
struct FToolData
{
	GENERATED_BODY()

public:

	// Tool class this is based on
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TSubclassOf<class UToolBase> ToolClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FAttributeDataSet AttributeData;
};