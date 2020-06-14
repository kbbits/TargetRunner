#pragma once

#include "Engine/DataTable.h"
#include "TextRow.generated.h"

USTRUCT(BlueprintType)
struct FTextRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FText Text;
};