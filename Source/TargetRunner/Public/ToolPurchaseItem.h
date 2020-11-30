#pragma once

#include "GoodsQuantity.h"
#include "ToolPurchaseItem.generated.h"

USTRUCT(BlueprintType)
struct FToolPurchaseItem : public FTableRowBase
{
	GENERATED_BODY()

public:

	// Tool class represented by this item purchase
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TSubclassOf<class UToolBase> ToolClass;

	// Tool becomes available in the market based on player's experience level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 LevelAvailable;

	// The tool should not be available in the market if the player already has InventoryLimit number of this tool in their tool inventory.
	// Value of <= 0 means no limit. Default = 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 InventoryLimit;

	// The base cost (in goods) of this item.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, meta = (TitleProperty = "Goods"))
		FGoodsQuantitySet Cost;
};