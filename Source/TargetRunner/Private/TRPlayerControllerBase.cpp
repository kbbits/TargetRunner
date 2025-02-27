// Fill out your copyright notice in the Description page of Project Settings.


#include "TRPlayerControllerBase.h"
#include "ToolBase.h"
#include "TRPlayerState.h"
#include "TargetRunner.h"
#include "TRGameModeLobby.h"
#include "GoodsFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
// For intellisense
#include "..\Public\TRPlayerControllerBase.h"

ATRPlayerControllerBase::ATRPlayerControllerBase()
	: Super()
{
	FactionId = FGenericTeamId(static_cast<uint8>(ETRFaction::Player));
	MaxEquippedWeapons = 1;
	MaxEquippedEquipment = 1;
	CurrentTool = nullptr;
	GoodsInventory = CreateDefaultSubobject<UInventoryActorComponent>(TEXT("GoodsInventory"));
	if (GoodsInventory)	{
		AddOwnedComponent(GoodsInventory);
		GoodsInventory->SetIsReplicated(true);
	}
	PersistentDataComponent = CreateDefaultSubobject<UTRPersistentDataComponent>(TEXT("PersistentDataComponent"));
	if (PersistentDataComponent) {
		AddOwnedComponent(PersistentDataComponent);
		PersistentDataComponent->SetIsReplicated(true);
	}
	else { UE_LOG(LogTRGame, Error, TEXT("TRPlayerController constructor failed to create TRPersistentDataComponent")); }//, *UTRPersistentDataComponent::StaticClass()->GetName()) }

	RunSpeedAttribute = CreateDefaultSubobject<UActorAttributeComponent>(TEXT("RunSpeedAttribute"));
	if (RunSpeedAttribute)
	{
		AddOwnedComponent(RunSpeedAttribute);
		RunSpeedAttribute->SetIsReplicated(true);
		RunSpeedAttribute->AttributeData.Name = FName(TEXT("RunSpeed"));
		//RunSpeedAttribute->AttributeData.MinValue = 100;
		//RunSpeedAttribute->AttributeData.MaxValue = 600;
		//RunSpeedAttribute->AttributeData.CurrentValue = 600;
		RunSpeedAttribute->OnDeltaCurrent.AddDynamic(this, &ATRPlayerControllerBase::OnRunSpeedChanged);
	}
	else { UE_LOG(LogTRGame, Error, TEXT("TRPlayerController constructor failed to create RunSpeedAttribute component")); }

	JumpForceAttribute = CreateDefaultSubobject<UActorAttributeComponent>(TEXT("JumpForceAttribute"));
	if (JumpForceAttribute)
	{
		AddOwnedComponent(JumpForceAttribute);
		JumpForceAttribute->SetIsReplicated(true);
		JumpForceAttribute->AttributeData.Name = FName(TEXT("JumpForce"));
		JumpForceAttribute->OnDeltaCurrent.AddDynamic(this, &ATRPlayerControllerBase::OnJumpForceChanged);
	}
	else { UE_LOG(LogTRGame, Error, TEXT("TRPlayerController constructor failed to create JumpForceAttribute component")); }

	CollectionRangeAttribute = CreateDefaultSubobject<UActorAttributeComponent>(TEXT("CollectionRangeAttribute"));
	if (CollectionRangeAttribute)
	{
		AddOwnedComponent(CollectionRangeAttribute);
		CollectionRangeAttribute->SetIsReplicated(true);
		CollectionRangeAttribute->AttributeData.Name = FName(TEXT("CollectionRange"));
		CollectionRangeAttribute->OnDeltaCurrent.AddDynamic(this, &ATRPlayerControllerBase::OnCollectionRangeChanged);
	}
	else { UE_LOG(LogTRGame, Error, TEXT("TRPlayerController constructor failed to create CollectionRangeAttribute component")); }
}


void ATRPlayerControllerBase::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
	OnPreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);
}


void ATRPlayerControllerBase::SeamlessTravelFrom(class APlayerController* OldPC)
{
	Super::SeamlessTravelFrom(OldPC);
	
	if (!OldPC) {
		UE_LOG(LogTRGame, Warning, TEXT("TRPlayerControllerBase::SeamlessTravelFrom old player controller is null"));
		return;
	}
	FPlayerSaveData TmpSaveData;
	UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("TRPlayerControllerBase::SeamlessTravelFrom %s to %s"), *OldPC->GetName(), *this->GetName());
	ATRPlayerControllerBase* OldTRPlayerController = Cast<ATRPlayerControllerBase>(OldPC);
	if (OldTRPlayerController)
	{
		// Get data from old controller. Allow player state to be null on this call. We don't have one yet. 
		// It's travelling properites are handled in it's own CopyProperties.
		OldTRPlayerController->GetPlayerSaveData(TmpSaveData, true);
		// Update current controller with that data. This won't update PlayerState since GUID will be invalid.
		UpdateFromPlayerSaveData(TmpSaveData);
		// Some data not saved in SaveData
		FactionId = OldTRPlayerController->FactionId;

		// TODO: Verify this works on client too -- or we will need to send data to client here

		/*
		// Inventory
		if (OldTRPlayerController->GoodsInventory)
		{
			TArray<FGoodsQuantity> TmpGoods;
			TArray<FGoodsQuantity> TmpSnapGoods;
			OldTRPlayerController->GoodsInventory->GetAllGoods(TmpGoods);
			OldTRPlayerController->GoodsInventory->GetSnapshotGoods(TmpSnapGoods);
			GoodsInventory->ServerSetInventory(TmpGoods, TmpSnapGoods);
		}
		// Attributes
		if (RunSpeedAttribute && OldTRPlayerController->RunSpeedAttribute) {
			RunSpeedAttribute->CopyPropertiesFromOther(OldTRPlayerController->RunSpeedAttribute);
		}
		if (JumpForceAttribute && OldTRPlayerController->JumpForceAttribute) {
			JumpForceAttribute->CopyPropertiesFromOther(OldTRPlayerController->JumpForceAttribute);
		}
		if (CollectionRangeAttribute && OldTRPlayerController->CollectionRangeAttribute) {
			CollectionRangeAttribute->CopyPropertiesFromOther(OldTRPlayerController->CollectionRangeAttribute);
		}
		// Tool Inventory
		ToolInventory.Empty(OldTRPlayerController->ToolInventory.Num());
		ToolInventory.Append(OldTRPlayerController->ToolInventory);
		// Equipped Tools
		for (UToolBase* Tool : OldTRPlayerController->EquippedTools)
		{
			if (Tool) {
				ServerEquipTool(Tool->ItemGuid);
			}
		}
		MaxEquippedWeapons = OldTRPlayerController->MaxEquippedWeapons;
		MaxEquippedEquipment = OldTRPlayerController->MaxEquippedEquipment;
		FactionId = OldTRPlayerController->FactionId;
		*/
	}
	else
	{
		UE_LOG(LogTRGame, Error, TEXT("TRPlayerControllerBase::SeamlessTravelFrom - Old PlayerController is not a valid TRPlayerController."))
	}
}


