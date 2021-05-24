#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/Actor.h"
#include "TargetRunner.h"
#include "ResourceQuantity.h"
#include "RoomGridTemplate.h"
#include "ResourceDropperBase.generated.h"

/**
 *
 */
UCLASS(BlueprintType, Blueprintable, Config=Game)
class TARGETRUNNER_API UResourceDropperBase : public UObject
{
    GENERATED_BODY()

public:
    // Constructor for defaults
    UResourceDropperBase();
        
public:

    // The minimum percent of total level resources to reserve for the exit
    // Default = 0.15;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config)
        float MinReservedForExit;

    // The maximum percent of total level resources to reserve for the exit
    // Default = 0.25;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config)
        float MaxReservedForExit;

    // The minimum percent of all rooms that will have resources.
    // Default = 0.6;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config)
        float MinRoomPercent;

    // The maximum percent of all rooms that will have resources.
    // Default = 0.8;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config)
        float MaxRoomPercent;

protected:

    // The total resources to distribute amongst room templates.
    // This is not set directly. It is set in the call to DistributeResources();
    //TArray<FResourceQuantity> TotalResources;

    // The random stream we are using to generate the LevelTemplate and content.
    //FRandomStream ResourceStream;


// ###  Functions ###

public:
    
    // Sublcasses override this to implement different logic for resource distriution.
    virtual void DistributeResources(UPARAM(ref) FRandomStream& RandStream, const TArray<FResourceQuantity>& TotalLevelResources, FRoomGridTemplate& TemplateGrid);

    // This will distribute one instance of each actor sub-class across the grid.
    // Default implementation distributes specials across the grid, attempting to place them off of the shortest start->finish path.
    // Sublcasses override this to implement different logic for special actor distriution.
    virtual void DistributeSpecials(UPARAM(ref) FRandomStream& RandStream, const TArray<TSubclassOf<AActor>>& SpecialActorClasses, FRoomGridTemplate& TemplateGrid);

protected:

    // Debug
    #if WITH_EDITOR
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
        bool bEnableClassDebugLog = false;
    FORCEINLINE void DebugLog(const FString& LogString) { if (bEnableClassDebugLog) { UE_LOG(LogTRGame, Log, TEXT("%s"), *LogString); } };
    #else
    FORCEINLINE void DebugLog(const FString& LogString) { };
    #endif


};