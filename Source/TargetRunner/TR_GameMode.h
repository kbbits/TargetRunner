// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LevelTemplate.h"
#include "LevelForgeBase.h"
#include "ResourceDropperBase.h"
#include "GoodsDropper.h"
#include "TREnemyCharacter.h"
#include "PlatformGridMgr.h"
#include "ToolBase.h"
#include "ToolActorBase.h"
#include "TR_GameMode.generated.h"

/**
 * Base GameMode for working with Grid Manager etc.
 * Typical flow once GameMode is constructed:
 *		Get the new LevelTemplate and call SetNewLevelTemplate(), this will initialize all rand streams for the new level seed.
 *      If grid manager does not already exist in the world, spwan or load sublevel that contains the grid manager.
 *		Call InitGridManager() to find and initialize the grid manager.
 *      Call GenerateGrid() on the grid manager.
 *		Call SpawnLevel() to spawn the generated level in the world.
 */
UCLASS()
class TARGETRUNNER_API ATR_GameMode : public AGameMode
{
	GENERATED_BODY()

public:
	// Constructor for defaults
	ATR_GameMode(const FObjectInitializer& OI);

	// Size of each grid cell in world units.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float GridCellSize;

	// The number of subdivisions along each X & Y axis that each cell is divided.
	// Creating RoomCellSubdivision x RoomCellSubdivision total subcells in each cell.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 RoomCellSubdivision;

	// The GridForge class to use if LevelTemplat is set to "Default".
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UGridForgeBase> DefaultGridForgeClass;

	// The LevelForge class to use.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<ULevelForgeBase> DefaultLevelForgeClass;

	// The ResourceDropper class to use.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UResourceDropperBase> DefaultResourceDropperClass;

	// Until we implement LevelTemplate->Theme stuff, this is the room class that will be used.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<ARoomPlatformBase> DefaultRoomClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<FName, TSubclassOf<AToolActorBase>> ToolClassMap;

	// The GoodsDropper instance for this game mode.
	// Currently only used in older platform based game mode.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UGoodsDropper* GoodsDropper;

	// Clutter goods drops, each row is a drop table and has name in format Clutter<level tier>. ex: Clutter01
	// Can also contain other rows (drop tables) referenced by these ClutterXx rows.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UDataTable* GoodsDropperTable;

	// Clutter resource drops, each row is a drop table and has name in format ClutterResources<level tier>. ex: ClutterResources01
	// Can also contain other rows (drop tables) referenced by these ClutterResourcesXx rows.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UDataTable* ResourceDropTable;

	// Rows are FEnemyCharacterClassSet.
	// Row names must be set to the Tier the row represents.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UDataTable* EnemyMobsByTierTable;

	// Room components data to use. Describes floors and ceiling room actors to use when spawning rooms.
	// Using different data tables allows, for example, different themes.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UDataTable* RoomComponentsTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bEnableClassDebug = false;

protected:

	// Indicates if the room map generation has started.
	// This will remain true until
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		bool bRoomMapGenerateStarted;

	// Indicates that the LevelTemplate has been set and isready to be used.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
		bool bLevelTemplateReady;

	// Indicates that the RoomGridTemplate has been set and is ready to be used.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		bool bRoomGridTemplateReady;

	// Indicates that the level map of rooms has been spawned.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		bool bRoomMapReady;

	// Clutter goods drops quantities multiplied by (DifficultyLevel ^ ClutterGoodsLevelScaleExp)
	// Default is 0.25
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float ClutterGoodsLevelScaleExp;

	// Clutter resource drops quantities multiplied by (DifficultyLevel ^ ClutterResourceLevelScaleExp)
	// Default is 0.25
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float ClutterResourceLevelScaleExp;
	
private:

	UPROPERTY(BlueprintGetter=GetLevelTemplate)
		FLevelTemplate LevelTemplate;

	// The "parent" random stream - used to seed other streams.
	// Seed this one for each new level. It will re-seed other streams as appropriate
	UPROPERTY(BlueprintGetter=GetGeneratorStream)
		FRandomStream GeneratorRandStream;

	// Used to generate the map grid. Note helper functions: FRandRangeGrid and RandRangeGrid
	UPROPERTY(BlueprintGetter=GetGridStream)
		FRandomStream GridRandStream;

	// Used to generate the resource distribution. 
	UPROPERTY(BlueprintGetter = GetResourceDropperStream)
		FRandomStream ResourceDropperRandStream;

protected:

	int32 TotalPlayerControllersForTravel;

private:

	// Our Grid Manager. Set by InitGridManager.
	APlatformGridMgr* GridManager;

protected:

	// Called when the game starts or when spawned
	// This will attempt to grab the level template from the game instance.
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

	virtual void HandleMatchIsWaitingToStart() override;

	// Spawn a new grid manager at the given transform.
	// If there is already an existing grid manager, this will just return the existing one.
	//UFUNCTION(BlueprintCallable)
	//	APlatformGridMgr* SpawnNewGridManager(const TSubclassOf<APlatformGridMgr> GridManagerClass, const FTransform Transform);

public:
	/*---------- Player save/load --------------------*/

