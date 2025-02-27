// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryActorComponent.h"
#include "TargetRunner.h"
#include "NamedPrimitiveTypes.h"
#include "GameFramework/PlayerController.h"

// Sets default values for this component's properties
UInventoryActorComponent::UInventoryActorComponent()
{
	//SetIsReplicated(false);
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
}


//void UInventoryActorComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//	DOREPLIFETIME(UInventoryActorComponent, );
//}


// Called when the game starts
void UInventoryActorComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


bool UInventoryActorComponent::ShouldUpdateClient()
{
	// Only true if we are server authority
	if (GetOwnerRole() < ROLE_Authority) {
		return false;
	}
	bool bUpdateClient = false;
	APlayerController* TmpPC = Cast<APlayerController>(GetOwner());
	if (TmpPC) {
		bUpdateClient = !TmpPC->IsLocalController();
	} 
	else 
	{
		ENetMode NetMode = GetNetMode();
		bUpdateClient = NetMode != NM_Client && NetMode != NM_Standalone;
	}
	UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("InventoryActorComponent - ShouldUpdateClient: %s."), bUpdateClient ? TEXT("True") : TEXT("False"));
	return bUpdateClient;
}


// Called every frame
void UInventoryActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


bool UInventoryActorComponent::AddSubtractGoodsInternal(const FGoodsQuantity& GoodsDelta, const bool bNegateGoodsQuantities, float& NewCurrentQuantity, FGoodsQuantity& NewSnapshotDelta, const bool bAddToSnapshot)
{
	if (GoodsDelta.Quantity == 0.0f) { return true; }

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
		//FGoodsQuantity& TmpGoodsQuantity = Inventory[Index];
		//GoodsQuantity = TmpGoodsQuantity;
		GoodsQuantity = Inventory[Index];
	}
	UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("InventoryActorComponent - AddSubtractGoodsInternal current: %s: %d."), *GoodsQuantity.Name.ToString(), (int32)GoodsQuantity.Quantity);
	NetQuantity = GoodsQuantity.Quantity + (bNegateGoodsQuantities ? GoodsDelta.Quantity * -1.0f : GoodsDelta.Quantity);
	if (NetQuantity < 0.0f) { 
		return false; 
	}
	else
	{
		GoodsQuantity.Quantity = NetQuantity;
		if (Index == INDEX_NONE) {
			Inventory.Add(GoodsQuantity);
		}
		else {
			Inventory[Index].Quantity = GoodsQuantity.Quantity;
		}
		NewCurrentQuantity = GoodsQuantity.Quantity;
		if (bAddToSnapshot)
		{
			NewSnapshotDelta.Name = GoodsDelta.Name;
			NewSnapshotDelta.Quantity = bNegateGoodsQuantities ? GoodsDelta.Quantity * -1.0f : GoodsDelta.Quantity;
			Index = SnapshotInventory.IndexOfByKey(NewSnapshotDelta.Name);
			if (Index == INDEX_NONE) {
				SnapshotInventory.Add(NewSnapshotDelta);
			}
			else {
				SnapshotInventory[Index].Quantity = SnapshotInventory[Index].Quantity + NewSnapshotDelta.Quantity;
			}
		}
		TArray<FGoodsQuantity> TmpTotalGoodsArray;
		TmpTotalGoodsArray.Add(GoodsQuantity);
		TArray<FGoodsQuantity> TmpDeltaGoodsArray;
		TmpDeltaGoodsArray.Add(FGoodsQuantity(GoodsDelta.Name, (bNegateGoodsQuantities ? GoodsDelta.Quantity * -1.0f : GoodsDelta.Quantity)));
		OnInventoryChanged.Broadcast(TmpDeltaGoodsArray, TmpTotalGoodsArray);		
	}
	return true;
}


