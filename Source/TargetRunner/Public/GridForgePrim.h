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

    virtual void GenerateGridTemplate(UPARAM(ref) FRandomStream& RandStream, FRoomGridTemplate& TemplateGrid, bool& bSuccessful) override;

    // Generates the underlying cell maze, which the room template is based on.
    virtual void GenerateGridTemplateCells(UPARAM(ref) FRandomStream& RandStream, const FRoomGridTemplate& RoomGridTemplate, bool& bSuccessful) override;

protected:

    FVector2D PickCoord(UPARAM(ref) FRandomStream& RandStream, const TArray<FVector2D>& CoordArray);

};