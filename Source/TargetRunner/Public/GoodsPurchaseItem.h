#pragma once

#include "GoodsQuantity.h"
#include "GoodsPurchaseItem.generated.h"

USTRUCT(BlueprintType)
struct FGoodsPurchaseItem : public FTableRowBase
{
	GENERATED_BODY()

public:

	// Internal name of the GoodsType. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 TierAvailable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, meta = (TitleProperty = "Name"))
		TArray<FGoodsQuantity> Cost;
};