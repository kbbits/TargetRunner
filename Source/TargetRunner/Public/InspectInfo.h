#pragma once

#include "InspectInfoItem.h"
#include "InspectInfo.generated.h"

USTRUCT(BlueprintType)
struct FInspectInfo
{
	
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspectable Item")
		FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspectable Item")
		TArray<FInspectInfoItem> DetailInfo;
};