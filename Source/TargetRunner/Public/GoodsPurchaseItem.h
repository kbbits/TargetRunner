#pragma once

#include "Engine/DataTable.h"
#include "GoodsQuantity.h"
#include "GoodsPurchaseItem.generated.h"

USTRUCT(BlueprintType)
struct FGoodsPurchaseItem : public FTableRowBase
{
	GENERATED_BODY()

public:

	// Internal name of the GoodsType that this purchase represents.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Name;

	// Goods become available in the market based on player's experience level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 LevelAvailable;

	// The base cost (in goods) of this item.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, meta = (TitleProperty = "Name"))
		TArray<FGoodsQuantity> Cost;
};