bool UInventoryActorComponent::AddSubtractGoods(const FGoodsQuantity& GoodsDelta, const bool bNegateGoodsQuantities, float& CurrentQuantity, const bool bAddToSnapshot)
{
	/*
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
	NetQuantity = GoodsQuantity.Quantity + (bNegateGoodsQuantities ? GoodsDelta.Quantity * -1.0f : GoodsDelta.Quantity);
	if (NetQuantity < 0.0f) 
	{ 
		CurrentQuantity = GoodsQuantity.Quantity;
		return false; 
	}
	else
	{
		CurrentQuantity = NetQuantity;
		ServerAddSubtractGoods(GoodsDelta, bNegateGoodsQuantities, bAddToSnapshot);
		return true;
	}
	*/
	FGoodsQuantity SnapshotDelta;
	if (AddSubtractGoodsInternal(GoodsDelta, bNegateGoodsQuantities, CurrentQuantity, SnapshotDelta, bAddToSnapshot))
	{
		if (GetOwnerRole() < ROLE_Authority) {
			ServerAddSubtractGoods(GoodsDelta, bNegateGoodsQuantities, bAddToSnapshot);
		}
		else if (ShouldUpdateClient()) 	{
			ClientUpdateInventoryQuantity(FGoodsQuantity(GoodsDelta.Name, CurrentQuantity), SnapshotDelta);
		}
		return true;
	}
	return false;
}


void UInventoryActorComponent::ServerAddSubtractGoods_Implementation(const FGoodsQuantity& GoodsDelta, const bool bNegateGoodsQuantities, const bool bAddToSnapshot)
{
	UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("InventoryActorComponent - ServerAddSubtractGoods: %s: %d."), *GoodsDelta.Name.ToString(), (int32)GoodsDelta.Quantity * (bNegateGoodsQuantities ? -1 : 1));
	float CurrentQuantity = 0.f;
	FGoodsQuantity SnapshotDelta;
	if (AddSubtractGoodsInternal(GoodsDelta, bNegateGoodsQuantities, CurrentQuantity, SnapshotDelta, bAddToSnapshot))
	{
		// Send authoritative change back to client.
		if (ShouldUpdateClient()) {
			ClientUpdateInventoryQuantity(FGoodsQuantity(GoodsDelta.Name, CurrentQuantity), SnapshotDelta);
		}
	}
}

bool UInventoryActorComponent::ServerAddSubtractGoods_Validate(const FGoodsQuantity& GoodsDelta, const bool bNegateGoodsQuantities, const bool bAddToSnapshot)
{
	return true;
}


bool UInventoryActorComponent::AddSubtractGoodsArrayInternal(const TArray<FGoodsQuantity>& GoodsDeltas, const bool bNegateGoodsQuantities, TArray<FGoodsQuantity>& NewCurrentQuantities, TArray<FGoodsQuantity>& NewSnapshotDeltas, const bool bAddToSnapshot)
{
	if (GoodsDeltas.Num() == 0) {
		return true;
	}
	//TArray<FGoodsQuantity> NewGoods;
	//TArray<FGoodsQuantity> SnapshotDeltas;
	TArray<FGoodsQuantity> NewGoodsDeltas;
	FGoodsQuantity GoodsQuantity;
	float NetQuantity;
	bool bValid = true;
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
			//FGoodsQuantity& TmpGoodsQuantity = Inventory[Index];
			//GoodsQuantity = TmpGoodsQuantity;
			GoodsQuantity = Inventory[Index];
		}
		UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("InventoryActorComponent - AddSubtractGoodsArrayInternal New item: %s, %s: %d."), Index == INDEX_NONE ? TEXT("True") : TEXT("False"), *GoodsQuantity.Name.ToString(), (int32)GoodsQuantity.Quantity);
		NetQuantity = GoodsQuantity.Quantity + (bNegateGoodsQuantities ? TmpGoodsDelta.Quantity * -1.0f : TmpGoodsDelta.Quantity);
		if (NetQuantity < 0.0f) {
			bValid = false;
		}
		NewCurrentQuantities.Add(FGoodsQuantity(GoodsQuantity.Name, NetQuantity));
		NewGoodsDeltas.Add(FGoodsQuantity(TmpGoodsDelta.Name, (bNegateGoodsQuantities ? TmpGoodsDelta.Quantity * -1.0f : TmpGoodsDelta.Quantity)));
	}
	for (int32 i = 0; i < NewCurrentQuantities.Num(); i++)
	{
		Index = Inventory.IndexOfByKey(NewCurrentQuantities[i].Name);
		if (bValid)
		{
			// Make the inv. changes if valid
			if (Index == INDEX_NONE) {
				Inventory.Add(NewCurrentQuantities[i]);
			}
			else {
				Inventory[Index].Quantity = NewCurrentQuantities[i].Quantity;
			}
			UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("InventoryActorComponent - AddSubtractGoodsArrayInternal updated New item: %s, %s: %d."), Index == INDEX_NONE ? TEXT("True") : TEXT("False"), *NewCurrentQuantities[i].Name.ToString(), (int32)NewCurrentQuantities[i].Quantity);
		}
		else {
			// If not valid, set all NewCurrentQuantities to current inventory quantity.
			if (Index == INDEX_NONE) {
				NewCurrentQuantities[i].Quantity = 0.0f;
			}
			else {
				NewCurrentQuantities[i].Quantity = Inventory[Index].Quantity;
			}
		}
	}
	if (!bValid) {
		return false;
	}
	if (bAddToSnapshot)
	{
		for (FGoodsQuantity GoodsDeltaItem : NewGoodsDeltas)
		{
			//FGoodsQuantity SnapshotDelta = FGoodsQuantity(GoodsDeltaItem.Name, bNegateGoodsQuantities ? GoodsDeltaItem.Quantity * -1.0f : GoodsDeltaItem.Quantity);
			NewSnapshotDeltas.Add(GoodsDeltaItem);
			Index = SnapshotInventory.IndexOfByKey(GoodsDeltaItem.Name);
			if (Index == INDEX_NONE) {
				SnapshotInventory.Add(GoodsDeltaItem);
			}
			else {
				SnapshotInventory[Index].Quantity = SnapshotInventory[Index].Quantity + GoodsDeltaItem.Quantity;
			}
		}
	}
	OnInventoryChanged.Broadcast(NewGoodsDeltas, NewCurrentQuantities);
	return true;
}


