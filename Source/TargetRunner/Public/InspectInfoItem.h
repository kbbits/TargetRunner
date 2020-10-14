#pragma once

#include "InspectInfoItem.generated.h"

USTRUCT(BlueprintType)
struct FInspectInfoItem
{

	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspectable Item")
		FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspectable Item")
		FText Info;

public:

	FInspectInfoItem()
	{
		Label = FText();
		Info = FText();
	}

	FInspectInfoItem(const FText& NewLabel, const FText& NewInfo)
	{
		Label = NewLabel;
		Info = NewInfo;
	}
};