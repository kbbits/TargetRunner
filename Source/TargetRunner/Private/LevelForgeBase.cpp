
#include "LevelForgeBase.h"
#include "..\Public\LevelForgeBase.h"
#include "TextRow.h"
#include "TieredGoodsQuantityRange.h"
#include "RoomSpecialActorsByTier.h"
#include "ResourceFunctionLibrary.h"

ULevelForgeBase::ULevelForgeBase(const FObjectInitializer& OI)
	: Super(OI)
{
	MAX_TIER = 10;
	MAX_LEVEL = 20;
	MAX_EXTENT_X = 5;
	MAX_EXTENT_Y = 5;
	BaseResourceQuantityRange = FFloatRange(5000.0f, 6000.0f);
	ResourceQuantityTierScalingExp = 0.5f;
	HigherTierResourceQuantityMultiplier = 0.2f;
	ResourceQuantityLevelScalingExp = 0.25f;
	BaseUnlockCost = 1000.0f;
	UnlockCostScalingExp = 1.75f;
	UnlockCostLevelScalingExp = 0.25f;
	BaseAvailableTime = 300.0f;
	AvailableTimeScaleExp = 0.75f;
	AvailableTimeLevelScaleExp = 0.25f;
}

void ULevelForgeBase::GenerateNewLevelTemplate(const int32 NewSeed, const float Tier, const float DifficultyLevel, FLevelTemplate& NewLevelTemplate, bool& Successful)
{
	DebugLog(FString::Printf(TEXT("GenerateNewLevelTemplate Start. NewSeed: %d"), NewSeed));
	Successful = false;
	NewLevelTemplate.LevelSeed = NewSeed;
	LevelStream.Initialize(NewSeed);
	LevelStream.Reset();
	
	NewLevelTemplate.Tier = Tier;
	NewLevelTemplate.DifficultyLevel = DifficultyLevel;
	NewLevelTemplate.GridForgeType = FName(TEXT("Default"));
	if (!GenerateDisplayName(NewLevelTemplate.DisplayName)) { return; }
	if (!GenerateThumbnail(NewLevelTemplate.Thumbnail)) { return; }
	if (!GenerateGridExtents(Tier, DifficultyLevel, NewLevelTemplate)) { return; }
	NewLevelTemplate.ResourcesAvailable.Empty();
	if (!GenerateResourcesAvailable(Tier, DifficultyLevel, NewLevelTemplate.ResourcesAvailable)) { return; }
	NewLevelTemplate.SpecialsAvailable.Empty();
	if (!GenerateSpecialsAvailable(Tier, DifficultyLevel, NewLevelTemplate.SpecialsAvailable)) { return; }
	NewLevelTemplate.AvailableTime = FMath::Clamp<float>((FMath::Pow(Tier, AvailableTimeScaleExp) * ((BaseAvailableTime * (FMath::Pow(DifficultyLevel, AvailableTimeLevelScaleExp))) / 10.0f)) * 10.0f, BaseAvailableTime, 14400.0f);
	if (Tier > 3 || (Tier > 0 && DifficultyLevel > 9))
	{
		NewLevelTemplate.StartHourOfDay = 9.0f + (12.0f + FMath::Clamp(LevelStream.FRandRange(Tier * -1.5f, Tier * 1.5f), -12.0f, 12.0f));
	}
	else
	{
		NewLevelTemplate.StartHourOfDay = 9.0f + (LevelStream.FRandRange(0.0f, 4.0f) - 2.0f);
	}
	// Unlock cost
	TArray<FGoodsQuantityRange> CostFactors;
	float GoodsCostFactor = 0.0f;
	NewLevelTemplate.UnlockCost = FMath::RoundFromZero(FMath::Pow(Tier, UnlockCostScalingExp) * ((BaseUnlockCost * (FMath::Pow(DifficultyLevel, UnlockCostLevelScalingExp))) / 10.0f)) * 10.f;
	UnlockGoodsCostFactorForTier((int32)Tier, CostFactors);
	for (FGoodsQuantityRange TmpRange : CostFactors)
	{
		GoodsCostFactor = LevelStream.FRandRange(0.0f, TmpRange.QuantityMax - TmpRange.QuantityMin) + TmpRange.QuantityMin;
		NewLevelTemplate.UnlockGoods.Add(FGoodsQuantity(TmpRange.GoodsName, FMath::RoundFromZero((NewLevelTemplate.UnlockCost * GoodsCostFactor) / 10.0f) * 10.0f));
	}
	
	// TODO: Theme, ThemeTags, OtherResourcesAvailable

	// Set the ID as compound key
	NewLevelTemplate.LevelId = FName(FString::Printf(TEXT("%d:%d:%d:%s:%s"), (int32)NewLevelTemplate.Tier, NewLevelTemplate.DifficultyLevel, NewLevelTemplate.LevelSeed, *NewLevelTemplate.GridForgeType.ToString(), *NewLevelTemplate.Theme.ToString()));

	DebugLog(FString::Printf(TEXT("GenerateNewLevelTemplate Successful.")));
	Successful = true;
}


