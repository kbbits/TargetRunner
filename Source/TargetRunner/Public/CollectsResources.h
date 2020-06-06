#pragma once


#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GoodsQuantity.h"
#include "CollectsResources.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UCollectsResources : public UInterface
{
	GENERATED_BODY()
};

/**		
 * Represents an entity that collects resources.
 */
class TARGETRUNNER_API ICollectsResources
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.

public:
		
	// Returns true if this resource collector has a homing target, false otherwise.
	// If it does have a homing target, the location is set in TargetLocation.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Resource Collecting")
		bool GetCollectionTargetLocation(FVector& TargetLocation);

	// Give this entity resource goods to collect.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Resource Collecting")
		void CollectResourceGoods(const TArray<FGoodsQuantity>& CollectedGoods);
};