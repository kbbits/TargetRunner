
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
		//UE_LOG(LogTRGame, Log, TEXT("RoomComponentActor %s SpawnRCACollections - %d collections already spawned"), *GetName(), SpawnedCollections.Num());
		return true;
	}
	if (!GetWorld()) {
		return false;
	}
	FilterRCACollections();
	//UE_LOG(LogTRGame, Log, TEXT("RoomComponentActor %s SpawnRCACollections spawning %d collections"), *GetName(), SubRoomComponents.Num());
	ARoomComponentActorCollectionActor* CollectionActor = nullptr;
	FTransform SpawnTransform;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	// Spawn a collection actor for each in our collection
	for (FRoomComponentActorCollection RCACollection : SubRoomComponents)
	{
		if (RCACollection.RoomComponentActors.Num() == 0) {
			UE_LOG(LogTRGame, Warning, TEXT("RoomComponentActor %s skipping collection because it contains no RCA entries."), *GetName());
			continue;
		}
		SpawnTransform = RCACollection.LocalTransform * GetTransform();
		//UE_LOG(LogTRGame, Log, TEXT("    RoomComponentActor %s start spawning collection (contains %d RCAs)"), *GetName(), RCACollection.RoomComponentActors.Num());
		CollectionActor = GetWorld()->SpawnActorDeferred<ARoomComponentActorCollectionActor>(ARoomComponentActorCollectionActor::StaticClass(), SpawnTransform, SpawnParams.Owner, SpawnParams.Instigator, SpawnParams.SpawnCollisionHandlingOverride);
		if (CollectionActor) {
			CollectionActor->RoomComponentActorCollection = RCACollection;
			CollectionActor->RandSeed = RandStream.RandRange(1, INT_MAX - 1);
			//UE_LOG(LogTRGame, Log, TEXT("    RoomComponentActor %s finish spawning collection %s (contains %d RCAs) with seed %d"), *GetName(), *CollectionActor->GetName(), CollectionActor->RoomComponentActorCollection.RoomComponentActors.Num(),  CollectionActor->RandSeed);
			UGameplayStatics::FinishSpawningActor(CollectionActor, CollectionActor->GetTransform());
			CollectionActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
			//UE_LOG(LogTRGame, Log, TEXT("    RoomComponentActor %s finished spawning collection %s"), *GetName(), *CollectionActor->GetName());
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
	if (Parent && RandSeed == 0)
	{
		// If we were created by a collection actor, get seed from them.
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
	if (Parent && RandSeed == 0)
	{
		// If we were created by a collection actor, get seed from them.
		ARoomComponentActorCollectionActor* CollectionActor = Cast<ARoomComponentActorCollectionActor>(Parent);
		if (CollectionActor) {
			RandSeed = CollectionActor->GetRandSeedForChild();
		}
	}
	if (RandSeed > 0 && RandStream.GetInitialSeed() != RandSeed) {
		RandStream.Initialize(RandSeed);
	}
	//UE_LOG(LogTRGame, Log, TEXT("RoomComponentActor %s OnConstruction. RandSeed: %d Location: %s"), *GetName(), RandSeed, *GetActorLocation().ToString());
	RespawnRCACollections();
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
	//UE_LOG(LogTRGame, Log, TEXT("RoomComponentActor %s DestroyRCACollections destroying %d collections"), *GetName(), SpawnedCollections.Num());
	TWeakObjectPtr<ARoomComponentActorCollectionActor> CollectionActor;
	for (int32 i = 0; i < SpawnedCollections.Num(); i++)
	{
		CollectionActor = SpawnedCollections[i];
		if (CollectionActor.IsValid()) {
			//UE_LOG(LogTRGame, Log, TEXT("    Destroying %s"), *CollectionActor.Get()->GetName());
			CollectionActor.Get()->Destroy();			
		}
	}
	SpawnedCollections.Empty();
}


void ARoomComponentActor::RespawnRCACollections()
{
	//UE_LOG(LogTRGame, Log, TEXT("RoomComponentActor %s RespawnRCACollections"), *GetName());
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
	if (bFound && IsValid(PickedClass) && PickedRoomComponentActor_CAC->GetChildActorClass() != PickedClass) {
		if (IsValid(PickedRoomComponentActor_CAC->GetChildActor())) {
			PickedRoomComponentActor_CAC->DestroyChildActor();
		}
		PickedRoomComponentActor_CAC->SetChildActorClass(PickedClass);
		PickedRoomComponentActor_CAC->CreateChildActor();
		ARoomComponentActor* ChildRCA = Cast<ARoomComponentActor>(PickedRoomComponentActor_CAC->GetChildActor());
		if (ChildRCA) {
			//ChildRCA->RespawnRCACollections();
			//UE_LOG(LogTRGame, Log, TEXT("RCACollection %s created child component for RCA %s. Child RandSeed: %d"),
			//	*GetName(),
			//	*ChildRCA->GetName(),
			//	ChildRCA->RandSeed);
		}
	}
}


TSubclassOf<ARoomComponentActor> ARoomComponentActorCollectionActor::PickRoomComponentActorClass(bool& bFoundValid)
{
	if (IsValid(PickedClass) && PickedClass != ARoomComponentActor::StaticClass()) {
		return PickedClass;
	}
	if (RoomComponentActorCollection.RoomComponentActors.Num() == 0) {
		UE_LOG(LogTRGame, Log, TEXT("RCACollection cannot PickRCA class, collection %s contains no entries."), *GetName());
		bFoundValid = false;
		PickedClass = NULL;
		return PickedClass;
	}
	float TotalWeight = 0.0f;
	// Sum total weight
	for (const FRoomComponentActorCollectionItem CollectionItem : RoomComponentActorCollection.RoomComponentActors)
	{
		if (!IsValid(CollectionItem.RCAClass)) {
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
		if (IsValid(PickedClass)) {
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
			if (PickedWeight <= TotalWeight) {
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
}


bool ARoomComponentActorCollectionActor::AllISMsCopiedOut()
{
	if (!IsValid(PickedClass)) {
		return false;
	}
	ARoomComponentActor* ChildRCA = Cast<ARoomComponentActor>(PickedRoomComponentActor_CAC->GetChildActor());
	if (!IsValid(ChildRCA)) {
		return true;
	}
	return ChildRCA->AllISMsCopiedOut();
}