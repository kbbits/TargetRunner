#pragma once


#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ResourceRateFilter.h"
#include "TargetRunner.h"
#include "ExtractsResources.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UExtractsResources : public UInterface
{
	GENERATED_BODY()
};

/**		
 * Represents a tool (or weapon, etc.) that has the ability to extract resources from objects that implement the ExtractableResource interface.
 */
class TARGETRUNNER_API IExtractsResources
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.

public:
		
	// Returns the resource extraction rates for resource types relevant to this entity
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Extracts Resources")
		TArray<FResourceRateFilter> GetExtractionRates();

};