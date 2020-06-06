// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GoodsDropper.h"
#include "PlatformGridMgr.h"
#include "TRToolBase.h"
#include "TR_GameMode.generated.h"

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API ATR_GameMode : public AGameMode
{
	GENERATED_BODY()

public:
	// Constructor for defaults
	ATR_GameMode();

	// Size of each grid cell in world units.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float GridCellSize;

	// The difficulty rating for the current level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float LevelDifficulty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<FName, TSubclassOf<ATRToolBase>> ToolClassMap;

	// The "parent" random stream - used to seed other streams.
	// Seed this one for each new level. It will re-seed other streams as appropriate
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRandomStream GeneratorRandStream;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UGoodsDropper* GoodsDropper;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UDataTable* GoodsDropperTable;

protected:

	// Used to generate the map grid
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FRandomStream GridRandStream;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		bool InitGridManager(APlatformGridMgr* GridManager);

	UFUNCTION(BlueprintPure)
		void GetGridExtents(FVector2D& MinExtents, FVector2D& MaxExtents);

	// Get a float in range from the Grid Stream.
	UFUNCTION(BlueprintCallable)
		float FRandRangeGrid(const float Min, const float Max);

	// Get an int in range from the Grid Stream.
	UFUNCTION(BlueprintCallable)
		int32 RandRangeGrid(const int32 Min, const int32 Max);
		
	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
		void ToolClassByName(const FName ToolName, TSubclassOf<ATRToolBase>& ToolClass, bool& bValid);
				
};