void ATRPlayerControllerBase::InitPlayerState()
{
	Super::InitPlayerState();	
}


void ATRPlayerControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	UpdateMovementFromAttributes();
}


FGenericTeamId ATRPlayerControllerBase::GetGenericTeamId() const
{
	return FactionId;
}


void ATRPlayerControllerBase::ClientLevelTemplatesChanged_Implementation()
{
	// Just call the BP hook
	OnLevelTemplatesChanged();
}


bool ATRPlayerControllerBase::ClientLevelTemplatesChanged_Validate()
{
	return true;
}


void ATRPlayerControllerBase::UpdateMovementFromAttributes_Implementation()
{
	APawn* TmpPawn = GetPawn();
	if (TmpPawn == nullptr) { return; }

	UCharacterMovementComponent* MoveComp = Cast<UCharacterMovementComponent>(TmpPawn->GetMovementComponent());
	if (MoveComp && RunSpeedAttribute && JumpForceAttribute)
	{
		MoveComp->MaxWalkSpeed = RunSpeedAttribute->GetCurrent();
		UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("Player walk speed changed to: %.0f"), MoveComp->MaxWalkSpeed);
		MoveComp->JumpZVelocity = JumpForceAttribute->GetCurrent();
		UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("Player jump velocity changed to: %.0f"), MoveComp->JumpZVelocity);
	}
}


bool ATRPlayerControllerBase::UpdateMovementFromAttributes_Validate()
{
	return true;
}


void ATRPlayerControllerBase::OnRunSpeedChanged(float NewSpeed)
{
	UpdateMovementFromAttributes();
}


void ATRPlayerControllerBase::OnJumpForceChanged(float NewJumpForce)
{
	UpdateMovementFromAttributes();
}


void ATRPlayerControllerBase::OnCollectionRangeChanged(float NewCollectionRange)
{
}


void ATRPlayerControllerBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATRPlayerControllerBase, CurrentTool);
	DOREPLIFETIME(ATRPlayerControllerBase, MaxEquippedWeapons);
	DOREPLIFETIME(ATRPlayerControllerBase, MaxEquippedEquipment);
}


void ATRPlayerControllerBase::OnRep_CurrentTool(AToolActorBase* OldTool)
{
	if (IsValid(OldTool) && OldTool != CurrentTool) {
		OldTool->Destroy();
	}
	OnCurrentToolChanged.Broadcast(CurrentTool);
}


void ATRPlayerControllerBase::ApplyAttributeModifiers_Implementation(const TArray<FAttributeModifier>& NewModifiers)
{
	TArray<UActorAttributeComponent*> AttributeComps;
	GetComponents<UActorAttributeComponent>(AttributeComps);
	for (UActorAttributeComponent* Attr : AttributeComps)
	{
		// AddModifiers will filter for modifiers relevant to the attribute
		Attr->AddModifiers(NewModifiers);
	}
	ATRPlayerState* TRPlayerState = GetPlayerState<ATRPlayerState>();
	if (TRPlayerState)
	{
		TRPlayerState->ApplyAttributeModifiers(NewModifiers);
	}
}


bool ATRPlayerControllerBase::ApplyAttributeModifiers_Validate(const TArray<FAttributeModifier>& NewModifiers)
{
	return true;
}


void ATRPlayerControllerBase::RemoveAttributeModifiers_Implementation(const TArray<FAttributeModifier>& ModifiersToRemove)
{
	TArray<UActorAttributeComponent*> AttributeComps;
	GetComponents<UActorAttributeComponent>(AttributeComps);
	for (UActorAttributeComponent* Attr : AttributeComps)
	{
		// RemoveModifiers will filter for modifiers relevant to the attribute
		Attr->RemoveModifiers(ModifiersToRemove);
	}
	ATRPlayerState* TRPlayerState = GetPlayerState<ATRPlayerState>();
	if (TRPlayerState)
	{
		TRPlayerState->RemoveAttributeModifiers(ModifiersToRemove);
	}
}


bool ATRPlayerControllerBase::RemoveAttributeModifiers_Validate(const TArray<FAttributeModifier>& ModifiersToRemove)
{
	return true;
}


bool ATRPlayerControllerBase::HasCapacityForAwards_Implementation(const FPickupAwards& PickupAwards)
{
	return true;
}

void ATRPlayerControllerBase::ServerAddToolToInventory_Implementation(const FToolData& ToolData)
{
	FToolData TmpToolData = ToolData;
	bool bSuccess = false;
	if (IsValid(ToolData.ToolClass))
	{		
		if (!TmpToolData.AttributeData.ItemGuid.IsValid()) { 
			TmpToolData.AttributeData.ItemGuid = FGuid::NewGuid(); 
		}
		ToolInventory.Add(TmpToolData.AttributeData.ItemGuid, TmpToolData);
		OnToolInventoryAdded.Broadcast(TmpToolData);
		bSuccess = true;
	}
	UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("TRPlayerControllerBase - ServerAddToolToInventory tried add tool guid %s. Success %s"), *TmpToolData.AttributeData.ItemGuid.ToString(), bSuccess ? *FString(TEXT("True")) : *FString(TEXT("False")));
	if (bSuccess && !IsLocalController())
	{
		//UE_LOG(LogTRGame, Log, TEXT("TRPlayerControllerBase - ServerAddToolToInventory calling client."));
		ClientAddToolToInventory(TmpToolData, TmpToolData.AttributeData.ItemGuid);
	}
}


