#pragma once

#include "Engine/DataTable.h"
#include "DynSceneLayerItem.h"
#include "DynSceneLayer.generated.h"

USTRUCT(BlueprintType)
struct FDynSceneLayer : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float MinPicks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float MaxPicks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Meta = (Tooltip = "If True, this layer contains 'enemy' entities. (affects quantity appearing) Default = false"))
		bool EnemyLayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FDynSceneLayerItem> WeightedSceneryItems;

};