// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "GoodsQuantity.h"
#include "GoodsQuantityRange.h"
#include "GoodsDropTable.h"
#include "GoodsDropper.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class TARGETRUNNER_API UGoodsDropper : public UObject
{
	GENERATED_BODY()
	
public:

	// Add this DataTable of GoodsDropTables to our known list of goods drop table data.
	// The DataTable rows _must_ be GoodsDropTable structs.
	UFUNCTION(BlueprintCallable, Category = "Goods")
		bool AddDropTableDataToLibrary(UDataTable* GoodsDropTableData);

	UFUNCTION(BlueprintCallable, Category = "Goods")
		bool RemoveDropTableDataFromLibrary(UDataTable* GoodsDropTableData);

	UFUNCTION(BlueprintCallable, Category = "Goods")
		void ClearDropTableLibrary();
		
	// Evaluate this drop table and return all Goods droppped.
	UFUNCTION(BlueprintCallable, Category = "Goods")
		TArray<FGoodsQuantity> EvaluateGoodsDropTable(UPARAM(ref) FRandomStream& RandStream, const FGoodsDropTable& GoodsTable, const float Level = 0);

private:
	// Our collection of DataTables, each containing GoodsDropTables rows
	TArray<UDataTable*> DropTableLibrary;

	// Find the GoodsDropTable data in the DropTableLibrary that has the given name.
	const FGoodsDropTable* FindDropTableInLibrary(const FName DropTableName) const;

	// Evaluate this list of DropChances as a weighted list of items.
	// Returns the goods dropped by the single selected item from the weighted list.
	TArray<FGoodsQuantity> EvaluateGoodsDropChanceWeighted(UPARAM(ref)FRandomStream RandStream, const TArray<FGoodsDropChance>& DropChances, const float Level = 0);

	// Evanuate this single DropChance by a % chance according to it's Chance value: 0.0 - 1.0
	TArray<FGoodsQuantity> EvaluateGoodsDropChancePercent(UPARAM(ref)FRandomStream& RandStream, const FGoodsDropChance& DropChance, const float Level = 0);

	// Get a random drop of the goods from this GoodsDropChance.
	TArray<FGoodsQuantity> GoodsForDropChance(UPARAM(ref)FRandomStream& RandStream, const FGoodsDropChance& DropChance, const float Level = 0);

	// Determine the exact quantity within this GoodsQuantityRange.
	FGoodsQuantity EvaluateGoodsQuantityRange(UPARAM(ref)FRandomStream& RandStream, const FGoodsQuantityRange& QuantityRange);
};
