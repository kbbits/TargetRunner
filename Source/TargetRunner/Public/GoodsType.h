#pragma once

#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "GoodsType.generated.h"

USTRUCT(BlueprintType)
struct FGoodsType : public FTableRowBase
{
	GENERATED_BODY()

public:
	// Internal name of this GoodsType. Must be unique.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Name;

	// Name displayed to player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FText DisplayName;

	// Thumbnail for GUI use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TAssetPtr<UTexture2D> Thumbnail;

	// Optional actor class to spawn when placing into the world.
	// If null, this GoodsType cannot be placed in the world. (ex: add directly to player inventory instead)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TSoftClassPtr<AActor> ActorClass;

	// Various tags related to this GoodsType.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName GoodsTags;

	FORCEINLINE bool operator==(const FGoodsType& OtherType) const
	{
		if (Name != OtherType.Name) return false;
		return true;
	}

	FORCEINLINE bool operator==(const FGoodsType& OtherType)
	{
		if (Name != OtherType.Name) return false;
		return true;
	}

	FORCEINLINE bool operator==(FGoodsType& OtherType)
	{
		if (Name != OtherType.Name) return false;
		return true;
	}

	FORCEINLINE bool operator==(const FName& OtherName) const
	{
		if (Name != OtherName) return false;
		return true;
	}

	FORCEINLINE bool operator==(const FName& OtherName)
	{
		if (Name != OtherName) return false;
		return true;
	}

	FORCEINLINE bool operator==(FName& OtherName)
	{
		if (Name != OtherName) return false;
		return true;
	}
};