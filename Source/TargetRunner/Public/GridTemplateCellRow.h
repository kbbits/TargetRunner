#pragma once


#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TrEnums.h"
#include "GridTemplateCell.h"
#include "GridTemplateCellRow.generated.h"

UCLASS(BlueprintType)
class UGridTemplateCellRow : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<int32, UGridTemplateCell*> RowCells;
};