bool UInventoryActorComponent::AddSubtractGoodsArray(const TArray<FGoodsQuantity>& GoodsDeltas, const bool bNegateGoodsQuantities, TArray<FGoodsQuantity>& CurrentQuantities, const bool bAddToSnapshot)
{
	TArray<FGoodsQuantity> SnapshotDeltas;
	if (AddSubtractGoodsArrayInternal(GoodsDeltas, bNegateGoodsQuantities, CurrentQuantities, SnapshotDeltas, bAddToSnapshot))
	{
		if (GetOwnerRole() < ROLE_Authority) {
			ServerAddSubtractGoodsArray(GoodsDeltas, bNegateGoodsQuantities, bAddToSnapshot);
		}
		else if (ShouldUpdateClient()) {
			ClientUpdateInventoryQuantities(CurrentQuantities, SnapshotDeltas);
		}
		return true;
	}
	return false;
}


void UInventoryActorComponent::ServerAddSubtractGoodsArray_Implementation(const TArray<FGoodsQuantity>& GoodsDeltas, const bool bNegateGoodsQuantities, const bool bAddToSnapshot)
{
	TArray<FGoodsQuantity> CurrentQuantities;
	TArray<FGoodsQuantity> SnapshotDeltas;
	if (AddSubtractGoodsArrayInternal(GoodsDeltas, bNegateGoodsQuantities, CurrentQuantities, SnapshotDeltas, bAddToSnapshot))
	{
		// Send authoritative quantities back to client
		if (ShouldUpdateClient()) {
			ClientUpdateInventoryQuantities(CurrentQuantities, SnapshotDeltas);
		}
	}
}

bool UInventoryActorComponent::ServerAddSubtractGoodsArray_Validate(const TArray<FGoodsQuantity>& GoodsDeltas, const bool bNegateGoodsQuantities, const bool bAddToSnapshot)
{
	return true;
}


void UInventoryActorComponent::SetInventoryInternal(const TArray<FGoodsQuantity>& NewGoods, const TArray<FGoodsQuantity>& NewSnapshotGoods)
{
	Inventory.Empty(NewGoods.Num());
	if (NewGoods.Num() > 0)	{
		Inventory.Append(NewGoods);
	}
	SnapshotInventory.Empty(NewSnapshotGoods.Num());
	if (NewSnapshotGoods.Num() > 0)	{
		SnapshotInventory.Append(NewSnapshotGoods);
	}
}


void UInventoryActorComponent::ServerSetInventory_Implementation(const TArray<FGoodsQuantity>& NewGoods, const TArray<FGoodsQuantity>& NewSnapshotGoods)
{
	SetInventoryInternal(NewGoods, NewSnapshotGoods);
	if (ShouldUpdateClient()) {
		// TODO: Should page the data if it exceeds size allowed in a single RPC call.
		ClientSetInventory(NewGoods, NewSnapshotGoods);
	}
}

bool UInventoryActorComponent::ServerSetInventory_Validate(const TArray<FGoodsQuantity>& NewGoods, const TArray<FGoodsQuantity>& NewSnapshotGoods)
{
	return true;
}


