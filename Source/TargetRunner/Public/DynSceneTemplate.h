#pragma once


#include "Engine/DataTable.h"
#include "DynSceneLayer.h"
#include "DynSceneTemplate.generated.h"

USTRUCT(BlueprintType)
struct FDynSceneTemplate : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FDynSceneLayer> Layers;

};