bool ATRPlayerControllerBase::ServerAddToolToInventory_Validate(const FToolData& ToolData)
{
	return true;
}


void ATRPlayerControllerBase::ClientAddToolToInventory_Implementation(const FToolData& ToolData, const FGuid AddedGuid)
{
	if (IsValid(ToolData.ToolClass))
	{
		ToolInventory.Add(ToolData.AttributeData.ItemGuid, ToolData);
		OnToolInventoryAdded.Broadcast(ToolData);
	}
	UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("TRPlayerControllerBase - ClientAddToolToInventory added tool guid %s."), *ToolData.AttributeData.ItemGuid.ToString());
}

bool ATRPlayerControllerBase::ClientAddToolToInventory_Validate(const FToolData& ToolData, const FGuid AddedGuid)
{
	return true;
}


bool ATRPlayerControllerBase::EquipToolInternal(const FGuid ToolGuid, const bool bSuppressNotifications)
{
	FToolData* ToolData = ToolInventory.Find(ToolGuid);
	if (ToolData == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("TRPlayerControllerBase - EquipToolInternal tool guid %s not found in inventory."), *ToolGuid.ToString());
		return false;
	}
	for (UToolBase* ToolRef : EquippedTools)
	{
		// If tool is already equipped, we're done.
		if (ToolRef->ItemGuid == ToolGuid) {
			return true;
		}
	}
	UToolBase* TmpTool = UToolBase::CreateToolFromToolData(*ToolData, this);
	int CurrentEquipped = 0;
	int MaxEquipped = 0;
	if (TmpTool->IsA<UToolWeaponBase>())
	{
		// Unequip weapons until we have room to equip the requested one
		TArray<UToolWeaponBase*> TmpEquippedWeapons;
		GetEquippedWeapons(TmpEquippedWeapons);
		MaxEquipped = MaxEquippedWeapons;
		while (TmpEquippedWeapons.Num() > 0 && TmpEquippedWeapons.Num() >= MaxEquipped)
		{
			// Suppress notifications, we'll send one at the end
			UnequipToolInternal(TmpEquippedWeapons[0]->ItemGuid, true);
			TmpEquippedWeapons.RemoveAt(0, 1, false);
		}
		CurrentEquipped = TmpEquippedWeapons.Num();
	}
	else
	{
		// Unequip equipment until we have room to equip requested one
		TArray<UToolEquipmentBase*> TmpEquippedEquipment;
		GetEquippedEquipment(TmpEquippedEquipment);
		MaxEquipped = MaxEquippedEquipment;
		while (TmpEquippedEquipment.Num() > 0 && TmpEquippedEquipment.Num() >= MaxEquipped)
		{
			// Suppress notifications, we'll send one at the end
			UnequipToolInternal(TmpEquippedEquipment[0]->ItemGuid, true);
			TmpEquippedEquipment.RemoveAt(0);
		}
		CurrentEquipped = TmpEquippedEquipment.Num();
	}
	// If we found the tool and have room to equip it, do so
	if (TmpTool && CurrentEquipped < MaxEquipped) 
	{
		EquippedTools.Add(TmpTool);
		// Only need to apply modifiers on server side. They replicate themselves.
		if (GetLocalRole() == ROLE_Authority) {
			ApplyAttributeModifiers(TmpTool->GetEquipModifiers());
		}
		if (!bSuppressNotifications) {
			OnEquippedToolsChanged.Broadcast();
		}
		UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("EquipToolInternal - Equipped tool %s"), *TmpTool->ItemGuid.ToString(EGuidFormats::Digits));
		return true;
	}
	return false;
}


void ATRPlayerControllerBase::ServerEquipTool_Implementation(const FGuid ToolGuid)
{
	/*
	FToolData* ToolData = ToolInventory.Find(ToolGuid);
	if (ToolData == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("TRPlayerControllerBase - ServerEquipTool tool guid %s not found in inventory."), *ToolGuid.ToString());
		return;
	}
	UToolBase* TmpTool = UToolBase::CreateToolFromToolData(*ToolData, this);
	int CurrentEquipped = 0;
	int MaxEquipped = 0;
	if (TmpTool->IsA<UToolWeaponBase>())
	{
		TArray<UToolWeaponBase*> TmpWeapons;
		GetEquippedWeapons(TmpWeapons);
		MaxEquipped = MaxEquippedWeapons;
		while (TmpWeapons.Num() > 0 && TmpWeapons.Num() >= MaxEquipped)
		{
			ServerUnequipTool(TmpWeapons[0]->ItemGuid);
			TmpWeapons.RemoveAt(0);
		}
		CurrentEquipped = TmpWeapons.Num();
	}
	else 
	{
		TArray<UToolEquipmentBase*> TmpEquipment;
		GetEquippedEquipment(TmpEquipment);
		MaxEquipped = MaxEquippedEquipment;
		while (TmpEquipment.Num() > 0 && TmpEquipment.Num() >= MaxEquipped)
		{
			ServerUnequipTool(TmpEquipment[0]->ItemGuid);
			TmpEquipment.RemoveAt(0);
		}
		CurrentEquipped = TmpEquipment.Num();
	}
	if (TmpTool && CurrentEquipped < MaxEquipped) {
		EquippedTools.Add(TmpTool);
		ApplyAttributeModifiers(TmpTool->GetEquipModifiers());
		OnEquippedToolsChanged.Broadcast();
		//UE_LOG(LogTRGame, Log, TEXT("TRPlayerControllerBase - ServerEquipTool tool %s equipped."), *ToolData->AttributeData.ItemDisplayName.ToString());
		if (!IsLocalController())
		{
			//UE_LOG(LogTRGame, Log, TEXT("TRPlayerControllerBase - ServerEquipTool calling client rpc."));
			ClientEquipTool(ToolGuid);
		}
	}
	*/
	// If this is for a PC not on the server, i.e. a client player, update them.
	if (EquipToolInternal(ToolGuid) && !IsLocalController()) {
		ClientEquipTool(ToolGuid);
	}
}


