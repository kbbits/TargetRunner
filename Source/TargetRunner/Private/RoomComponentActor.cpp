
#include "RoomComponentActor.h"
#include "TargetRunner.h"
#include "Kismet/GameplayStatics.h"

ARoomComponentActor::ARoomComponentActor()
{
	bReplicates = false;
	PrimaryActorTick.bCanEverTick = false;
	bMyISMsCopiedOut = true;
	bAllChildISMsCopiedOut = false;
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);
#if WITH_EDITORONLY_DATA
	UBlueprint* Blueprint = Cast<UBlueprint>(GetClass()->ClassGeneratedBy);
	if (Blueprint) {
		Blueprint->bRunConstructionScriptOnDrag = false;
	}
#endif
}


bool ARoomComponentActor::FilterRCACollections()
{
	bool bRemovedAny = false;
	TArray<FRoomComponentActorCollectionItem> InvalidItems;
	for (FRoomComponentActorCollection RCACollection : SubRoomComponents)
	{
		InvalidItems.Empty();
		for (FRoomComponentActorCollectionItem RCAItem : RCACollection.RoomComponentActors)
		{
			if (!CanContainRoomComponentClass(RCAItem.RCAClass)) {
				InvalidItems.Add(RCAItem);
			}
		}
		for (FRoomComponentActorCollectionItem RCAItem : InvalidItems)
		{
			RCACollection.RoomComponentActors.Remove(RCAItem);
			bRemovedAny = true;
		}
	}
	if (bRemovedAny) {
		UE_LOG(LogTRGame, Warning, TEXT("RoomComponentActor FilterRCACollections removed items."))
	}
	return bRemovedAny;
}


bool ARoomComponentActor::SpawnRCACollections()
{
	if (SpawnedCollections.Num() > 0) {
		UE_LOG(LogTRGame, Log, TEXT("RoomComponentActor %s SpawnRCACollections - %d collections already spawned"), *GetName(), SpawnedCollections.Num());
		return true;
	}
	if (!GetWorld()) {
		return false;
	}
	FilterRCACollections();
	if (SubRoomComponents.Num() > 0) {
		UE_LOG(LogTRGame, Log, TEXT("RoomComponentActor %s SpawnRCACollections spawning %d collections"), *GetName(), SubRoomComponents.Num());
	}
	int32 Count = 0;
	ARoomComponentActorCollectionActor* CollectionActor = nullptr;
	FTransform SpawnTransform;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = this;
	/*
	if (GetOwner()) {
		SpawnParams.Owner = GetOwner();
	}
	else 
	{
		if (GetParentActor())
		{
			if (GetParentActor()->GetOwner()) {
				SpawnParams.Owner = GetParentActor()->GetOwner();
			}
			else {
				SpawnParams.Owner = GetParentActor();
			}
		}
	}
	*/
	// Spawn a collection actor for each in our collection
	for (FRoomComponentActorCollection RCACollection : SubRoomComponents)
	{
		Count++;
		if (RCACollection.RoomComponentActors.Num() == 0) 
		{
			UE_LOG(LogTRGame, Warning, TEXT("RoomComponentActor %s skipping collection because it contains no RCA entries."), *GetName());
			continue;
		}
		//SpawnTransform = RCACollection.LocalTransform * GetTransform();
		SpawnTransform = RCACollection.LocalTransform * GetTransform();
		UE_LOG(LogTRGame, Log, TEXT("    RoomComponentActor %s start spawning collection %d (contains %d RCAs)"), *GetName(), Count, RCACollection.RoomComponentActors.Num());
		CollectionActor = GetWorld()->SpawnActorDeferred<ARoomComponentActorCollectionActor>(ARoomComponentActorCollectionActor::StaticClass(), SpawnTransform, SpawnParams.Owner, SpawnParams.Instigator, SpawnParams.SpawnCollisionHandlingOverride);
		if (CollectionActor) 
		{
			UE_LOG(LogTRGame, Log, TEXT("RoomComponentActor setting RCACollection owner to %s"), *SpawnParams.Owner->GetName());
			CollectionActor->SetOwner(SpawnParams.Owner);
			CollectionActor->RoomComponentActorCollection = RCACollection;
			CollectionActor->RandSeed = RandStream.RandRange(1, INT_MAX - 1);
			//UE_LOG(LogTRGame, Log, TEXT("    RoomComponentActor %s finish spawning collection %s (contains %d RCAs) with seed %d"), *GetName(), *CollectionActor->GetName(), CollectionActor->RoomComponentActorCollection.RoomComponentActors.Num(),  CollectionActor->RandSeed);
			UGameplayStatics::FinishSpawningActor(CollectionActor, CollectionActor->GetTransform());
			CollectionActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform); // FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			UE_LOG(LogTRGame, Log, TEXT("    RoomComponentActor %s finished spawning collection %s"), *GetName(), *CollectionActor->GetName());
			SpawnedCollections.Add(CollectionActor);
		}
	}
	return SpawnedCollections.Num() > 0;
}


