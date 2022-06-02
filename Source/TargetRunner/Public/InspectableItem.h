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
 * Represents an entity that has a DisplayName and InspectInfo to be displayed to the player when they "inspect" the item.
 */
class TARGETRUNNER_API IInspectableItem
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.

public:

	// Called by the inspector when it begins inspecting
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inspectable Item")
		void OnBeginInspect();

	// Called by the inspector when it ends inspecting
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inspectable Item")
		void OnEndInspect();

	// Inspector calls to get the item's DisplayName
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inspectable Item")
		FText GetItemDisplayName();

	// Inspector calls to get the item's full inspect info
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inspectable Item")
		FInspectInfo GetInspectInfo();
};