bool ATRPlayerControllerBase::ServerEquipTool_Validate(const FGuid ToolGuid)
{
	return true;
}


void ATRPlayerControllerBase::ClientEquipTool_Implementation(const FGuid ToolGuid)
{
	/*
	FToolData* ToolData = ToolInventory.Find(ToolGuid);
	if (ToolData == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("TRPlayerControllerBase - ClientEquipTool tool guid %s not found in inventory."), *ToolGuid.ToString());
		return;
	}
	UToolBase* TmpTool = UToolBase::CreateToolFromToolData(*ToolData, this);
	int CurrentEquipped = 0;
	int MaxEquipped = 0;
	if (TmpTool->IsA<UToolWeaponBase>())
	{
		TArray<UToolWeaponBase*> TmpWeapons;
		GetEquippedWeapons(TmpWeapons);
		MaxEquipped = MaxEquippedWeapons;
		while (TmpWeapons.Num() > 0 && TmpWeapons.Num() >= MaxEquipped)
		{
			ServerUnequipTool(TmpWeapons[0]->ItemGuid);
			TmpWeapons.RemoveAt(0);
		}
		CurrentEquipped = TmpWeapons.Num();
	}
	else
	{
		TArray<UToolEquipmentBase*> TmpEquipment;
		GetEquippedEquipment(TmpEquipment);
		MaxEquipped = MaxEquippedEquipment;
		while (TmpEquipment.Num() > 0 && TmpEquipment.Num() >= MaxEquipped)
		{
			ServerUnequipTool(TmpEquipment[0]->ItemGuid);
			TmpEquipment.RemoveAt(0);
		}
		CurrentEquipped = TmpEquipment.Num();
	}
	if (TmpTool && CurrentEquipped < MaxEquipped) {
		EquippedTools.Add(TmpTool);
		// Only need to apply modifiers on server side.
		OnEquippedToolsChanged.Broadcast();
		//UE_LOG(LogTRGame, Log, TEXT("TRPlayerControllerBase - ClientEquipTool tool %s equipped."), *ToolData->AttributeData.ItemDisplayName.ToString());
	}
	*/
	EquipToolInternal(ToolGuid);
}


bool ATRPlayerControllerBase::ClientEquipTool_Validate(const FGuid ToolGuid)
{
	return true;
}


bool ATRPlayerControllerBase::UnequipToolInternal(const FGuid ToolGuid, const bool bSuppressNotifications)
{
	UToolBase* FoundTool = nullptr;
	for (UToolBase* TmpTool : EquippedTools)
	{
		if (TmpTool->ItemGuid == ToolGuid)
		{
			FoundTool = TmpTool;
			break;
		}
	}
	if (FoundTool)
	{
		if (CurrentTool && CurrentTool->Tool->ItemGuid == FoundTool->ItemGuid) 
		{
			CurrentTool->Destroy();
			CurrentTool = nullptr;
			if (GetLocalRole() == ROLE_Authority) {
				OnRep_CurrentTool(CurrentTool);
			}
		}
		EquippedTools.Remove(FoundTool);
		// Only need to adjust attributes on server. They replicate.
		if (GetLocalRole() == ROLE_Authority) {
			RemoveAttributeModifiers(FoundTool->GetEquipModifiers());
		}
		if (!bSuppressNotifications) {
			OnEquippedToolsChanged.Broadcast();
		}
		UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("UnequipToolInternal - Unequipped tool %s"), *FoundTool->ItemGuid.ToString(EGuidFormats::Digits));
		return true;
	}
	// Tool was already unequipped
	return false;
}


void ATRPlayerControllerBase::ServerUnequipTool_Implementation(const FGuid ToolGuid)
{
	/*
	UToolBase* FoundTool = nullptr;
	for (UToolBase* TmpTool : EquippedTools)
	{
		if (TmpTool->ItemGuid == ToolGuid)
		{
			FoundTool = TmpTool;
			break;
		}
	}
	if (FoundTool)
	{
		EquippedTools.Remove(FoundTool);
		RemoveAttributeModifiers(FoundTool->GetEquipModifiers());
		OnEquippedToolsChanged.Broadcast();
		if (!IsLocalController())
		{
			ClientUnequipTool(ToolGuid);
		}
	}
	*/
	UnequipToolInternal(ToolGuid);
	if (!IsLocalController()) {
		ClientUnequipTool(ToolGuid);
	}
}


bool ATRPlayerControllerBase::ServerUnequipTool_Validate(const FGuid ToolGuid)
{
	return true;
}


void ATRPlayerControllerBase::ClientUnequipTool_Implementation(const FGuid ToolGuid)
{
	/*
	UToolBase* FoundTool = nullptr;
	for (UToolBase* TmpTool : EquippedTools)
	{
		if (TmpTool->ItemGuid == ToolGuid)
		{
			FoundTool = TmpTool;
			break;
		}
	}
	if (FoundTool)
	{
		EquippedTools.Remove(FoundTool);
		// Only remove modifiers on server side
		OnEquippedToolsChanged.Broadcast();
	}
	*/
	UnequipToolInternal(ToolGuid);
}


bool ATRPlayerControllerBase::ClientUnequipTool_Validate(const FGuid ToolGuid)
{
	return true;
}


void ATRPlayerControllerBase::UnequipAllToolsInternal(const bool bSuppressNotifications)
{
	bool bChanged = EquippedTools.Num() > 0;
	while (EquippedTools.Num() > 0)
	{
		UToolBase* TmpTool = EquippedTools.Last();
		if (TmpTool) {
			UnequipToolInternal(TmpTool->ItemGuid, true);
		}
	}
	if (bChanged && !bSuppressNotifications) {
		OnEquippedToolsChanged.Broadcast();
	}
}