void ARoomComponentActor::PostInitProperties()
{
	Super::PostInitProperties();
	//UE_LOG(LogTRGame, Log, TEXT("RoomComponentActor PostInitProperties. RandSeed: %d Location: %s"), RandSeed, *GetActorLocation().ToString());
}


#if WITH_EDITOR
void ARoomComponentActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (RandSeed > 0 && RandStream.GetInitialSeed() != RandSeed) {
		RandStream.Initialize(RandSeed);
	}
	FilterRCACollections();
}
#endif


void ARoomComponentActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	AActor* Parent = GetParentActor();
	if (Parent == nullptr) {
		Parent = GetAttachParentActor();
	}
	if (Parent && RandSeed == 0)
	{
		// If we have a perent collection actor, get seed from them.
		ARoomComponentActorCollectionActor* CollectionActor = Cast<ARoomComponentActorCollectionActor>(Parent);
		if (CollectionActor) {
			RandSeed = CollectionActor->GetRandSeedForChild();
		}
	}
	if (RandSeed > 0 && RandStream.GetInitialSeed() != RandSeed) {
		RandStream.Initialize(RandSeed);
	}
	//UE_LOG(LogTRGame, Log, TEXT("RoomComponentActor %s PostInitializeComponents. RandSeed: %d Location: %s"), *GetName(), RandSeed, *GetActorLocation().ToString());
	SpawnRCACollections();	
}

void ARoomComponentActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	AActor* Parent = GetParentActor();
	if (Parent == nullptr) {
		Parent = GetAttachParentActor();
	}
	if (Parent && RandSeed == 0)
	{
		// If we have a parent collection actor, get seed from them.
		ARoomComponentActorCollectionActor* CollectionActor = Cast<ARoomComponentActorCollectionActor>(Parent);
		if (CollectionActor) {
			RandSeed = CollectionActor->GetRandSeedForChild();
		}
	}
	if (RandSeed > 0 && RandStream.GetInitialSeed() != RandSeed) {
		RandStream.Initialize(RandSeed);
	}
	//UE_LOG(LogTRGame, Log, TEXT("RoomComponentActor %s OnConstruction. RandSeed: %d Location: %s"), *GetName(), RandSeed, *GetActorLocation().ToString());
	if (GetWorld()->WorldType == EWorldType::Game) {
		SpawnRCACollections();
	}
	else {
		RespawnRCACollections();
	}
}


void ARoomComponentActor::BeginPlay()
{
	Super::BeginPlay();
}


