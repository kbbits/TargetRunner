#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GridForgeBase.h"
#include "RoomGridTemplate.h"
#include "GridForgeManual.generated.h"

/**
 *
 */
UCLASS()
class TARGETRUNNER_API UGridForgeManual : public UGridForgeBase
{
    GENERATED_BODY()

public:

    virtual void GenerateGridTemplate(UPARAM(ref) FRandomStream& RandStream, FRoomGridTemplate& RoomGridTemplate, bool& Successful) override;

protected:

};