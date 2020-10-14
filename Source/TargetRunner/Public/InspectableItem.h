#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InspectInfo.h"
#include "InspectableItem.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UInspectableItem : public UInterface
{
	GENERATED_BODY()
};

/**
 * Represents an entity that has a DisplayName and InspectInfo.
 */
class TARGETRUNNER_API IInspectableItem
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.

public:

	// Get the item's DisplayName
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inspectable Item")
		FText GetItemDisplayName();

	// Gets the item's inspect info
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inspectable Item")
		FInspectInfo GetInspectInfo();
};