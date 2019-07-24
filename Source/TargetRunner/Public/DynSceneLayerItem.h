#pragma once

#include "Engine/DataTable.h"
#include "DynSceneItemTemplate.h"
#include "DynSceneLayerItem.generated.h"

USTRUCT(BlueprintType)
struct FDynSceneLayerItem : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName ItemTemplateName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float ListWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float QuantityMin;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float QuantityMax;

};