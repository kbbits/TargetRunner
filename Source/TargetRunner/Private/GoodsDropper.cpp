// Fill out your copyright notice in the Description page of Project Settings.

#include "GoodsDropper.h"
#include "GoodsFunctionLibrary.h"

/*
*/
UGoodsDropper::UGoodsDropper()
	: Super()
{
	RandStream.GenerateNewSeed();
}

/*
*/
bool UGoodsDropper::AddDropTableDataToLibrary(UDataTable* GoodsDropTableData)
{
	if (!GoodsDropTableData->IsValidLowLevel())
	{
		UE_LOG(LogTRGame, Warning, TEXT("GoodsDropper AddDropTableToLibrary goods drop table is not valid"));
		return false;
	}
	TArray<FName> RowNames = GoodsDropTableData->GetRowNames();
	if (RowNames.Num() > 0)
	{
		// Check that this is a table of FGoodsDropTable rows.
		FGoodsDropTable* FirstTable = GoodsDropTableData->FindRow<FGoodsDropTable>(RowNames[0], "", false);
		if (FirstTable == nullptr) {
			return false;
		}
		//DropTableLibraryComposite.ParentTables.AddUnique(GoodsDropTableData);
		DropTableLibrary.AddUnique(GoodsDropTableData);
	}
	return true;
}

/*
*/
bool UGoodsDropper::RemoveDropTableDataFromLibrary(UDataTable * GoodsDropTableData)
{
	int32 NumRemoved = DropTableLibrary.Remove(GoodsDropTableData);
	return NumRemoved > 0;
}

/*
*/
void UGoodsDropper::ClearDropTableLibrary()
{
	DropTableLibrary.Empty();
}

/*
*/
void UGoodsDropper::SeedRandomStream(const int32 NewSeed)
{
	RandStream.Initialize(NewSeed);
}

/*
*/
TArray<FGoodsQuantity> UGoodsDropper::EvaluateGoodsDropTable(const FGoodsDropTable& GoodsTable, const float QuantityScale)
{
	TArray<FGoodsQuantity> AllGoods;
	int32 TotalPicks;

	if (GoodsTable.bAsWeightedList)
	{
		// Make a number of picks
		TotalPicks = RandStream.RandRange(GoodsTable.MinWeightedPicks, GoodsTable.MaxWeightedPicks);
		// Each pick is one item from the weighted list of items.
		for (int i = 1; i <= TotalPicks; i++) {
			AllGoods.Append(EvaluateGoodsDropChanceWeighted(GoodsTable.GoodsOddsList, QuantityScale));
		}
	}
	else
	{
		// Each entry has a percent chance to be included.
		for (FGoodsDropChance DropChance : GoodsTable.GoodsOddsList)
		{
			if (DropChance.Chance > 0.0f) // Don't bother evaluating if Chance is not > 0
			{
				AllGoods.Append(EvaluateGoodsDropChancePercent(DropChance, QuantityScale));
			}
		}
	}
	return AllGoods;
}

TArray<FGoodsQuantity> UGoodsDropper::EvaluateGoodsDropTableByName(const FName & DropTableName, const float QuantityScale)
{
	TArray<FGoodsQuantity> AllGoods;
	const FGoodsDropTable* FoundDropTable = FindDropTableInLibrary(DropTableName);

	if (FoundDropTable != nullptr) {
		AllGoods = EvaluateGoodsDropTable(*FoundDropTable, QuantityScale);
	}
	else
	{
		AllGoods = TArray<FGoodsQuantity>();
		// TODO: report missed drop table hit
	}
	return AllGoods;
}

/*
*/
const FGoodsDropTable* UGoodsDropper::FindDropTableInLibrary(const FName DropTableName) const
{
	FGoodsDropTable* FoundDropTable = nullptr;

	// Look in each data table until we find one containing a row with the given name. Then return that row.
	for (UDataTable* Table : DropTableLibrary)
	{
		FoundDropTable = Table->FindRow<FGoodsDropTable>(DropTableName, "", false);
		// If we found the table, stop looking through library
		if (FoundDropTable != nullptr) { 
			break; 
		}
	}
	return FoundDropTable;
}

/*
*/
TArray<FGoodsQuantity> UGoodsDropper::EvaluateGoodsDropChanceWeighted(const TArray<FGoodsDropChance>& DropChances, const float QuantityScale)
{
	TArray<FGoodsQuantity> AllGoods;
	float TotalWeight = 0.0f;
	float AccumWeight = 0.0f;
	float Pick = 0.0f;

	for (const FGoodsDropChance& DropChance : DropChances) {
		TotalWeight += FMath::Abs<float>(DropChance.Chance);
	}
	if (TotalWeight <= 0.0f) { return AllGoods; }
	Pick = RandStream.FRandRange(0.0f, TotalWeight);
	for (const FGoodsDropChance& DropChance : DropChances)
	{
		if (DropChance.Chance <= 0.0f) { continue; }
		AccumWeight += DropChance.Chance;
		if (Pick <= AccumWeight)
		{
			AllGoods.Append(GoodsForDropChance(DropChance, QuantityScale));
			break;
		}
	}
	return AllGoods;
}

/*
*/
TArray<FGoodsQuantity> UGoodsDropper::EvaluateGoodsDropChancePercent(const FGoodsDropChance& DropChance, const float QuantityScale)
{
	TArray<FGoodsQuantity> AllGoods;
	if (DropChance.Chance <= 0.0f) { return AllGoods; }
	// Determine if this drop chance evaluates to a successful drop
	if (RandStream.FRandRange(0.0f, 1.0f) <= DropChance.Chance)
	{
		AllGoods = GoodsForDropChance(DropChance, QuantityScale);
	}
	return AllGoods;
}

/*
*/
TArray<FGoodsQuantity> UGoodsDropper::GoodsForDropChance(const FGoodsDropChance & DropChance, const float QuantityScale)
{
	TArray<FGoodsQuantity> AllGoods;
	FGoodsQuantity TmpGoods;

	if (DropChance.GoodsQuantities.Num() > 0)
	{
		// Evaluate all GoodsQuantities and add them to our collection
		AllGoods.Append(UGoodsFunctionLibrary::GoodsQuantitiesFromRanges(RandStream, DropChance.GoodsQuantities, QuantityScale));
	}
	
	// Evaluate any other GoodsDropTables and add them to our collection (if any)
	for (const FName& DropTableName : DropChance.OtherGoodsDrops)
	{
		// Find drop table in library
		const FGoodsDropTable* DropTable = FindDropTableInLibrary(DropTableName);
		if (DropTable) {
			AllGoods.Append(EvaluateGoodsDropTable(*DropTable, QuantityScale));
		}
	}
	return AllGoods;
}
