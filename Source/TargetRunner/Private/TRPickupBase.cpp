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
}

void ATRPickupBase::OnRep_Collected()
{

}

void ATRPickupBase::GetResourceGoods_Implementation(TArray<FGoodsQuantity>& CollectedGoods)
{
	if (!bCollected)
	{
		CollectedGoods.Empty(PickupGoods.Num());
		CollectedGoods.Append(PickupGoods);
	}
	else
	{
		CollectedGoods.Empty();
	}
}

void ATRPickupBase::NotifyCollected_Implementation()
{
	bCollected = true;
	Destroy();
}


FText ATRPickupBase::GetItemDisplayName_Implementation()
{
	if (PickupGoods.Num() > 0)
	{
		return FText::FromString(PickupGoods[0].Name.ToString());
	}
	return FText();
}

FInspectInfo ATRPickupBase::GetInspectInfo_Implementation()
{
	FInspectInfo Info;
	Info.DisplayName = GetItemDisplayName();
	for (FGoodsQuantity Goods : PickupGoods)
	{
		Info.DetailInfo.Add(FInspectInfoItem(FText::FromString(Goods.Name.ToString()), FText::FromString(FString::Printf(TEXT("%.0f"), Goods.Quantity))));
	}
	return Info;
}