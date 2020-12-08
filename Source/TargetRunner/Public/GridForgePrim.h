#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GridForgeBase.h"
#include "RoomGridTemplate.h"
#include "GridForgePrim.generated.h"

/**
 *
 */
UCLASS()
class TARGETRUNNER_API UGridForgePrim : public UGridForgeBase 
{
    GENERATED_BODY()

public:

    // Stop generating additional grid after a path to the exit has been found.
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
        bool bStopWhenExitFound = true;

    virtual void GenerateGridTemplate(UPARAM(ref) FRandomStream& RandStream, FRoomGridTemplate& TemplateGrid, bool& bSuccessful) override;

    // Generates the underlying cell maze, which the room template is based on.
    virtual void GenerateGridTemplateCells(UPARAM(ref) FRandomStream& RandStream, bool& bSuccessful) override;

protected:

    // Pick next coordinate to evaluate. Changing how this is done changes the nature of the produced grid.
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
        FVector2D PickCoord(UPARAM(ref) FRandomStream& RandStream, const TArray<FVector2D>& CoordArray);

};