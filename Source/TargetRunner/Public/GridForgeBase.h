#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RoomGridTemplate.h"
#include "GridForgeBase.generated.h"

/**
 *
 */
UCLASS()
class TARGETRUNNER_API UGridForgeBase : public UObject
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable)
        void GenerateGridTemplate(FRoomGridTemplate& TemplateGrid, bool& Successful);

protected:

    // Returns the grid row. Creates the row if none existed.
    FRoomGridRow& GetGridRow(FRoomGridTemplate& TemplateGrid, int32 RowNum);
};