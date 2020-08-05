// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryActorComponent.h"
#include "GameFramework/PlayerController.h"

// Sets default values for this component's properties
UInventoryActorComponent::UInventoryActorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


//void UInventoryActorComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//	DOREPLIFETIME(UInventoryActorComponent, Inventory);
//}


// Called when the game starts
void UInventoryActorComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


bool UInventoryActorComponent::ShouldUpdateClient()
{
	bool bUpdateClient = false;
	APlayerController* TmpPC = Cast<APlayerController>(GetOwner());
	if (TmpPC)
	{
		bUpdateClient = !TmpPC->IsLocalController();
	} 
	else 
	{
		ENetMode NetMode = GetNetMode();
		bUpdateClient = NetMode != NM_Client && NetMode != NM_Standalone;
	}
	return bUpdateClient;
}


// Called every frame
void UInventoryActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


bool UInventoryActorComponent::AddSubtractGoods(const FGoodsQuantity& GoodsDelta, float& CurrentQuantity)
{
	FGoodsQuantity GoodsQuantity;
	float NetQuantity;
	int32 Index = Inventory.IndexOfByKey(GoodsDelta.Name);
	if (Index == INDEX_NONE)
	{
		GoodsQuantity.Name = GoodsDelta.Name;
		GoodsQuantity.Quantity = 0.0f;
	}
	else
	{
		FGoodsQuantity& TmpGoodsQuantity = Inventory[Index];
		GoodsQuantity = TmpGoodsQuantity;
	}
	if (GoodsDelta.Quantity == 0.0f) 
	{ 
		CurrentQuantity = GoodsQuantity.Quantity;
		return true; 
	}
	NetQuantity = GoodsQuantity.Quantity - GoodsDelta.Quantity;
	if (NetQuantity < 0.0f) 
	{ 
		CurrentQuantity = GoodsQuantity.Quantity;
		return false; 
	}
	else
	{
		CurrentQuantity = NetQuantity;
		ServerAddSubtractGoods(GoodsDelta);
		return true;
	}
}


void UInventoryActorComponent::ServerAddSubtractGoods_Implementation(const FGoodsQuantity& GoodsDelta)
{
	if (GoodsDelta.Quantity == 0.0f) { return; }

	FGoodsQuantity GoodsQuantity;
	float NetQuantity;
	int32 Index = Inventory.IndexOfByKey(GoodsDelta.Name);
	if (Index == INDEX_NONE)
	{
		GoodsQuantity.Name = GoodsDelta.Name;
		GoodsQuantity.Quantity = 0.0f;
	}
	else
	{
		FGoodsQuantity& TmpGoodsQuantity = Inventory[Index];
		GoodsQuantity = TmpGoodsQuantity;
	}
	
	NetQuantity = GoodsQuantity.Quantity - GoodsDelta.Quantity;
	if (NetQuantity < 0.0f) { return; }
	else
	{
		GoodsQuantity.Quantity = NetQuantity;
		if (Index == INDEX_NONE)
		{
			Inventory.Add(GoodsQuantity);
		}
		else
		{
			Inventory[Index].Quantity = GoodsQuantity.Quantity;
		}
		TArray<FGoodsQuantity> TmpGoodsArray;
		TmpGoodsArray.Add(GoodsQuantity);
		OnInventoryChanged.Broadcast(TmpGoodsArray);
		if (ShouldUpdateClient())
		{
			// Update client
			ClientUpdateInventoryQuantity(GoodsQuantity);
		}
	}
}

bool UInventoryActorComponent::ServerAddSubtractGoods_Validate(const FGoodsQuantity& GoodsDelta)
{
	return true;
}


