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
		int32 DistanceToStart = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 DistanceToEnd = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 DistanceToShortestPath = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ETRGridCellState CellState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bOnPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bFlagged;

	// The parent cell in our tree, connecting to this one.
	// Currently, there will only be one parent in this array.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FVector2D> ConnectedFrom;

	// The "child" cells this one is connected to as a "parent".
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FVector2D> ConnectedCells;

public:

	// Get our X, Y coords as an FVector2D
	UFUNCTION(BlueprintCallable)
		const FVector2D GetCoords();
};