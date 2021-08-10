// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "Engine/CompositeDataTable.h"
#include "GoodsQuantity.h"
#include "GoodsQuantityRange.h"
#include "GoodsDropTable.h"
#include "GoodsDropper.generated.h"

/**
 * Provides functionaly to evaluate groups of drop tables (FGoodsDropTable).
 * Each goods drop table is a collection of goods drop chances.
 * Note: when evaluated a goods drop chance in a goods drop table may reference other goods drop tables. This allows the designer to 
 * set up some goods drop tables as categories of goods and other goods drop tables that have chances of including results from those
 * categorized goods drop tables.
 */
UCLASS(BlueprintType, Blueprintable)
class TARGETRUNNER_API UGoodsDropper : public UObject
{
	GENERATED_BODY()
	
public:
	// Constructor
	UGoodsDropper();

public:

	// Add this DataTable of GoodsDropTables to our known list of goods drop table data.
	// The DataTable rows _must_ be GoodsDropTable structs.
	UFUNCTION(BlueprintCallable, Category = "Goods")
		bool AddDropTableDataToLibrary(UDataTable* GoodsDropTableData);

	UFUNCTION(BlueprintCallable, Category = "Goods")
		bool RemoveDropTableDataFromLibrary(UDataTable* GoodsDropTableData);

	UFUNCTION(BlueprintCallable, Category = "Goods")
		void ClearDropTableLibrary();

	UFUNCTION(BlueprintCallable, Category = "Goods")
		void SeedRandomStream(const int32 NewSeed);
		
	// Evaluate this drop table and return all Goods droppped.
	UFUNCTION(BlueprintCallable, Category = "Goods")
		TArray<FGoodsQuantity> EvaluateGoodsDropTable(const FGoodsDropTable& GoodsTable, const float QuantityScale = -1.0f);

	// Evaluate the named drop table and return all Goods droppped.
	UFUNCTION(BlueprintCallable, Category = "Goods")
		TArray<FGoodsQuantity> EvaluateGoodsDropTableByName(const FName& DropTableName, const float QuantityScale = -1.0f);

private:
	// Our random stream.  Use SeedRandomStream to set this if needed.
	FRandomStream RandStream;

	// Our collection of DataTables, each containing GoodsDropTables rows
	TArray<UDataTable*> DropTableLibrary;
	//UCompositeDataTable DropTableLibraryComposite;  // TODO use composite data table instead. Or, load all data from tables into object array...

	// Find the GoodsDropTable data in the DropTableLibrary that has the given name.
	const FGoodsDropTable* FindDropTableInLibrary(const FName DropTableName) const;

	// Evaluate this list of DropChances as a weighted list of items.
	// Returns the goods dropped by the single selected item from the weighted list.
	TArray<FGoodsQuantity> EvaluateGoodsDropChanceWeighted(const TArray<FGoodsDropChance>& DropChances, const float QuantityScale = -1.0f);

	// Evanuate this single DropChance by a % chance according to it's Chance value: 0.0 - 1.0
	TArray<FGoodsQuantity> EvaluateGoodsDropChancePercent(const FGoodsDropChance& DropChance, const float QuantityScale = -1.0f);

	// Get a random drop of the goods from this GoodsDropChance.
	TArray<FGoodsQuantity> GoodsForDropChance(const FGoodsDropChance& DropChance, const float QuantityScale = -1.0f);

};