bool UInventoryActorComponent::AddSubtractGoodsArray(const TArray<FGoodsQuantity>& GoodsDeltas, TArray<FGoodsQuantity>& CurrentQuantities)
{
	TArray<FGoodsQuantity> CurrentGoods;
	TArray<FGoodsQuantity> NewGoods;
	FGoodsQuantity GoodsQuantity;
	float NetQuantity;
	int32 Index = INDEX_NONE;
	bool bCanMakeUpdate = true;
	for (FGoodsQuantity TmpGoodsDelta : GoodsDeltas)
	{
		Index = Inventory.IndexOfByKey(TmpGoodsDelta.Name);
		if (Index == INDEX_NONE)
		{
			GoodsQuantity.Name = TmpGoodsDelta.Name;
			GoodsQuantity.Quantity = 0.0f;
		}
		else
		{
			FGoodsQuantity& TmpGoodsQuantity = Inventory[Index];
			GoodsQuantity = TmpGoodsQuantity;
		}
		CurrentGoods.Add(GoodsQuantity);
		NetQuantity = GoodsQuantity.Quantity - TmpGoodsDelta.Quantity;
		if (NetQuantity < 0.0f) {
			bCanMakeUpdate = false;
			NewGoods.Add(GoodsQuantity);
		}
		else
		{
			NewGoods.Add(FGoodsQuantity(GoodsQuantity.Name, NetQuantity));
		}
	}
	if (bCanMakeUpdate)
	{
		CurrentQuantities = NewGoods;
		ServerAddSubtractGoodsArray(GoodsDeltas);
		return true;
	}
	else
	{
		CurrentQuantities = CurrentGoods;
		return false;
	}
}


void UInventoryActorComponent::ServerAddSubtractGoodsArray_Implementation(const TArray<FGoodsQuantity>& GoodsDeltas)
{
	TArray<FGoodsQuantity> NewGoods;
	FGoodsQuantity GoodsQuantity;
	float NetQuantity;
	int32 Index = INDEX_NONE;
	for (FGoodsQuantity TmpGoodsDelta : GoodsDeltas)
	{
		Index = Inventory.IndexOfByKey(TmpGoodsDelta.Name);
		if (Index == INDEX_NONE)
		{
			GoodsQuantity.Name = TmpGoodsDelta.Name;
			GoodsQuantity.Quantity = 0.0f;
		}
		else
		{
			FGoodsQuantity& TmpGoodsQuantity = Inventory[Index];
			GoodsQuantity = TmpGoodsQuantity;
		}
		NetQuantity = GoodsQuantity.Quantity - TmpGoodsDelta.Quantity;
		if (NetQuantity < 0.0f) {
			return;
		}
		else
		{
			NewGoods.Add(FGoodsQuantity(GoodsQuantity.Name, NetQuantity));
		}
	}
	for (FGoodsQuantity NewGoodsItem : NewGoods)
	{
		Index = Inventory.IndexOfByKey(NewGoodsItem.Name);
		if (Index == INDEX_NONE)
		{
			Inventory.Add(NewGoodsItem);
		}
		else
		{
			Inventory[Index].Quantity = NewGoodsItem.Quantity;
		}
	}
	OnInventoryChanged.Broadcast(NewGoods);
	if (ShouldUpdateClient())
	{
		ClientUpdateInventoryQuantities(NewGoods);
	}
}

bool UInventoryActorComponent::ServerAddSubtractGoodsArray_Validate(const TArray<FGoodsQuantity>& GoodsDeltas)
{
	return true;
}


void UInventoryActorComponent::ClientUpdateInventoryQuantity_Implementation(const FGoodsQuantity NewQuantity)
{
	int32 Index = Inventory.IndexOfByKey(NewQuantity.Name);
	if (Index == INDEX_NONE)
	{
		Inventory.Add(NewQuantity);
	}
	else
	{
		Inventory[Index].Quantity = NewQuantity.Quantity;
	}
	TArray<FGoodsQuantity> TmpGoodsArray;
	TmpGoodsArray.Add(NewQuantity);
	OnInventoryChanged.Broadcast(TmpGoodsArray);
}

bool UInventoryActorComponent::ClientUpdateInventoryQuantity_Validate(const FGoodsQuantity NewQuantity)
{
	return true;
}


void UInventoryActorComponent::ClientUpdateInventoryQuantities_Implementation(const TArray<FGoodsQuantity>& NewQuantities)
{
	int32 Index = INDEX_NONE;
	for (FGoodsQuantity NewGoodsItem : NewQuantities)
	{
		Index = Inventory.IndexOfByKey(NewGoodsItem.Name);
		if (Index == INDEX_NONE)
		{
			Inventory.Add(NewGoodsItem);
		}
		else
		{
			Inventory[Index].Quantity = NewGoodsItem.Quantity;
		}
	}
	OnInventoryChanged.Broadcast(NewQuantities);
}

bool UInventoryActorComponent::ClientUpdateInventoryQuantities_Validate(const TArray<FGoodsQuantity>& NewQuantities)
{
	return true;
}