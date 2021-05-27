#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/Pawn.h"
#include "DestructibleObject.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UDestructibleObject : public UInterface
{
	GENERATED_BODY()
};

/**
 * UNUSED 
 * Represents an entity that can be damaged and destroyed, usually to drop pickups.
 */
class TARGETRUNNER_API IDestructibleObject
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.

public:

	// Get the item's DisplayName
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Destructible Object")
		void OnDestructibleObjectDamaged(const APawn* Instigator, const FHitResult& Hit, const float Damage);
};