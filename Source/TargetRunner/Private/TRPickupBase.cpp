#include "TRPickupBase.h"	

// Constructor for defaults
ATRPickupBase::ATRPickupBase(const FObjectInitializer& OI) : Super(OI)
{
	bReplicates = true;
	// Turn off tick
	PrimaryActorTick.bCanEverTick = false;

	//RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene"));
	//SetRootComponent(RootScene);
}


// Called when the game starts or when spawned
void ATRPickupBase::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void ATRPickupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ATRPickupBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATRPickupBase, bCollected);
	DOREPLIFETIME_CONDITION(ATRPickupBase, PickupAwards, COND_InitialOnly);
}


void ATRPickupBase::OnRep_Collected()
{

}


void ATRPickupBase::OnRep_PickupAwards_Implementation()
{

}


void ATRPickupBase::GetPickupAwards_Implementation(FPickupAwards& PickupsAwarded)
{
	if (!bCollected)
	{
		PickupsAwarded.PickupItems.Empty(PickupAwards.PickupItems.Num());
		PickupsAwarded.PickupItems.Append(PickupAwards.PickupItems);
	}
	else
	{
		PickupsAwarded.PickupItems.Empty();
	}
}


void ATRPickupBase::GetPickupGoods_Implementation(TArray<FGoodsQuantity>& PickupGoods)
{
	if (!bCollected)
	{
		TArray<FGoodsQuantity> PickedUpGoods;
		for (FPickupAwardsItem CurAward : PickupAwards.PickupItems)
		{
			PickedUpGoods.Append(CurAward.PickupGoods);
		}
		PickupGoods.Empty(PickedUpGoods.Num());
		PickupGoods.Append(PickedUpGoods);
	}
	else
	{
		PickupGoods.Empty();
	}
}


void ATRPickupBase::NotifyPickupCollected_Implementation()
{
	bCollected = true;
	Destroy();
}


void ATRPickupBase::OnBeginInspect_Implementation()
{
}


void ATRPickupBase::OnEndInspect_Implementation()
{
}


FText ATRPickupBase::GetItemDisplayName_Implementation()
{
	if (PickupAwards.PickupItems.Num() > 0)
	{
		for (FPickupAwardsItem PickupItem : PickupAwards.PickupItems)
		{
			if (PickupItem.PickupGoods.Num() > 0)
			{
				return FText::FromString(PickupItem.PickupGoods[0].Name.ToString());
			}
		}
		for (FPickupAwardsItem PickupItem : PickupAwards.PickupItems)
		{
			if (PickupItem.PickupEnergy > 0.0f)
			{
				return FText::FromString(FString::Printf(TEXT("%.0f Energy"), PickupItem.PickupEnergy));
			}
			if (PickupItem.PickupAnimus > 0.0f)
			{
				return FText::FromString(FString::Printf(TEXT("%.0f Animus"), PickupItem.PickupAnimus));
			}
		}
	}
	return FText();
}


FInspectInfo ATRPickupBase::GetInspectInfo_Implementation()
{
	FInspectInfo Info;
	Info.DisplayName = GetItemDisplayName();
	for (FPickupAwardsItem AwardsItem : PickupAwards.PickupItems)
	{
		for (FGoodsQuantity Goods : AwardsItem.PickupGoods)
		{
			Info.DetailInfo.Add(FInspectInfoItem(FText::FromString(Goods.Name.ToString()), FText::FromString(FString::Printf(TEXT("%.0f"), Goods.Quantity))));
		}
	}
	return Info;
}


//void ATRPickupBase::GetResourceGoods_Implementation(TArray<FGoodsQuantity>& CollectedGoods)
//{
//	GetPickupGoods(CollectedGoods);
//}


//void ATRPickupBase::NotifyCollected_Implementation()
//{
//	bCollected = true;
//	Destroy();
//}
