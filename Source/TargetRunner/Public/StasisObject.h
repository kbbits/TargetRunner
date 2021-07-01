#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TREnums.h"
#include "StasisObject.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UStasisObject : public UInterface
{
	GENERATED_BODY()
};

/**
 * Represents an entity that can be put in to stasis (sleep) and awakened from stasis.
 */
class TARGETRUNNER_API IStasisObject
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.

public:

	// Get the current stasis state
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Stasis")
		ETRStasisState GetStasisState();

	// Set the item to In Stasis if it is not already InStasis.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Stasis")
		void StasisSleep();

	// Wake the item from statis if it is not already Awake.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Stasis")
		void StasisWake();
};