void ATRPlayerControllerBase::ServerUnequipAllTools_Implementation()
{
	/*
	bool bChanged = false;
	for (UToolBase* TmpTool : EquippedTools)
	{
		if (TmpTool)
		{
			RemoveAttributeModifiers(TmpTool->GetEquipModifiers());
			bChanged = true;
		}
	}
	EquippedTools.Empty();
	if (bChanged)
	{
		OnEquippedToolsChanged.Broadcast();
		if (!IsLocalController())
		{
			ClientUnequipAllTools();
		}
	}
	*/
	UnequipAllToolsInternal();
	if (!IsLocalController()) {
		ClientUnequipAllTools();
	}
}


bool ATRPlayerControllerBase::ServerUnequipAllTools_Validate()
{
	return true;
}


void ATRPlayerControllerBase::ClientUnequipAllTools_Implementation()
{
	/*UToolBase* FoundTool = nullptr;
	bool bChanged = EquippedTools.Num() > 0;
	EquippedTools.Empty();
	if (bChanged)
	{
		OnEquippedToolsChanged.Broadcast();
	}*/
	UnequipAllToolsInternal();
}


bool ATRPlayerControllerBase::ClientUnequipAllTools_Validate()
{
	return true;
}


void ATRPlayerControllerBase::ServerRenameTool_Implementation(const FGuid& ToolGuid, const FText& NewName)
{
	FToolData* ToolData = ToolInventory.Find(ToolGuid);
	UToolBase* FoundTool = nullptr;
	if (ToolData == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("TRPlayerControllerBase - ServerRenameTool tool guid %s not found in inventory."), *ToolGuid.ToString());
		return;
	}
	for (UToolBase* TmpTool : EquippedTools)
	{
		if (TmpTool->ItemGuid == ToolGuid)
		{
			FoundTool = TmpTool;
			break;
		}
	}
	if (FoundTool == nullptr)
	{
		FoundTool = UToolBase::CreateToolFromToolData(*ToolData, this);
	}
	if (FoundTool && !FoundTool->DisplayName.EqualTo(NewName, ETextComparisonLevel::Default))
	{
		FToolData TmpToolData;
		FoundTool->DisplayName = NewName;
		FoundTool->ToToolData(TmpToolData);
		ToolInventory.Add(FoundTool->ItemGuid, TmpToolData);
		OnEquippedToolsChanged.Broadcast();
		if (!IsLocalController())
		{
			//UE_LOG(LogTRGame, Log, TEXT("TRPlayerControllerBase - ServerRenameTool calling client rpc."));
			ClientRenameTool(ToolGuid, NewName);
		}
	}
}

bool ATRPlayerControllerBase::ServerRenameTool_Validate(const FGuid& ToolGuid, const FText& NewName)
{
	return true;
}


void ATRPlayerControllerBase::ClientRenameTool_Implementation(const FGuid& ToolGuid, const FText& NewName)
{
	FToolData* ToolData = ToolInventory.Find(ToolGuid);
	UToolBase* FoundTool = nullptr;
	if (ToolData == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("TRPlayerControllerBase - ClientRenameTool tool guid %s not found in inventory."), *ToolGuid.ToString());
		return;
	}
	for (UToolBase* TmpTool : EquippedTools)
	{
		if (TmpTool->ItemGuid == ToolGuid)
		{
			FoundTool = TmpTool;
			break;
		}
	}
	if (FoundTool == nullptr) {
		FoundTool = UToolBase::CreateToolFromToolData(*ToolData, this);
	}
	if (FoundTool && !FoundTool->DisplayName.EqualTo(NewName, ETextComparisonLevel::Default))
	{
		FToolData TmpToolData;
		FoundTool->DisplayName = NewName;
		FoundTool->ToToolData(TmpToolData);
		ToolInventory.Add(FoundTool->ItemGuid, TmpToolData);
		OnEquippedToolsChanged.Broadcast();
	}
}

bool ATRPlayerControllerBase::ClientRenameTool_Validate(const FGuid& ToolGuid, const FText& NewName)
{
	return true;
}


void ATRPlayerControllerBase::ServerUpgradeTool_Implementation(const FGuid ToolGuid, const ETRToolUpgrade UpgradeType, const FResourceRateFilter RateDelta)
{
	FToolData* ToolData = ToolInventory.Find(ToolGuid);
	UToolBase* FoundTool = nullptr;
	FResourceRateFilter* FoundRate = nullptr;
	bool bWasEquipped = false;
	if (ToolData == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("TRPlayerControllerBase - ServerUpgradeTool tool guid %s not found in inventory."), *ToolGuid.ToString());
		return;
	}
	for (UToolBase* TmpTool : EquippedTools)
	{
		if (TmpTool->ItemGuid == ToolGuid)
		{
			FoundTool = TmpTool;
			bWasEquipped = true;
			break;
		}
	}
	if (FoundTool == nullptr) {
		FoundTool = UToolBase::CreateToolFromToolData(*ToolData, this);
	}
	if (UpgradeType == ETRToolUpgrade::DamageRate)
	{
		FoundRate = FoundTool->BaseDamageRates.FindByPredicate([RateDelta](FResourceRateFilter DamageRate) {
			return DamageRate.ResourceTypeFilter == RateDelta.ResourceTypeFilter;
		});
	} 
	else if (UpgradeType == ETRToolUpgrade::ExtractionRate)
	{
		FoundRate = FoundTool->BaseResourceExtractionRates.FindByPredicate([RateDelta](FResourceRateFilter DamageRate) {
			return DamageRate.ResourceTypeFilter == RateDelta.ResourceTypeFilter;
		});
	}
	if (FoundRate)
	{
		FoundRate->Rate += RateDelta.Rate;
		UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("TRPlayerControllerBase - ServerUpgradeTool tool %s upgraded."), *ToolData->AttributeData.ItemDisplayName.ToString());
	}
	FToolData TmpToolData;
	FoundTool->ToToolData(TmpToolData);
	ToolInventory.Add(FoundTool->ItemGuid, TmpToolData);
	if (bWasEquipped) { 
		//OnEquippedToolsChanged.Broadcast(); 
	}
	if (!IsLocalController())
	{
		//UE_LOG(LogTRGame, Log, TEXT("TRPlayerControllerBase - ServerUpgradeTool calling client rpc."));
		ClientUpgradeTool(ToolGuid, UpgradeType, RateDelta);
	}
}

