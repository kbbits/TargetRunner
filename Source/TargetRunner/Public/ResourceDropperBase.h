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
UCLASS(BlueprintType, Blueprintable, Config = Game)
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

    // The percent +/- of resources, 0.0 - 1.0, that will be distributed amongst rooms.
    // ex: 0.2 = Each room will have up to 20% more or less of each resource (calculated per-resource type) than 
    // the original PerRoomResource counts. Total resources will remain the same. They're just mixed around a bit amongst the rooms.
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config)
        float RoomResourceVariance = 0.2f;

    // The minimum resources allowed to remain in a room when distributing resources. If the quantity of resources
    // is below this minimum, then all of the resources will be moved (instead of leaving a tiny quantity).
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config)
        float MinRemainingRoomResources = 10.0f;

    // Print debug info to logs
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
        bool bEnableClassDebugLog = false;

protected:

    // The total resources to distribute amongst room templates.
    // This is not set directly. It is set in the call to DistributeResources();
    //TArray<FResourceQuantity> TotalResources;


// ###  Functions ###

public:

    // Sublcasses override this to implement different logic for resource distriution.
    virtual void DistributeResources(UPARAM(ref) FRandomStream& RandStream, const TArray<FResourceQuantity>& TotalLevelResources, FRoomGridTemplate& TemplateGrid);

    // This will distribute one instance of each actor sub-class across the grid.
    // Default implementation distributes specials across the grid, attempting to place them away from the shortest start->finish path.
    // Sublcasses override this to implement different logic for special actor distriution.
    virtual void DistributeSpecials(UPARAM(ref) FRandomStream& RandStream, const TArray<TSubclassOf<AActor>>& SpecialActorClasses, FRoomGridTemplate& TemplateGrid);

    // Given a single PerRoomResource, generates an array of RoomResources each with the resource distribution varied by the RoomResourceVariance.
    virtual void GeneratePerRoomResources(UPARAM(ref) FRandomStream& RandStream, UPARAM(ref) FRoomGridTemplate& TemplateGrid, const TArray<FResourceQuantity>& PerRoomResourcesOrig, const TArray<FIntPoint>& RoomCoords, TMap<FIntPoint, TArray<FResourceQuantity>>& RoomResources);

};