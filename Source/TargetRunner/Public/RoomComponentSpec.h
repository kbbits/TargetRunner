#pragma once

#include "Engine/DataTable.h"
#include "PrefabricatorAsset.h"
#include "RoomComponentActor.h"
#include "TREnums.h"
#include "RoomExitInfo.h"
#include "RoomComponentSpec.generated.h"


/**
* Describes a room component (ex: a floor piece) that can be used.
* Intended to be used in data table as a row describing a room component available for use.
*/
USTRUCT(BlueprintType)
struct FRoomComponentSpec : public FTableRowBase
{
	GENERATED_BODY()

public:
	FRoomComponentSpec()
	{
		Type = ETRRoomComponentType::Floor;
		PickWeight = 1.f;
	}

public:

	// The RoomComponentActor to spawn for this component. It must be oriented to default (North) orientation.
	// Takes precedence over ComponentPrefab.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TSubclassOf<ARoomComponentActor> ComponentActor;

	// The prefag to spawn for this component. Prefab must be oriented to default (North) orientation.
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	//	TSoftObjectPtr<UPrefabricatorAssetInterface> ComponentPrefab;

	// The type of this component. Room, Ceiling, etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		ETRRoomComponentType Type;

	// The exit configurations this component type is valid for.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<ETRRoomExitLayout>	ValidExitLayouts;

	// The relative weight of this component when selecting amongst available components as a weighted list.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float PickWeight;

};