void ARoomComponentActor::Destroyed()
{
	/*
	if (!GetWorld() || (GetWorld() && GetWorld()->WorldType != EWorldType::Game)) {
		AActor* Parent = GetParentActor();
		if (Parent)
		{
			// If we were created by a collection actor and it is still valid, destroy the parent collection when we get destroyed.
			ARoomComponentActorCollectionActor* CollectionActor = Cast<ARoomComponentActorCollectionActor>(Parent);
			if (IsValid(CollectionActor)) {
				CollectionActor->Destroy();
				UE_LOG(LogTRGame, Log, TEXT("RoomComponentActor %s destroyed parent collection actor %s"), *GetName(), *CollectionActor->GetName());
			}
		}
		//UE_LOG(LogTRGame, Log, TEXT("RoomComponentActor %s Destroyed, destroying %d spawned collections."), *GetName(), SpawnedCollections.Num());
		DestroyRCACollections();
	}
	else {
		// If in game world don't destroy them. They will destroy themselves.
		SpawnedCollections.Empty();
		//UE_LOG(LogTRGame, Log, TEXT("RoomComponentActor %s Destroyed, NOT destroying spawned collections."), *GetName(), SpawnedCollections.Num());
	}
	*/
	DestroyRCACollections();
	Super::Destroyed();
}


bool ARoomComponentActor::CanContainRoomComponentClass(const TSubclassOf<ARoomComponentActor> ContainClass)
{
	if (ContainClass == ARoomComponentActor::StaticClass()) {
		UE_LOG(LogTRGame, Warning, TEXT("RoomComponentActor sub collection cannot contain RoomComponentActor base class."));
		return false;
	}
	if (ContainClass == GetClass()) {
		UE_LOG(LogTRGame, Warning, TEXT("RoomComponentActor sub collection cannot contain itself."));
		return false;
	}
	if (ContainClass->IsChildOf(GetClass()))
	{
		UE_LOG(LogTRGame, Warning, TEXT("RoomComponentActor sub collection cannot contain child classes of itself."));
		return false;
	}
	return true;
}


void ARoomComponentActor::DestroyRCACollections()
{
	UE_LOG(LogTRGame, Log, TEXT("RoomComponentActor %s DestroyRCACollections destroying %d collections"), *GetName(), SpawnedCollections.Num());
	TWeakObjectPtr<ARoomComponentActorCollectionActor> CollectionActor;
	for (int32 i = 0; i < SpawnedCollections.Num(); i++)
	{
		CollectionActor = SpawnedCollections[i];
		if (CollectionActor.IsValid()) {
			UE_LOG(LogTRGame, Log, TEXT("    %s Destroying %s"), *GetName(), *CollectionActor.Get()->GetName());
			//if (IsValid(CollectionActor.Get()->PickedRoomComponentActor_CAC->GetChildActor())) {
			//	CollectionActor.Get()->PickedRoomComponentActor_CAC->DestroyChildActor();
			//}
			CollectionActor.Get()->Reset();
			CollectionActor.Get()->Destroy();			
		}
	}
	SpawnedCollections.Empty();
}


void ARoomComponentActor::RespawnRCACollections()
{
	UE_LOG(LogTRGame, Log, TEXT("RoomComponentActor %s RespawnRCACollections"), *GetName());
	DestroyRCACollections();
	if (RandSeed > 0 && RandStream.GetInitialSeed() != RandSeed) {
		RandStream.Initialize(RandSeed);
	}
	if (GetWorld())
	{
		SpawnRCACollections();
	}
}


int32 ARoomComponentActor::GetPrimaryISMCs(TArray<UInstancedStaticMeshComponent*>& PrimaryISMCs)
{
	PrimaryISMCs.Empty();
	TInlineComponentArray<UInstancedStaticMeshComponent*> ISMComps;
	GetComponents(ISMComps, false);
	for (UInstancedStaticMeshComponent* ISMComp : ISMComps)
	{
		if (IsValid(ISMComp) &&  ISMComp->GetAttachParent() == RootScene)
		{
			PrimaryISMCs.Add(ISMComp);
		}
	}
	return PrimaryISMCs.Num();
}