void UInventoryActorComponent::ClientSetInventory_Implementation(const TArray<FGoodsQuantity>& NewGoods, const TArray<FGoodsQuantity>& NewSnapshotGoods)
{
	SetInventoryInternal(NewGoods, NewSnapshotGoods);
}

bool UInventoryActorComponent::ClientSetInventory_Validate(const TArray<FGoodsQuantity>& NewGoods, const TArray<FGoodsQuantity>& NewSnapshotGoods)
{
	return true;
}


void UInventoryActorComponent::ClientUpdateInventoryQuantity_Implementation(const FGoodsQuantity NewQuantity, const FGoodsQuantity SnapshotDelta)
{
	FGoodsQuantity GoodsDelta(NewQuantity.Name, 0.f);
	int32 Index = Inventory.IndexOfByKey(NewQuantity.Name);
	UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("InventoryActorComponent - ClientUpdateInventoryQuantity New item: %s, %s: %d."), Index == INDEX_NONE ? TEXT("True") : TEXT("False"), *NewQuantity.Name.ToString(), (int32)NewQuantity.Quantity);
	if (Index == INDEX_NONE)
	{
		Inventory.Add(NewQuantity);
		GoodsDelta.Quantity = NewQuantity.Quantity;
	}
	else
	{
		GoodsDelta.Quantity = NewQuantity.Quantity - Inventory[Index].Quantity;
		Inventory[Index].Quantity = NewQuantity.Quantity;
	}
	if (!SnapshotDelta.Name.IsNone() && SnapshotDelta.Quantity != 0.0f)
	{
		Index = SnapshotInventory.IndexOfByKey(SnapshotDelta.Name);
		if (Index == INDEX_NONE) {
			SnapshotInventory.Add(SnapshotDelta);
		}
		else {
			SnapshotInventory[Index].Quantity = SnapshotInventory[Index].Quantity + SnapshotDelta.Quantity;
		}
	}
	TArray<FGoodsQuantity> TmpGoodsArray;
	TmpGoodsArray.Add(NewQuantity);
	TArray<FGoodsQuantity> TmpDeltaGoodsArray;
	TmpDeltaGoodsArray.Add(GoodsDelta);
	OnInventoryChanged.Broadcast(TmpDeltaGoodsArray, TmpGoodsArray);
}

bool UInventoryActorComponent::ClientUpdateInventoryQuantity_Validate(const FGoodsQuantity NewQuantity, const FGoodsQuantity SnapshotDelta)
{
	return true;
}


void UInventoryActorComponent::ClientUpdateInventoryQuantities_Implementation(const TArray<FGoodsQuantity>& NewQuantities, const TArray<FGoodsQuantity>& SnapshotDeltas)
{
	int32 Index = INDEX_NONE;
	TArray<FGoodsQuantity> GoodsDeltas;
	for (FGoodsQuantity NewGoodsItem : NewQuantities)
	{
		Index = Inventory.IndexOfByKey(NewGoodsItem.Name);
		UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("InventoryActorComponent - ClientUpdateInventoryQuantities New item: %s, %s: %d."), Index == INDEX_NONE ? TEXT("True") : TEXT("False"), *NewGoodsItem.Name.ToString(), (int32)NewGoodsItem.Quantity);
		if (Index == INDEX_NONE)
		{
			Inventory.Add(NewGoodsItem);
			GoodsDeltas.Add(NewGoodsItem);
		}
		else
		{
			GoodsDeltas.Add(FGoodsQuantity(NewGoodsItem.Name, NewGoodsItem.Quantity - Inventory[Index].Quantity));
			Inventory[Index].Quantity = NewGoodsItem.Quantity;
		}
	}
	for (FGoodsQuantity SnapshotDelta : SnapshotDeltas)
	{
		if (!SnapshotDelta.Name.IsNone() && SnapshotDelta.Quantity != 0.0f)
		{
			Index = SnapshotInventory.IndexOfByKey(SnapshotDelta.Name);
			if (Index == INDEX_NONE) {
				SnapshotInventory.Add(SnapshotDelta);
			}
			else {
				SnapshotInventory[Index].Quantity = SnapshotInventory[Index].Quantity + SnapshotDelta.Quantity;
			}
		}
	}
	OnInventoryChanged.Broadcast(GoodsDeltas, NewQuantities);
}