void ULevelForgeBase::UnlockGoodsCostFactorForTier(const int32 Tier, TArray<FGoodsQuantityRange>& CostFactors)
{
	FTieredGoodsQuantityRange* TierRow = UnlockGoodsCostTable->FindRow<FTieredGoodsQuantityRange>(FName(FString::FromInt(Tier)), "", false);
	if (TierRow)
	{
		CostFactors.Empty(TierRow->QuantityRanges.Num());
		CostFactors.Append(TierRow->QuantityRanges);
	}
	else {
		CostFactors.Empty();
	}
}


/*===================  Protected "generator" functions called by public functions ===========*/

bool ULevelForgeBase::GenerateDisplayName(FText& DisplayName)
{
	DebugLog(FString::Printf(TEXT("GenerateDisplayName")));
	if (!VerifyNameTablesValid()) { return false; }

	FFormatNamedArguments CompoundName;
	TArray<FName> RowNames = DescriptiveNamesTable->GetRowNames();
	FTextRow* PickedRow = DescriptiveNamesTable->FindRow<FTextRow>(RowNames[LevelStream.RandRange(0, RowNames.Num() - 1)], "", false);
	CompoundName.Add("Descriptive", PickedRow->Text);
	
	RowNames = ProperNamesTable->GetRowNames();
	PickedRow = ProperNamesTable->FindRow<FTextRow>(RowNames[LevelStream.RandRange(0, RowNames.Num() -1)], "", false);
	CompoundName.Add("Proper", PickedRow->Text);

	DisplayName = FText::Format(NSLOCTEXT("TargetRunner", "LevelNameFmt", "{Descriptive} {Proper}"), CompoundName);

	DebugLog(FString::Printf(TEXT("GenerateDisplayName: %s"), *DisplayName.ToString()));
	return true;
}


bool ULevelForgeBase::GenerateThumbnail(TAssetPtr<UTexture2D> Thumbnail)
{
	Thumbnail = DefaultThumbnail;
	return true;
}


bool ULevelForgeBase::GenerateGridExtents(const float Tier, const int32 DifficultyLevel, FLevelTemplate& LevelTemplate)
{
	DebugLog(TEXT("GenerateGridExtents"));
	//MAX_TIER > 0 ? (Tier / static_cast<float>(MAX_TIER)) : 1.0f;
	float NormalizedDifficulty = MAX_LEVEL > 0 ? (static_cast<float>(DifficultyLevel) / static_cast<float>(MAX_LEVEL)) : 1.f; 
	int32 TmpExtent;
	TmpExtent = (int32)FMath::RoundHalfToZero(FMath::GetMappedRangeValueClamped(FVector2D(1, MAX_LEVEL), FVector2D(1, MAX_EXTENT_X), DifficultyLevel));
	LevelTemplate.MinX = -TmpExtent;
	LevelTemplate.MaxX = TmpExtent;
	TmpExtent = (int32)FMath::RoundHalfToZero(FMath::GetMappedRangeValueClamped(FVector2D(1, MAX_LEVEL), FVector2D(1, MAX_EXTENT_Y), DifficultyLevel));
	LevelTemplate.MinY = -TmpExtent;
	LevelTemplate.MaxY = TmpExtent;
	// Chance to adjust MaxX by +/- 1
	if (LevelStream.FRandRange(0.0f, 1.0f) < (NormalizedDifficulty + (Tier * 0.05f)))
	{
		LevelTemplate.MaxX += FMath::Clamp<int32>(LevelStream.FRandRange(0.0f, 1.0f) >= 0.5f ? 1 : -1, 1, MAX_EXTENT_X);
	}
	// Chance to adjust MaxY by +/- 1
	if (LevelStream.FRandRange(0.0f, 1.0f) < (NormalizedDifficulty + (Tier * 0.05f)))
	{
		LevelTemplate.MaxY += FMath::Clamp<int32>(LevelStream.FRandRange(0.0f, 1.0f) >= 0.5f ? 1 : -1, 1, MAX_EXTENT_Y);
	}
	DebugLog(FString::Printf(TEXT("GenerateGridExtents done MinX:%d MinY:%d MaxX:%d MaxY%d"), LevelTemplate.MinX, LevelTemplate.MinY, LevelTemplate.MaxX, LevelTemplate.MaxY));
	return true;
}