int32 ARoomComponentActor::GetAltISMCs(const UInstancedStaticMeshComponent* ParentISMC, TArray<UInstancedStaticMeshComponent*>& AltISMCs)
{
	TArray<USceneComponent*> AllChildren;
	UInstancedStaticMeshComponent* TmpISMC = nullptr;
	AltISMCs.Empty();
	if (IsValid(ParentISMC))
	{
		ParentISMC->GetChildrenComponents(true, AllChildren);
		for (USceneComponent* SC : AllChildren)
		{
			TmpISMC = Cast<UInstancedStaticMeshComponent>(SC);
			if (IsValid(TmpISMC))
			{
				AltISMCs.Add(TmpISMC);
			}
		}
	}
	return AltISMCs.Num();
}


bool ARoomComponentActor::ApplyAltMeshes()
{
	if (AltMeshPercent <= 0.0f) { return false; }
	TArray<UInstancedStaticMeshComponent*> PrimaryISMCs;
	TArray<UInstancedStaticMeshComponent*> AltISMCs;
	TArray<int32> RemovedPrimaryIndexes;
	TArray<FTransform> AltTransforms;
	FTransform TmpTransform;
	int32 InstanceIndex = 0;
	int32 InstanceCount = 0;	
	bool AnyAltsApplied = false;
	float AltPercent = AltMeshPercent > 1.0f ? 1.0f : AltMeshPercent;
	if (GetPrimaryISMCs(PrimaryISMCs) > 0)
	{
		for (UInstancedStaticMeshComponent* ISMC : PrimaryISMCs)
		{
			if (GetAltISMCs(ISMC, AltISMCs) > 0)
			{
				// Iterate primary instances, giving each a chance to become an alt instance.
				InstanceCount = ISMC->GetInstanceCount();
				RemovedPrimaryIndexes.Empty(static_cast<int32>(InstanceCount * AltPercent));
				AltTransforms.Empty(static_cast<int32>(InstanceCount * AltPercent));
				for (InstanceIndex = 0; InstanceIndex < InstanceCount; InstanceIndex++)
				{
					if (FMath::FRand() <= AltPercent)
					{
						// Get transform in world space
						ISMC->GetInstanceTransform(InstanceIndex, TmpTransform, true);
						AltTransforms.Add(TmpTransform);
						RemovedPrimaryIndexes.Add(InstanceIndex);
						AnyAltsApplied = true;
					}
				}
				// Remove primary instances. Iterate from end to start index because RemoveIndex can compact the array resluting in changed indexes >= the index removed.
				InstanceCount = RemovedPrimaryIndexes.Num();
				for (InstanceIndex = InstanceCount - 1; InstanceIndex >= 0; InstanceIndex--)
				{
					ISMC->RemoveInstance(RemovedPrimaryIndexes[InstanceIndex]);
				}
				// Add the alt instances
				for (FTransform AltTransform : AltTransforms)
				{
					AltISMCs[FMath::RandRange(0, AltISMCs.Num() - 1)]->AddInstanceWorldSpace(AltTransform);
				}
			}
		}
	}
	return AnyAltsApplied;
}


bool ARoomComponentActor::RevertToPrimaryMeshes()
{
	TArray<UInstancedStaticMeshComponent*> PrimaryISMCs;
	TArray<UInstancedStaticMeshComponent*> AltISMCs;
	TArray<FTransform> AltTransforms;
	FTransform TmpTransform;
	int32 InstanceIndex = 0;
	int32 InstanceCount = 0;
	bool AnyAltsRemoved = false;
	if (GetPrimaryISMCs(PrimaryISMCs) > 0)
	{
		for (UInstancedStaticMeshComponent* ISMC : PrimaryISMCs)
		{
			if (GetAltISMCs(ISMC, AltISMCs) > 0)
			{
				for (UInstancedStaticMeshComponent* AltISMC : AltISMCs)
				{
					InstanceCount = AltISMC->GetInstanceCount();
					AltTransforms.Empty(InstanceCount);
					for (InstanceIndex = 0; InstanceIndex < InstanceCount; InstanceIndex++)
					{
						AltISMC->GetInstanceTransform(InstanceIndex, TmpTransform);
						AltTransforms.Add(TmpTransform);
						AnyAltsRemoved = true;
					}
					AltISMC->ClearInstances();
					for (FTransform AltTransform : AltTransforms)
					{
						ISMC->AddInstance(AltTransform);
					}
				}				
			}
		}
	}
	return AnyAltsRemoved;
}