	// Saves player data for all connected PlayerControllers.
	// Saving is managed through each PlayerController's PersistentDataComponent.
	UFUNCTION(BlueprintCallable)
		void SaveAllPlayerData();

	// Loads player data for all PCs
	// Loading is managed through each PC's PDC.
	UFUNCTION(BlueprintCallable)
		void ReloadAllPlayerData();


	/*---------- Level Template and Grid -------------*/

	// Set and initialize the level template. Also seeds rand streams.
	UFUNCTION(BlueprintCallable)
		void SetNewLevelTemplate(const FLevelTemplate& NewTemplate);

	UFUNCTION(BlueprintPure)
		FLevelTemplate GetLevelTemplate();

	// Creates the goods dropper if it does not exist. Then adds the GoodsDropTable and ResoureDropTable data tables 
	// to the GoodsDropper and applies all initial settings.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void InitGoodsDropper();

	// Evaluates the drops for clutter shootables.
	// Returns list of goods to drop. 
	// GoodsDropTable names are in format: Clutter<level tier> ex: Clutter01 
	UFUNCTION(BlueprintCallable)
		void GetClutterDropGoods(TArray<FGoodsQuantity>& DroppedGoods);

	// Evaluates the resource drops for clutter shootables.
	// Returns list of resources to drop.
	// Clutter resource drops are implemented through goods drop tables with the resource code as the goods name. And translated to resource quantities.
	// Clutter resource drop table names (i.e. rows in a GoodsDropper table) are in the format: ClutterResources<level tier> ex: ClutterResources01
	UFUNCTION(BlueprintCallable)
		void GetClutterDropResources(TArray<FResourceQuantity>& DroppedResources);

	UFUNCTION(BlueprintCallable)
		void GetEnemyMobsForTier(const int32 Tier, TArray<TSubclassOf<ATREnemyCharacter>>& EnemyClasses);

	// Sets the current grid manager and initializes it from the level template. i.e. sets up grid extents, cell size, etc.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		bool InitGridManager();

	// Gets the current grid manager in the level.
	// Returns nullptr if no grid manager could be found.
	UFUNCTION(BlueprintPure)
		APlatformGridMgr* GetGridManager();

	UFUNCTION(BlueprintPure)
		void GetGridExtents(FVector2D& MinExtents, FVector2D& MaxExtents);

	// Called to spawn the platforms, of the level.
	// NOTE: current blueprint logic DOES NOT call this for RoomPlatforGridMgr (only basic PlatformGridMgr)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		bool SpawnLevel();

	/*------------ Random Streams -------------------------*/

	/** Re-seeds random streams, based on the incoming new parent seed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void ReseedAllStreams(const int32 NewSeed);

	UFUNCTION(BlueprintPure)
		FRandomStream& GetGeneratorStream();

	UFUNCTION(BlueprintPure)
		FRandomStream& GetGridStream();

	UFUNCTION(BlueprintPure)
		FRandomStream& GetResourceDropperStream();

	// Get a float in range from the Grid Stream.
	UFUNCTION(BlueprintCallable)
		float FRandRangeGrid(const float Min, const float Max);

	// Get an int in range from the Grid Stream.
	UFUNCTION(BlueprintCallable)
		int32 RandRangeGrid(const int32 Min, const int32 Max);

	/*--------------- Class Maps -----------------------*/
		
	/** Get the tool class by the given name. Currently this map of names -> classes is maintained in the ToolClassMap property. */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
		void ToolClassByName(const FName ToolName, TSubclassOf<AToolActorBase>& ToolClass, bool& bValid);
				
	UFUNCTION(BlueprintCallable)
		TSubclassOf<UGridForgeBase> GetGridForgeClass();

	UFUNCTION(BlueprintCallable)
		TSubclassOf<UResourceDropperBase> GetResourceDropperClass();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		TSubclassOf<ARoomPlatformBase> GetRoomClass();

	/** GameMode's seamless travel has completed. All player controllers may not have completed travel yet. */
	UFUNCTION(BlueprintNativeEvent)
		void OnGameModeSeamlessTravelComplete();

	/** All players have completed travel. */
	UFUNCTION(BlueprintNativeEvent)
		void OnAllPlayersTravelComplete();

public:

	/** Called when GameMode has completed seamless travel.
	*  We override to add a hook for GameMode level actions.
	*/
	virtual void PostSeamlessTravel() override;

	/**
	 * Handles all player initialization that is shared between the travel methods
	 * (i.e. called from both PostLogin() and HandleSeamlessTravelPlayer())
	 * This calls the BP version OnGenericPlayerInitialization.
	 */
	virtual void GenericPlayerInitialization(AController* C) override;

	/** Handle when player controllers change during seamless travel. */
	//virtual void SwapPlayerControllers(APlayerController* OldPC, APlayerController* NewPC) override;

protected:

	/** When each player is starting on the map level. */
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;


	/** Checks that there are no travelling players and that the room map has been spawned */
	virtual bool ReadyToStartMatch_Implementation() override;

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = Game, meta = (DisplayName = "OnGenericPlayerInitialization", ScriptName = "OnGenericPlayerInitialization"))
		void OnGenericPlayerInitialization(AController* C);

};