bool ULevelForgeBase::GenerateResourcesAvailable(const float Tier, const int32 DifficultyLevel, TArray<FResourceQuantity>& ResourcesAvailable)
{
	DebugLog(TEXT("GenerateResourcesAvailable"));
	TMap<int32, FResourceTypeDataCollection> TierResources;
	if (UResourceFunctionLibrary::ResourceDataByTier(ResourceDataTable, 0.0f, Tier + 1, TierResources) == 0) 
	{ 
		UE_LOG(LogTRGame, Warning, TEXT("%s - GenerateResourcesAvailable - No resoures available in tiers 0-%d"), *this->GetName(), (int32)Tier + 1);
		return true;
	}
	if (BaseResourceQuantityRange.IsEmpty())
	{
		UE_LOG(LogTRGame, Error, TEXT("%s - GenerateResourcesAvailable - No BaseResourceQuantityRange set."), *this->GetName());
		return false;
	}

	TArray<int32> TiersAvailable;
	FResourceTypeDataCollection* TmpTierResourceData;
	FResourceType TmpResourceType;
	float TmpQuantity;
	float TierDeltaMultiplier;
	float LevelDeltaMultiplier = ResourceQuantityLevelScalingExp > 0 ? FMath::Pow(DifficultyLevel, ResourceQuantityLevelScalingExp) : 1.f;

	TierResources.GetKeys(TiersAvailable);
	for (int32 TmpTier : TiersAvailable)
	{
		TierDeltaMultiplier = 1.0f / FMath::Pow(FMath::Abs(Tier - TmpTier) + 1.0f, ResourceQuantityTierScalingExp);
		TmpTierResourceData = TierResources.Find(TmpTier);
		for (FResourceTypeData TmpResourceData : TmpTierResourceData->Data)
		{
			UResourceFunctionLibrary::ResourceTypeForData(TmpResourceData, TmpResourceType);
			if (TmpResourceType.IsValid())
			{
				DebugLog(FString::Printf(TEXT("    generating: %s"), *TmpResourceType.Code.ToString()));
				TmpQuantity = (LevelStream.FRandRange(0.0f, BaseResourceQuantityRange.GetUpperBoundValue() - BaseResourceQuantityRange.GetLowerBoundValue()) + BaseResourceQuantityRange.GetLowerBoundValue()) * TierDeltaMultiplier;
				if (TmpTier > Tier)
				{
					TmpQuantity = TmpQuantity * HigherTierResourceQuantityMultiplier;
				}
				TmpQuantity = FMath::RoundHalfToZero(TmpQuantity);
				if (TmpQuantity > 0.0f)
				{
					ResourcesAvailable.Add(FResourceQuantity(TmpResourceType, TmpQuantity * LevelDeltaMultiplier));
				}
			}
			else
			{
				UE_LOG(LogTRGame, Error, TEXT("%s - GenerateResourcesAvailable - invalid resource: %s"), *this->GetName(), *TmpResourceType.Code.ToString());
			}
		}
	}

	DebugLog(FString::Printf(TEXT("GenerateResourcesAvailable generated:")));
	for (FResourceQuantity dbgQuantity : ResourcesAvailable)
	{
		DebugLog(FString::Printf(TEXT("    %s: %d"), *dbgQuantity.ResourceType.Code.ToString(), (int32)dbgQuantity.Quantity));
	}
	return true;
}


