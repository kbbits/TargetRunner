#pragma once


#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TargetRunner.h"
#include "ResourceQuantity.h"
#include "ExtractableResource.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UExtractableResource : public UInterface
{
	GENERATED_BODY()
};

/**
 * Represents a resource node extractable by the player. ex: stone, copper, iron, etc.
 */
class TARGETRUNNER_API IExtractableResource
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.

public:
		
	// Gets the quanties of resources available in this entity in the form of:
	// An array of <FResourceQuantities> representing the current quantities of those resource types.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Extractable Resource")
		TArray<FResourceQuantity> GetResourceQuantities();

	// Gets the current quanty of a resource with the given type available in this entity.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Extractable Resource")
		float GetResourceQuantity(const FResourceType& ResourceType);
			
};