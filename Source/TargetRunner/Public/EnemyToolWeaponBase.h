#pragma once

#include "ToolWeaponBase.h"
#include "EnemyToolWeaponBase.generated.h"


UCLASS(BlueprintType, Blueprintable)
class TARGETRUNNER_API UEnemyToolWeaponBase : public UToolWeaponBase
{
	GENERATED_BODY()

public:

	UEnemyToolWeaponBase();

public:

	// Level this weapon will be scaled to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 Level;

	// Has this weapon been scaled up.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
		bool bScaleUpApplied;

public:

	// [Call on Server]
	// Subclasses should implement as needed. Base class only sets bScaleUpApplied=true.
	// This calculates and sets the values of the attributes of this object based on it's Level.
	// The default values of each attribute are scaled by level, then the new scaled values are set on the attribute(s).
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void ScaleToLevel();
};

