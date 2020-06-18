#pragma once


#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TrEnums.h"
#include "GridTemplateCell.generated.h"

UCLASS(BlueprintType)
class UGridTemplateCell : public UObject
{
	GENERATED_BODY()

public:

		UGridTemplateCell();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Y;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Group;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ETRGridCellState CellState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bOnPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bFlagged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FVector2D> ConnectedCells;
};