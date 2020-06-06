#pragma once


#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GoodsQuantity.h"
#include "CollectableResource.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UCollectableResource : public UInterface
{
	GENERATED_BODY()
};

/**		
 * Represents an entity that contains resources that can be collected.
 */
class TARGETRUNNER_API ICollectableResource
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.

public:
			
	// Get the resource goods to collect.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Resource Collecting")
		void GetResourceGoods(TArray<FGoodsQuantity>& CollectedGoods);

	// Call this to notify this entity that it has been collected.
	// Generally this entity will then destroy itself.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Resource Collecting")
		void NotifyCollected();
};