bool ARoomComponentActor::AllISMsCopiedOut()
{
	if (!bMyISMsCopiedOut) {
		return false;
	}    
	if (!bAllChildISMsCopiedOut) {
		return AllChildISMsCopiedOut();
	}
	return true;
}


bool ARoomComponentActor::AllChildISMsCopiedOut()
{
	if (bAllChildISMsCopiedOut) {
		return true;
	}
	TWeakObjectPtr<ARoomComponentActorCollectionActor> CollectionActor;
	for (int32 i = 0; i < SpawnedCollections.Num(); i++)
	{
		CollectionActor = SpawnedCollections[i];
		if (CollectionActor.IsValid()) {
			if (!CollectionActor.Get()->AllISMsCopiedOut()) {
				return false;
			}
		}
	}
	bAllChildISMsCopiedOut = true;
	return true;
}


/////////////////////////////////////////// ARoomComponentActorCollectionActor ///////////////////////////////////

ARoomComponentActorCollectionActor::ARoomComponentActorCollectionActor()
{
	bReplicates = false;
	PrimaryActorTick.bCanEverTick = false;
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);    
	PickedRoomComponentActor_CAC = CreateDefaultSubobject<UChildActorComponent>(TEXT("PickedRoomComponent"));
	PickedRoomComponentActor_CAC->SetChildActorClass(NULL);
	PickedRoomComponentActor_CAC->SetupAttachment(RootScene);
	//AddInstanceComponent(PickedRoomComponentActor_CAC);
	//PickedRoomComponentActor_CAC->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	if (PickedRoomComponentActor_CAC->GetOwner() == nullptr) {
		UE_LOG(LogTRGame, Error, TEXT("RCACollectionActor child CAC owner is null."));
	}
	//PickedRoomComponentActor_CAC->RegisterComponent();
	bool bFound = false;
	PickedClass = NULL;
#if WITH_EDITORONLY_DATA
	UBlueprint* Blueprint = Cast<UBlueprint>(GetClass()->ClassGeneratedBy);
	if (Blueprint) {
		Blueprint->bRunConstructionScriptOnDrag = false;
	}
#endif
}


void ARoomComponentActorCollectionActor::BeginPlay()
{
	Super::BeginPlay();
}


void ARoomComponentActorCollectionActor::PostInitProperties()
{
	Super::PostInitProperties();
}


void ARoomComponentActorCollectionActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	Reset();
	PickAndInit();
}


void ARoomComponentActorCollectionActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Reset();
	PickAndInit();
}


#if WITH_EDITOR
void ARoomComponentActorCollectionActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);	
}
#endif


int32 ARoomComponentActorCollectionActor::GetRandSeedForChild()
{
	return RandStream.RandRange(1, INT_MAX - 1);
}


