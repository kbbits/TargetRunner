#pragma once


#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LevelTemplate.h"
#include "PlayerLevelRecord.h"
#include "LevelTemplateContextStruct.h"
#include "LevelTemplateContext.generated.h"

UCLASS(BlueprintType)
class ULevelTemplateContext : public UObject
{
	GENERATED_BODY()

public:

	ULevelTemplateContext();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FLevelTemplate LevelTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<FGuid, FPlayerLevelRecord> PlayerRecords;

// ====== Functions

public:

	// Native code use
	FPlayerLevelRecord* PlayerRecord(const FGuid PlayerGuid);

	// Get the player record associated with the level template.
	UFUNCTION(BlueprintPure)
		void GetPlayerRecord(const FGuid PlayerGuid, bool& bFound, FPlayerLevelRecord& PlayerRecord);

	// Has this player guid unlocked this level template
	UFUNCTION(BlueprintPure)
		bool IsUnlockedForPlayer(const FGuid PlayerGuid);

	UFUNCTION(BlueprintCallable)
		FLevelTemplateContextStruct ToStruct();

	UFUNCTION(BlueprintCallable)
		static TArray<FLevelTemplateContextStruct> ToStructArray(const TArray<ULevelTemplateContext*> InArray);

	UFUNCTION(BlueprintCallable)
		static ULevelTemplateContext* LevelTemplateContextFromStruct(const FLevelTemplateContextStruct& InStruct, UObject* Outer);

	// Comparison overrides

	FORCEINLINE bool operator==(const ULevelTemplateContext& Other) const
	{
		if (LevelTemplate == Other.LevelTemplate) return true;
		return false;
	}

	FORCEINLINE bool operator==(const ULevelTemplateContext& Other)
	{
		if (LevelTemplate == Other.LevelTemplate) return true;
		return false;
	}

	FORCEINLINE bool operator==(ULevelTemplateContext& Other)
	{
		if (LevelTemplate == Other.LevelTemplate) return true;
		return false;
	}
};