bool ATRPlayerControllerBase::ServerUpgradeTool_Validate(const FGuid ToolGuid, const ETRToolUpgrade UpgradeType, const FResourceRateFilter RateDelta)
{
	return true;
}


void ATRPlayerControllerBase::ClientUpgradeTool_Implementation(const FGuid ToolGuid, const ETRToolUpgrade UpgradeType, const FResourceRateFilter RateDelta)
{
	FToolData* ToolData = ToolInventory.Find(ToolGuid);
	UToolBase* FoundTool = nullptr;
	FResourceRateFilter* FoundRate = nullptr;
	bool bWasEquipped = false;
	if (ToolData == nullptr)
	{
		UE_LOG(LogTRGame, Error, TEXT("TRPlayerControllerBase - ClientUpgradeTool tool guid %s not found in inventory."), *ToolGuid.ToString());
		return;
	}
	for (UToolBase* TmpTool : EquippedTools)
	{
		if (TmpTool->ItemGuid == ToolGuid)
		{
			FoundTool = TmpTool;
			bWasEquipped = true;
			break;
		}
	}
	if (FoundTool == nullptr) {
		FoundTool = UToolBase::CreateToolFromToolData(*ToolData, this);
	}
	if (UpgradeType == ETRToolUpgrade::DamageRate)
	{
		FoundRate = FoundTool->BaseDamageRates.FindByPredicate([RateDelta](FResourceRateFilter DamageRate) {
			return DamageRate.ResourceTypeFilter == RateDelta.ResourceTypeFilter;
		});
	}
	else if (UpgradeType == ETRToolUpgrade::ExtractionRate)
	{
		FoundRate = FoundTool->BaseResourceExtractionRates.FindByPredicate([RateDelta](FResourceRateFilter DamageRate) {
			return DamageRate.ResourceTypeFilter == RateDelta.ResourceTypeFilter;
		});
	}
	if (FoundRate)
	{
		FoundRate->Rate += RateDelta.Rate;
		UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("TRPlayerControllerBase - ClientUpgradeTool tool %s upgraded."), *ToolData->AttributeData.ItemDisplayName.ToString());
	}
	FToolData TmpToolData;
	FoundTool->ToToolData(TmpToolData);
	ToolInventory.Add(FoundTool->ItemGuid, TmpToolData);
	if (bWasEquipped) {
		//OnEquippedToolsChanged.Broadcast();
	}
}


bool ATRPlayerControllerBase::ClientUpgradeTool_Validate(const FGuid ToolGuid, const ETRToolUpgrade UpgradeType, const FResourceRateFilter RateDelta)
{
	return true;
}


void ATRPlayerControllerBase::ServerSetCurrentTool_Implementation(const FGuid ToolGuid)
{
	UToolBase* FoundTool = nullptr;
	for (UToolBase* TmpTool : EquippedTools)
	{
		if (TmpTool->ItemGuid == ToolGuid)
		{
			FoundTool = TmpTool;
			break;
		}
	}
	if (FoundTool) {
		SpawnAsCurrentTool(FoundTool);
	}
	else {
		UE_LOG(LogTRGame, Error, TEXT("TRPlayerController::ServerSetCurrentTool - Tool guid: %s not found in equipped tools."), *ToolGuid.ToString(EGuidFormats::Digits));
	}
}


bool ATRPlayerControllerBase::ServerSetCurrentTool_Validate(const FGuid ToolGuid)
{
	return true;
}


void ATRPlayerControllerBase::ServerAddLevelUpGoods_Implementation(const FGoodsQuantitySet& ContributedGoods)
{
	if (GetWorld() == nullptr) { return; }
	ATRGameModeLobby* TRGameMode = GetWorld()->GetAuthGameMode<ATRGameModeLobby>();
	ATRPlayerState* TRPlayerState = GetPlayerState<ATRPlayerState>();

	if (TRGameMode && TRPlayerState)
	{
		FPlayerLevelUpData LevelUpData;
		// Add the goods quantities to player progress.
		TRPlayerState->LevelUpGoodsProgress.GoodsQuantitySet.Goods = UGoodsFunctionLibrary::AddGoodsQuantities(TRPlayerState->LevelUpGoodsProgress.GoodsQuantitySet.Goods, ContributedGoods.Goods);
		// Check for level up
		if (TRGameMode->GetLevelUpDataForPlayer(this, LevelUpData))
		{
			bool bHasLevelUpReqs = true;
			TMap<FName, FGoodsQuantity> CurrentProgress = UGoodsFunctionLibrary::GoodsQuantityArrayToMap(TRPlayerState->LevelUpGoodsProgress.GoodsQuantitySet.Goods);
			for (FGoodsQuantity RequiredGoods : LevelUpData.GoodsRequired.Goods)
			{
				if (CurrentProgress.Contains(RequiredGoods.Name))
				{
					if (CurrentProgress[RequiredGoods.Name].Quantity < RequiredGoods.Quantity)
					{
						bHasLevelUpReqs = false;
						break;
					}
				}
				else
				{
					bHasLevelUpReqs = false;
					break;
				}
			}
			// If ready for level up, perform the level up.
			if (bHasLevelUpReqs)
			{
				// PlayerController changes from level up.
				if (LevelUpData.AddEquippedWeapons > 0) {
					MaxEquippedWeapons += LevelUpData.AddEquippedWeapons;
				}
				if (LevelUpData.AddEquippedEquipment > 0) {
					MaxEquippedEquipment += LevelUpData.AddEquippedEquipment;
				}
				// Player state changes from level up
				// TODO: Move this to TRPlayerState function
				TRPlayerState->ExperienceLevel = LevelUpData.Level;
				if (LevelUpData.AddMaxAnimus != 0.f) {
					TRPlayerState->AnimusAttribute->SetMaxBase(TRPlayerState->AnimusAttribute->GetMaxBase() + LevelUpData.AddMaxAnimus);
				}
				if (LevelUpData.AddMaxEnergy != 0.f) {
					TRPlayerState->EnergyAttribute->SetMaxBase(TRPlayerState->EnergyAttribute->GetMaxBase() + LevelUpData.AddMaxEnergy);
				}
				if (LevelUpData.AddMaxHealth != 0.f) {
					TRPlayerState->HealthAttribute->SetMaxBase(TRPlayerState->HealthAttribute->GetMaxBase() + LevelUpData.AddMaxHealth);
				}
				//  Add level up goods and tools
				if (LevelUpData.GoodsAwarded.Goods.Num() > 0)
				{
					TArray<FGoodsQuantity> CurrentQuant;
					GoodsInventory->AddSubtractGoodsArray(LevelUpData.GoodsAwarded.Goods, false , CurrentQuant);
				}
				if (LevelUpData.ToolsAwarded.Num() > 0)
				{
					for (TSubclassOf<UToolBase> NewToolClass : LevelUpData.ToolsAwarded)
					{
						UToolBase* NewTool = NewToolClass.GetDefaultObject();
						if (NewTool) {
							FToolData NewToolData;
							NewTool->ToToolData(NewToolData);
							ServerAddToolToInventory(NewToolData);
						}
					}
				}
				// Subtract the level up goods from the player's level up progress
				TRPlayerState->LevelUpGoodsProgress.GoodsQuantitySet.Goods = UGoodsFunctionLibrary::AddGoodsQuantities(TRPlayerState->LevelUpGoodsProgress.GoodsQuantitySet.Goods, LevelUpData.GoodsRequired.Goods, true);
				// Save updated player data
				PersistentDataComponent->ServerSavePlayerData();				
				// Call delegate here on server
				OnPlayerLevelUp.Broadcast(LevelUpData);
				// Notify client
				ClientOnPlayerLevelUp(LevelUpData);
			}
		}
	}
}

