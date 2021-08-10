#pragma once

#include "Engine/DataTable.h"
#include "DynSceneryBase.h"
#include "DynSceneItemTemplate.generated.h"

/*
* Describes an available scenery item for use as a DynScenery actor.
*/
USTRUCT(BlueprintType)
struct FDynSceneItemTemplate : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
		FName Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
		TSubclassOf<ADynSceneryBase> SceneryClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TAssetPtr<UStaticMesh> MeshOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TSubclassOf<AActor> ActorOption;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FVector ExtentsMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FVector ExtentsMax;
	
};