bool ULevelForgeBase::GenerateSpecialsAvailable(const float Tier, const int32 DifficultyLevel, TArray<TSubclassOf<AActor>>& SpecialsAvailable)
{
	DebugLog(TEXT("GenerateSpecialsAvailable"));
	FRoomSpecialActorsByTier* PickedRow;
	PickedRow = RoomSpecialActorsDataTable->FindRow<FRoomSpecialActorsByTier>(FName(FString::FromInt(FMath::TruncToInt(Tier))), "", false);
	if (PickedRow == nullptr) {
		UE_LOG(LogTRGame, Error, TEXT("%s - GenerateSpecialsAvailable - RoomSpecialActorsDataTable does not contain RoomSpecialActorsByTier rows."), *this->GetName());
		return false;
	}

	FSpecialActorChance Item;
	float TotalWeight = 0.0f;
	float AccumWeight = 0.0f;
	float pick = 0.0f;

	for (FSpecialActorsChanceSet CurChanceSet : PickedRow->SpecialActorChances)
	{
		DebugLog(FString::Printf(TEXT("    %3.0f pct chance for special"), CurChanceSet.ChanceToSpawn * 100.0f));
		if (LevelStream.FRandRange(0.0f, 1.0f) <= CurChanceSet.ChanceToSpawn)
		{
			TotalWeight = 0.0f;
			for (auto Itr(CurChanceSet.WeightedSpecialActorChances.CreateIterator()); Itr; Itr++)
			{
				TotalWeight += Itr->Weight;
			}
			for (int i = 1; i <= CurChanceSet.NumPicks; i++)
			{
				DebugLog(FString::Printf(TEXT("    picking %d"), i));
				pick = LevelStream.FRandRange(0, TotalWeight);
				AccumWeight = 0;
				for (auto Itr(CurChanceSet.WeightedSpecialActorChances.CreateIterator()); Itr; Itr++)
				{
					if (pick <= (AccumWeight + Itr->Weight))
					{
						Item = CurChanceSet.WeightedSpecialActorChances[Itr.GetIndex()];
						SpecialsAvailable.Append(Item.SpecialActorClasses);
						break;
					}
					else {
						AccumWeight += Itr->Weight;
					}
				}
			}
		}
	}
	DebugLog(FString::Printf(TEXT("GenerateSpecialsAvailable generated:")));
	for (TSubclassOf<AActor> CurClass : SpecialsAvailable)
	{
		DebugLog(FString::Printf(TEXT("    %s"), *GetNameSafe(CurClass)));
	}
	return true;
}


FString ULevelForgeBase::LevelTemplateToString(const FLevelTemplate& _LevelTemplate)
{
	return FString::Printf(TEXT("%s seed: %d"), *_LevelTemplate.DisplayName.ToString(), _LevelTemplate.LevelSeed);
}

/*================ Private functions ============================*/

bool ULevelForgeBase::VerifyNameTablesValid()
{
	bool bValid = true;
	if (ProperNamesTable == nullptr || DescriptiveNamesTable == nullptr) 
	{ 
		UE_LOG(LogTRGame, Error, TEXT("%s - GenerateDisplayName - Name tables are invalid."), *this->GetName());
		return false;
	}

	// Check ProperNamesTable has > 0 rows
	TArray<FName> RowNames = ProperNamesTable->GetRowNames();
	FTextRow* FirstRow = nullptr;
	if (RowNames.Num() == 0) { bValid = false; }
	// Check that ProperNamesTable is a table of FTextRow rows.
	if (bValid) {
		FirstRow = ProperNamesTable->FindRow<FTextRow>(RowNames[0], "", false);
		if (FirstRow == nullptr) { bValid = false; }
	}

	// Check DescriptiveNamesTable
	RowNames = DescriptiveNamesTable->GetRowNames();
	if (RowNames.Num() == 0) { bValid = false; }
	if (bValid) {
		// Check that DescriptiveNamesTable is a table of FTextRow rows.
		FirstRow = DescriptiveNamesTable->FindRow<FTextRow>(RowNames[0], "", false);
		if (FirstRow == nullptr) { bValid = false; }
	}
	
	if (!bValid)
	{
		UE_LOG(LogTRGame, Error, TEXT("%s - GenerateDisplayName - One or more name tables are empty."), *this->GetName());
	}
	return bValid;
}