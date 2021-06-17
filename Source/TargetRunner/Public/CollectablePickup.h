#pragma once


#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PickupAwards.h"
#include "CollectablePickup.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UCollectablePickup : public UInterface
{
	GENERATED_BODY()
};

/**
 * Represents an entity that contains PickupAwards (goods/energy/animus) that can be collected.
 */
class TARGETRUNNER_API ICollectablePickup
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.

public:

	// Get the full PickupAwards to collect from this pickup.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickup Collecting")
		void GetPickupAwards(FPickupAwards& PickupAwards);

	// Get the goods portion of the PickupAwards to collect.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickup Collecting")
		void GetPickupGoods(TArray<FGoodsQuantity>& PickupGoods);

	// Call this to notify this entity that it has been collected.
	// Generally this entity will then destroy itself.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickup Collecting")
		void NotifyPickupCollected();
};