void ARoomComponentActorCollectionActor::PickAndInit()
{
	//UE_LOG(LogTRGame, Log, TEXT("RCACollection %s PickAndInit RandSeed: %d"), *GetName(), RandSeed);
	bool bFound = true;
	RandStream.Initialize(RandSeed);
	Reset();
	GetRoomComponentActorClass(bFound);
	if (GetWorld()->WorldType == EWorldType::Game) // || GetWorld()->WorldType == EWorldType::GamePreview
	{
		if (bFound && IsValid(PickedClass) && (ChildRCA == nullptr || ChildRCA->GetClass() != PickedClass))
		{
			UE_LOG(LogTRGame, Log, TEXT("RCACollection child spawning as actor"));
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = GetOwner() == nullptr ? this : GetOwner();
			//FTransform SpawnTransform = PickedRoomComponentActor_CAC->GetComponentTransform();
			FTransform SpawnTransform = GetTransform();// PickedRoomComponentActor_CAC->GetRelativeTransform()* GetTransform(); //PickedRoomComponentActor_CAC->GetComponentTransform();
			UE_LOG(LogTRGame, Log, TEXT("RCACollection %s PickAndInit - Spawning child RCA class %s"), *GetName(), *PickedClass->GetName());
			ChildRCA = GetWorld()->SpawnActorDeferred<ARoomComponentActor>(PickedClass, SpawnTransform, SpawnParams.Owner, SpawnParams.Instigator, SpawnParams.SpawnCollisionHandlingOverride);
			if (ChildRCA)
			{
				if (GetOwner()) {
					UE_LOG(LogTRGame, Log, TEXT("RCACollection %s PickAndInit - Setting %s child RCA owner to collection owner %s"), *GetName(), *ChildRCA->GetName(), *GetOwner()->GetName());
					ChildRCA->SetOwner(GetOwner());
				}
				else {
					UE_LOG(LogTRGame, Log, TEXT("RCACollection %s PickAndInit - Setting %s child RCA owner to this collection."), *GetName(), *ChildRCA->GetName());
					ChildRCA->SetOwner(this);
				}
				ChildRCA->RandSeed = GetRandSeedForChild();
				UGameplayStatics::FinishSpawningActor(ChildRCA, ChildRCA->GetTransform());
				//if (GetAttachParentActor()) {
				//	ChildRCA->AttachToActor(GetAttachParentActor(), FAttachmentTransformRules::SnapToTargetIncludingScale);
				//}
				//else {
				ChildRCA->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform); // FAttachmentTransformRules::SnapToTargetNotIncludingScale
				//}
			}
			else {
				UE_LOG(LogTRGame, Error, TEXT("RCACollection %s PickAndInit - Error spawning child RCA"), *GetName());
			}
		}
	}
	else
	{
		if (bFound && IsValid(PickedClass) && PickedRoomComponentActor_CAC->GetChildActorClass() != PickedClass) 
		{
			UE_LOG(LogTRGame, Log, TEXT("RCACollection child spawning as child actor component"));
			if (IsValid(PickedRoomComponentActor_CAC->GetChildActor())) {
				PickedRoomComponentActor_CAC->DestroyChildActor();
			}
			PickedRoomComponentActor_CAC->SetChildActorClass(PickedClass);
			PickedRoomComponentActor_CAC->CreateChildActor();
			ARoomComponentActor* CAC_ChildRCA = Cast<ARoomComponentActor>(PickedRoomComponentActor_CAC->GetChildActor());
			if (CAC_ChildRCA)
			{
				/*
				if (GetOwner()) {
					UE_LOG(LogTRGame, Log, TEXT("RCACollection %s setting Child CAC owner to it's owner %s."), *GetName(), *GetOwner()->GetName());
					CAC_ChildRCA->SetOwner(GetOwner());
				}
				else {
					UE_LOG(LogTRGame, Log, TEXT("RCACollection %s has null owner. Setting Child CAC owner to this collection."), *GetName());
					CAC_ChildRCA->SetOwner(this);
				}*/
				//CAC_ChildRCA->SetOwner(this);
				CAC_ChildRCA->RandSeed = GetRandSeedForChild();
				//CAC_ChildRCA->RespawnRCACollections();
				UE_LOG(LogTRGame, Log, TEXT("RCACollection %s created child component for RCA %s. Child RandSeed: %d"),
					*GetName(),
					*CAC_ChildRCA->GetName(),
					CAC_ChildRCA->RandSeed);
			}
			else {
				UE_LOG(LogTRGame, Error, TEXT("RCACollection %s - Child actor of class %s on CAC not valid."), *GetName(), *PickedClass->GetName());
			}
		}
	}
	if (!bFound) 
	{
		if (PickedClass != nullptr) {
			UE_LOG(LogTRGame, Error, TEXT("RCACollection::PickAndInit - RCA collection %s, class %s not found."), *GetName(), *PickedClass->GetName());
		}
		else {
			UE_LOG(LogTRGame, Error, TEXT("RCACollection::PickAndInit - RCA collection %s picked class was null"), *GetName());
		}
	}
	bInitTried = true;
}