bool ATRPlayerControllerBase::ServerAddLevelUpGoods_Validate(const FGoodsQuantitySet& ContributedGoods)
{
	return true;
}


void ATRPlayerControllerBase::ClientOnPlayerLevelUp_Implementation(const FPlayerLevelUpData& LevelUpData)
{
	if (GetLocalRole() < ROLE_Authority) 
	{
		// Base class just calls OnPlayerLevelUp event
		OnPlayerLevelUp.Broadcast(LevelUpData);
	}
}

bool ATRPlayerControllerBase::ClientOnPlayerLevelUp_Validate(const FPlayerLevelUpData& LevelUpData)
{
	return true;
}


void ATRPlayerControllerBase::SpawnAsCurrentTool_Implementation(UToolBase* NewCurrentTool)
{
	FTransform SpawnTransform;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	if (!IsValid(NewCurrentTool->ToolActorClass))
	{
		UE_LOG(LogTRGame, Warning, TEXT("SpawnAsCurrentTool tool class is not valid."));
		return;
	}
	if (NewCurrentTool && IsValid(NewCurrentTool))
	{
		if (CurrentTool) {
			CurrentTool->Destroy();
		}
		APawn* CurPawn = GetPawn();
		CurrentTool = nullptr;
		if (CurPawn) 
		{
			SpawnParams.Owner = CurPawn;
			SpawnParams.Instigator = CurPawn;
			SpawnTransform = CurPawn->GetTransform();
			CurrentTool = GetWorld()->SpawnActorDeferred<AToolActorBase>(NewCurrentTool->ToolActorClass, SpawnTransform, SpawnParams.Owner, SpawnParams.Instigator, SpawnParams.SpawnCollisionHandlingOverride);
			if (CurrentTool)
			{
				CurrentTool->Tool = NewCurrentTool;
				CurrentTool->WeaponState = ETRWeaponState::Idle;
				CurrentTool->SetOwner(CurPawn);
				UGameplayStatics::FinishSpawningActor(CurrentTool, CurrentTool->GetTransform());
				NewCurrentTool->ToToolData(CurrentTool->ToolData);
			}
		}
		// Manually call rep_notify on server
		if (GetLocalRole() == ROLE_Authority) { 
			OnRep_CurrentTool(CurrentTool); 
		}
	}
}


void ATRPlayerControllerBase::GetEquippedWeapons(TArray<UToolWeaponBase*>& EquippedWeapons)
{
	UToolWeaponBase* TmpWeapon;
	EquippedWeapons.Empty();
	for (UToolBase* TmpTool : EquippedTools)
	{
		TmpWeapon = Cast<UToolWeaponBase>(TmpTool);
		if (TmpWeapon) {
			EquippedWeapons.Add(TmpWeapon);
		}
	}
}


void ATRPlayerControllerBase::GetEquippedEquipment(TArray<UToolEquipmentBase*>& EquippedEquipment)
{
	UToolEquipmentBase* TmpEquipment;
	EquippedEquipment.Empty();
	for (UToolBase* TmpTool : EquippedTools)
	{
		TmpEquipment = Cast<UToolEquipmentBase>(TmpTool);
		if (TmpEquipment) {
			EquippedEquipment.Add(TmpEquipment);
		}
	}
}


ARoomPlatformGridMgr* ATRPlayerControllerBase::FindGridManager()
{
	ARoomPlatformGridMgr* GridManager = nullptr;
	TArray<AActor*> FoundManagers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARoomPlatformGridMgr::StaticClass(), FoundManagers);
	if (FoundManagers.Num() > 0) {
		GridManager = Cast<ARoomPlatformGridMgr>(FoundManagers[0]);
	}
	return GridManager;
}


void ATRPlayerControllerBase::ClientUpdateRoomGridTemplate_Implementation(const FRoomGridTemplate& UpdatedTemplate, const TArray<FVector2D>& RoomCoords, const TArray<FRoomTemplate>& RoomTemplates)
{
	ARoomPlatformGridMgr* GridManager = FindGridManager();
	if (GridManager != nullptr)
	{
		GridManager->DestroyGrid();
		GridManager->SetRoomGridTemplateData(UpdatedTemplate, RoomCoords, RoomTemplates);
		//GridManager->SpawnRooms();
	}
	else {
		UE_LOG(LogTRGame, Warning, TEXT("ClientUpdateRoomGridTemplate - No Grid mgr found."))
	}
}