bool UInventoryActorComponent::ClientUpdateInventoryQuantities_Validate(const TArray<FGoodsQuantity>& NewQuantities, const TArray<FGoodsQuantity>& SnapshotDeltas)
{
	return true;
}


float UInventoryActorComponent::GetGoodsCount(const FName GoodsName)
{
	int32 Index = Inventory.IndexOfByKey(GoodsName);
	if (Index == INDEX_NONE) {
		return 0.0;
	}
	else {
		return Inventory[Index].Quantity;
	}
}


void UInventoryActorComponent::GetAllGoods(TArray<FGoodsQuantity>& AllGoods)
{
	AllGoods.Empty(Inventory.Num());
	AllGoods.Append(Inventory);
}


void UInventoryActorComponent::GetSaveableGoods(TArray<FGoodsQuantity>& AllSaveableGoods)
{	
	if (UnsaveableGoodsFilters.Num() == 0)
	{
		AllSaveableGoods.Empty(Inventory.Num());
		AllSaveableGoods.Append(Inventory);
	}
	else
	{
		bool bSaveable;
		AllSaveableGoods.Empty();
		for (FGoodsQuantity Goods : Inventory)
		{
			bSaveable = true;
			for (FString FilterStr : UnsaveableGoodsFilters)
			{
				if (Goods.Name.ToString().Contains(FilterStr))
				{
					bSaveable = false;
					break;
				}
			}
			if (bSaveable) { AllSaveableGoods.Add(Goods); }
		}
	}
}


bool UInventoryActorComponent::HasGoods(const FGoodsQuantity Goods, float& CurrentQuantity)
{
	CurrentQuantity = GetGoodsCount(Goods.Name);
	return CurrentQuantity >= Goods.Quantity;
}


bool UInventoryActorComponent::HasAllGoods(const TArray<FGoodsQuantity> Goods, TArray<FGoodsQuantity>& CurrentQuantities)
{
	bool bHasAll = true;
	float TmpCurQuantity = 0.0f;
	CurrentQuantities.Empty(Goods.Num());
	for (FGoodsQuantity TmpGQ : Goods)
	{
		if (!HasGoods(TmpGQ, TmpCurQuantity)) { bHasAll = false; }
		CurrentQuantities.Add(FGoodsQuantity(TmpGQ.Name, TmpCurQuantity));
	}
	return bHasAll;
}


bool UInventoryActorComponent::IsEmpty()
{
	return Inventory.Num() == 0;
}

/*
void UInventoryActorComponent::ServerSetSnapshotInventory_Implementation(const TArray<FGoodsQuantity>& NewSnapshotGoods)
{
	SnapshotInventory.Empty(NewSnapshotGoods.Num());
	SnapshotInventory.Append(NewSnapshotGoods);
	if (ShouldUpdateClient())
	{
		// Update client
		ClientSetSnapshotInventory(NewSnapshotGoods);
	}
}

bool UInventoryActorComponent::ServerSetSnapshotInventory_Validate(const TArray<FGoodsQuantity>& NewSnapshotGoods)
{
	return true;
}

void UInventoryActorComponent::ClientSetSnapshotInventory_Implementation(const TArray<FGoodsQuantity>& NewSnapshotGoods)
{
	SnapshotInventory.Empty(NewSnapshotGoods.Num());
	SnapshotInventory.Append(NewSnapshotGoods);
}

bool UInventoryActorComponent::ClientSetSnapshotInventory_Validate(const TArray<FGoodsQuantity>& NewSnapshotGoods)
{
	return true;
}
*/

void UInventoryActorComponent::ServerClearSnapshotInventory_Implementation()
{
	SnapshotInventory.Empty();
	if (ShouldUpdateClient())
	{
		// Update client
		ClientClearSnapshotInventory();
	}
}

bool UInventoryActorComponent::ServerClearSnapshotInventory_Validate()
{
	return true;
}

void UInventoryActorComponent::ClientClearSnapshotInventory_Implementation()
{
	SnapshotInventory.Empty();
}

bool UInventoryActorComponent::ClientClearSnapshotInventory_Validate()
{
	return true;
}

void UInventoryActorComponent::GetSnapshotGoods(TArray<FGoodsQuantity>& AllSnapshotGoods)
{
	AllSnapshotGoods.Append(SnapshotInventory);
}

bool UInventoryActorComponent::SnapshotInventoryIsEmpty()
{
	return SnapshotInventory.Num() == 0;
}