TSubclassOf<ARoomComponentActor> ARoomComponentActorCollectionActor::PickRoomComponentActorClass(bool& bFoundValid)
{
	if (IsValid(PickedClass) && PickedClass != ARoomComponentActor::StaticClass()) {
		return PickedClass;
	}
	if (RoomComponentActorCollection.RoomComponentActors.Num() == 0) 
	{
		UE_LOG(LogTRGame, Log, TEXT("RCACollection cannot PickRCA class, collection %s contains no entries."), *GetName());
		bFoundValid = false;
		PickedClass = NULL;
		return PickedClass;
	}
	float TotalWeight = 0.0f;
	// Sum total weight
	for (const FRoomComponentActorCollectionItem CollectionItem : RoomComponentActorCollection.RoomComponentActors)
	{
		if (!IsValid(CollectionItem.RCAClass)) 
		{
			UE_LOG(LogTRGame, Warning, TEXT("RCACollection Invalid RCA class in collection %s"), *GetName());
			bFoundValid = false;
			PickedClass = NULL;
			return PickedClass;
		}
		TotalWeight += FMath::Max(0.0f, CollectionItem.Weight);
	}
	// If total weight = 0, pick a random one
	if (TotalWeight == 0.0f)
	{
		bFoundValid = true;
		PickedClass = RoomComponentActorCollection.RoomComponentActors[RandStream.RandRange(0, RoomComponentActorCollection.RoomComponentActors.Num() - 1)].RCAClass;
		if (IsValid(PickedClass)) 
		{
			//UE_LOG(LogTRGame, Log, TEXT("RCACollection %s picked room component %s from %d choice(s)"),
			//	*GetName(),
			//	*PickedClass->GetName(),
			//	RoomComponentActorCollection.RoomComponentActors.Num());
		}
		return PickedClass;
	}
	else
	{	
		// Pick a target weight and grab the first one in our picked weight range.
		float PickedWeight = RandStream.FRandRange(0.0f, TotalWeight);
		TotalWeight = 0.0f;
		for (const FRoomComponentActorCollectionItem CollectionItem : RoomComponentActorCollection.RoomComponentActors)
		{
			TotalWeight += FMath::Max(0.0f, CollectionItem.Weight);
			if (PickedWeight <= TotalWeight) 
			{
				bFoundValid = true;
				PickedClass = CollectionItem.RCAClass;
				//UE_LOG(LogTRGame, Log, TEXT("RCACollection %s picked room component %s from %d choices (picked weight: %f)"),
				//	*GetName(),
				//	*PickedClass->GetName(),
				//	RoomComponentActorCollection.RoomComponentActors.Num(),
				//	PickedWeight);
				return PickedClass;
			}
		}
	}
	UE_LOG(LogTRGame, Error, TEXT("RCACollection::PickRoomComponentActorClass - Found no valid class."))
	bFoundValid = false;
	PickedClass = NULL;
	return PickedClass;
}


TSubclassOf<ARoomComponentActor> ARoomComponentActorCollectionActor::GetRoomComponentActorClass(bool& bFoundValid)
{
	PickRoomComponentActorClass(bFoundValid);
	return PickedClass;
}


void ARoomComponentActorCollectionActor::Reset()
{
	PickedClass = NULL;
	if (ChildRCA) 
	{
		ChildRCA->Destroy();
		ChildRCA = nullptr;
	}
}


bool ARoomComponentActorCollectionActor::AllISMsCopiedOut()
{
	if (!IsValid(PickedClass)) {
		return false;
	}
	if (bInitTried) 
	{
		//ARoomComponentActor* ChildRCA = Cast<ARoomComponentActor>(PickedRoomComponentActor_CAC->GetChildActor());
		//if (!IsValid(ChildRCA)) {
		//	return true;
		//}
		if (IsValid(ChildRCA)) {
			return ChildRCA->AllISMsCopiedOut();
		}
		return true;
	}
	return false;
}