bool ATRPlayerControllerBase::GetPlayerSaveData_Implementation(FPlayerSaveData& SaveData, const bool bAllowNullPlayerState)
{
	// Copy empty default struct in first in case this save already has data.
	SaveData = FPlayerSaveData();
	// Attribute components
	TArray<UActorAttributeComponent*> AttributeComps;
	GetComponents<UActorAttributeComponent>(AttributeComps);
	for (UActorAttributeComponent* Attr : AttributeComps) {
		Attr->FillAttributeDataArray(SaveData.AttributeData.Attributes);
	}
	// Inventory
	GoodsInventory->GetSaveableGoods(SaveData.GoodsInventory);
	GoodsInventory->GetSnapshotGoods(SaveData.SnapshotInventory);
	// Tools & equipped tools
	ToolInventory.GenerateValueArray(SaveData.ToolInventory);
	SaveData.LastEquippedItems.Empty(EquippedTools.Num());
	for (UToolBase* TmpTool : EquippedTools) {
		SaveData.LastEquippedItems.Add(TmpTool->ItemGuid);
	}
	// Other data
	SaveData.AttributeData.IntAttributes.Add(FTRNamedInt(FName(TEXT("MaxEquippedWeapons")), MaxEquippedWeapons));
	SaveData.AttributeData.IntAttributes.Add(FTRNamedInt(FName(TEXT("MaxEquippedEquipment")), MaxEquippedEquipment));
	// Get data from PlayerState
	ATRPlayerState* TRPlayerState = Cast<ATRPlayerState>(PlayerState);
	if (TRPlayerState != nullptr) {
		TRPlayerState->GetPlayerSaveData(SaveData);
	}
	else if(!bAllowNullPlayerState)
	{
		UE_LOG(LogTRGame, Error, TEXT("GetPlayerSaveData -  Player state not found."));
		return false;
	}	
	return true;
}


bool ATRPlayerControllerBase::UpdateFromPlayerSaveData_Implementation(const FPlayerSaveData& SaveData)
{
	UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("PlayerControllerBase UpdateFromPlayerData start: GUID %s."), *SaveData.PlayerGuid.ToString(EGuidFormats::Digits));
	UToolBase* TmpTool = nullptr;
	// Attribute components
	TArray<UActorAttributeComponent*> AttributeComps;
	GetComponents<UActorAttributeComponent>(AttributeComps);
	for (UActorAttributeComponent* Attr : AttributeComps)
	{
		Attr->UpdateFromAttributeDataArray(SaveData.AttributeData.Attributes);
	}
	//if (GetLocalRole() == ROLE_Authority) {
	//	// Inventory
	//	GoodsInventory->ServerSetInventory(SaveData.GoodsInventory, SaveData.SnapshotInventory);
	//}
	// Update local inventory
	GoodsInventory->SetInventoryInternal(SaveData.GoodsInventory, SaveData.SnapshotInventory);
	if (bEnableClassDebug)
	{
		UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("Inventory from save:"));
		for (FGoodsQuantity Goods : SaveData.GoodsInventory) {
			UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("    %s %d"), *Goods.Name.ToString(), (int32)Goods.Quantity);
		}
	}
	// Tools
	ToolInventory.Empty();
	for (FToolData CurToolData : SaveData.ToolInventory)
	{
		UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("    Adding tool from save %s %s"), *CurToolData.AttributeData.ItemDisplayName.ToString(), *CurToolData.AttributeData.ItemGuid.ToString(EGuidFormats::Digits));
		ToolInventory.Add(CurToolData.AttributeData.ItemGuid, CurToolData);
	}
	// Other data
	const FTRNamedInt* TmpInt;
	TmpInt = FindInNamedArray<FTRNamedInt>(SaveData.AttributeData.IntAttributes, FName(TEXT("MaxEquippedWeapons")));
	if (TmpInt) {
		MaxEquippedWeapons = TmpInt->Quantity;
	}
	MaxEquippedWeapons = MaxEquippedWeapons < 1 ? 1 : MaxEquippedWeapons;
	TmpInt = FindInNamedArray<FTRNamedInt>(SaveData.AttributeData.IntAttributes, FName(TEXT("MaxEquippedEquipment")));
	if (TmpInt) {
		MaxEquippedEquipment = TmpInt->Quantity;
	}
	MaxEquippedEquipment = MaxEquippedEquipment < 0 ? 0 : MaxEquippedEquipment;
	//if (GetLocalRole() == ROLE_Authority)
	//{
	//	// Re-equip all tools
	//	ServerUnequipAllTools();
	//	for (FGuid TmpGuid : SaveData.LastEquippedItems)
	//	{
	//		ServerEquipTool(TmpGuid);
	//	}
	//}
	// Equip only the tools from save data
	UnequipAllToolsInternal(true);
	for (FGuid TmpGuid : SaveData.LastEquippedItems) 
	{
		UE_CLOG(bEnableClassDebug, LogTRGame, Log, TEXT("    Equipping Tool from save GUID %s"), *TmpGuid.ToString(EGuidFormats::Digits));
		EquipToolInternal(TmpGuid, true);
	}
	OnEquippedToolsChanged.Broadcast();
	// Update PlayerState
	if (SaveData.PlayerGuid.IsValid())
	{
		ATRPlayerState* TRPlayerState = Cast<ATRPlayerState>(PlayerState);
		if (TRPlayerState)
		{
			TRPlayerState->UpdateFromPlayerSaveData(SaveData);
			return true;
		}
		else
		{
			UE_LOG(LogTRGame, Error, TEXT("UpdateFromPlayerSaveData - Player state not found."));
			return false;
		}
	}
	else {
		UE_LOG(LogTRGame, Warning, TEXT("UpdateFromPlayerSaveData - Player GUID in save file not valid. Not updating Player State."));
	}
	return true;
}

