// Fill out your copyright notice in the Description page of Project Settings.

#include "GoodsDropper.h"

/*
*/
bool UGoodsDropper::AddDropTableDataToLibrary(UDataTable* GoodsDropTableData)
{
	TArray<FName> RowNames = GoodsDropTableData->GetRowNames();
	if (RowNames.Num() > 0)
	{
		// Check that this is a table of FGoodsDropTable rows.
		FGoodsDropTable* FirstTable = GoodsDropTableData->FindRow<FGoodsDropTable>(RowNames[0], "", false);
		if (FirstTable == nullptr)
		{
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
TArray<FGoodsQuantity> UGoodsDropper::EvaluateGoodsDropTable(UPARAM(ref)FRandomStream& RandStream, const FGoodsDropTable& GoodsTable, const float Level)
{
	TArray<FGoodsQuantity> AllGoods;

	if (GoodsTable.bAsWeightedList)
	{
		AllGoods.Append(EvaluateGoodsDropChanceWeighted(RandStream, GoodsTable.GoodsOddsList, Level));
	}
	else
	{
		// Each entry has a percent chance to be included.
		for (FGoodsDropChance DropChance : GoodsTable.GoodsOddsList)
		{
			if (DropChance.Chance > 0.0f) // Don't bother evaluating if Chance is not > 0
			{
				AllGoods.Append(EvaluateGoodsDropChancePercent(RandStream, DropChance, Level));
			}
		}
	}

	return AllGoods;
}

TArray<FGoodsQuantity> UGoodsDropper::EvaluateGoodsDropTableByName(UPARAM(ref)FRandomStream & RandStream, const FName & DropTableName, const float Level)
{
	TArray<FGoodsQuantity> AllGoods;
	const FGoodsDropTable* FoundDropTable = FindDropTableInLibrary(DropTableName);
	if (FoundDropTable != nullptr)
	{
		AllGoods = EvaluateGoodsDropTable(RandStream, *FoundDropTable, Level);
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
		if (FoundDropTable != nullptr) { break; }
	}

	return FoundDropTable;
}

/*
*/
TArray<FGoodsQuantity> UGoodsDropper::EvaluateGoodsDropChanceWeighted(UPARAM(ref)FRandomStream RandStream, const TArray<FGoodsDropChance>& DropChances, const float Level)
{
	TArray<FGoodsQuantity> AllGoods;
	float TotalWeight = 0.0f;
	float AccumWeight = 0.0f;
	float Pick = 0.0f;

	for (const FGoodsDropChance& DropChance : DropChances)
	{
		TotalWeight += DropChance.Chance;
	}

	Pick = RandStream.FRandRange(0, TotalWeight);
	for (const FGoodsDropChance& DropChance : DropChances)
	{
		AccumWeight += DropChance.Chance;
		if (Pick <= AccumWeight)
		{
			AllGoods.Append(GoodsForDropChance(RandStream, DropChance, Level));
			break;
		}
	}

	return AllGoods;
}

/*
*/
TArray<FGoodsQuantity> UGoodsDropper::EvaluateGoodsDropChancePercent(UPARAM(ref)FRandomStream& RandStream, const FGoodsDropChance& DropChance, const float Level)
{
	TArray<FGoodsQuantity> AllGoods;
	
	// Determine if this drop chance evaluates to a successful drop
	if (RandStream.FRandRange(0.0f, 1.0f) <= DropChance.Chance)
	{
		AllGoods = GoodsForDropChance(RandStream, DropChance, Level);
	}
	return AllGoods;
}

/*
*/
TArray<FGoodsQuantity> UGoodsDropper::GoodsForDropChance(UPARAM(ref)FRandomStream & RandStream, const FGoodsDropChance & DropChance, const float Level)
{
	TArray<FGoodsQuantity> AllGoods;
	FGoodsQuantity TmpGoods;

	// Evaluate all GoodsQuantities and add them to our collection
	for (const FGoodsQuantityRange& GoodsQuantity : DropChance.GoodsQuantities)
	{
		TmpGoods = EvaluateGoodsQuantityRange(RandStream, GoodsQuantity);
		if (TmpGoods.Quantity > 0)
		{
			AllGoods.Add(TmpGoods);
		}
	}

	// Evaluate any other GoodsDropTables and add them to our collection (if any)
	for (const FName& DropTableName : DropChance.OtherGoodsDrops)
	{
		// Find drop table in library
		const FGoodsDropTable* DropTable = FindDropTableInLibrary(DropTableName);
		AllGoods.Append(EvaluateGoodsDropTable(RandStream, *DropTable, Level));
	}
	return AllGoods;
}

/*
*/
FGoodsQuantity UGoodsDropper::EvaluateGoodsQuantityRange(UPARAM(ref)FRandomStream& RandStream, const FGoodsQuantityRange& QuantityRange)
{
	FGoodsQuantity Goods(QuantityRange.GoodsName, 0.0f);

	if (QuantityRange.QuantityMax > 0)
	{
		if (QuantityRange.QuantityMin == QuantityRange.QuantityMax) 
		{
			Goods.Quantity = QuantityRange.QuantityMin;
		}
		else
		{
			Goods.Quantity = RandStream.FRandRange(0.0f, QuantityRange.QuantityMax - QuantityRange.QuantityMin) + QuantityRange.QuantityMin;
		}
	}
	return Goods;
}
