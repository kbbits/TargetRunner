#pragma once

#include "Engine/DataTable.h"
#include "GoodsQuantity.h"
#include "PickupAwards.generated.h"

USTRUCT(BlueprintType)
struct FPickupAwardsItem //: public FTableRowBase
{
	GENERATED_BODY()

public:

	// Goods to be collected
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FGoodsQuantity> PickupGoods;

	// Energy collected from this pickup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float PickupEnergy;

	// Animus collected from this pickup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float PickupAnimus;
};

USTRUCT(BlueprintType)
struct FPickupAwards : public FTableRowBase
{
	GENERATED_BODY()

public:

	// All the items in this pickup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FPickupAwardsItem> PickupItems;
};