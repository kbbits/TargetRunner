// Fill out your copyright notice in the Description page of Project Settings.


#include "TR_Character.h"
#include "TRPlayerControllerBase.h"
#include "CollectableResource.h"
#include "CollectablePickup.h"
#include "GoodsQuantity.h"
#include "TargetRunner.h"

// Sets default values
ATR_Character::ATR_Character(const FObjectInitializer& OI) : Super(OI)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ResourceCollectionVolume = OI.CreateDefaultSubobject<UCapsuleComponent>(this, FName("Resource Collector"));
	if (ResourceCollectionVolume)
	{
		ResourceCollectionVolume->SetupAttachment(GetRootComponent());
		ResourceCollectionVolume->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		ResourceCollectionVolume->InitCapsuleSize(GetCapsuleComponent()->GetScaledCapsuleRadius() * 1.7f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 1.2f);
		ResourceCollectionVolume->SetCollisionProfileName(TEXT("PickupOverlap"));
		ResourceCollectionVolume->OnComponentBeginOverlap.AddDynamic(this, &ATR_Character::OnCollectorOverlapBegin);
		ResourceCollectionVolume->OnComponentEndOverlap.AddDynamic(this, &ATR_Character::OnCollectorOverlapEnd);
	}
	else { UE_LOG(LogTRGame, Error, TEXT("TR_Character constructor failed to create ResourceCollectionVolume component")); }
}


void ATR_Character::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	SetupCollectionVolume();
}


void ATR_Character::PostInitProperties()
{
	Super::PostInitProperties();
#if WITH_EDITOR
	SetupCollectionVolume();
#endif
}

#if WITH_EDITOR
void ATR_Character::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{	
	Super::PostEditChangeProperty(PropertyChangedEvent);
	SetupCollectionVolume();
}
#endif

void ATR_Character::SetupCollectionVolume_Implementation()
{
	USceneComponent* CollectorParent = GetCollectorParentComponent();
	if (IsValid(ResourceCollectionVolume))
	{
		if (!CollectorParent)
		{  
			UE_LOG(LogTRGame, Warning, TEXT("TR_Character SetupCollectionVolume: GetCollectorParentComponent is null."));
			return;
		}
		if (ResourceCollectionVolume->GetAttachParent() != CollectorParent)
		{
			ResourceCollectionVolume->AttachToComponent(CollectorParent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			//UE_LOG(LogTRGame, Log, TEXT("TR_Character SetupCollectionVolume collector volume attached to %s"), *CollectorParent->GetName());
		}
		ResourceCollectionVolume->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		ResourceCollectionVolume->SetCapsuleSize(GetCapsuleComponent()->GetScaledCapsuleRadius() * 1.7f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 1.2f);
	}
}


// Called when the game starts or when spawned
void ATR_Character::BeginPlay()
{
	Super::BeginPlay();
	SetupCollectionVolume();
}


USceneComponent* ATR_Character::GetCollectorParentComponent_Implementation()
{
	return GetRootComponent();
}


// Called every frame
void ATR_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


// Called to bind functionality to input
void ATR_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


void ATR_Character::OnCollectorOverlapBegin_Implementation(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//UE_LOG(LogTRGame, Log, TEXT("Collector overlapped (native): %s"), *OtherActor->GetName());
	TArray<FGoodsQuantity> CollectedGoods;
	FPickupAwards CollectedAwards;
	ATRPlayerControllerBase* TRPlayerController = GetController<ATRPlayerControllerBase>();
	if (OtherActor && OtherActor != this)
	{
		// Try to Execute on C++ layer:
		const auto& CollectableResource = Cast<ICollectableResource>(OtherActor);
		if (CollectableResource) 
		{ 
			CollectableResource->Execute_GetResourceGoods(OtherActor, CollectedGoods);
			CollectableResource->Execute_NotifyCollected(OtherActor);
		}
		else
		{
			const auto& CollectablePickup = Cast<ICollectablePickup>(OtherActor);
			if (CollectablePickup)
			{
				CollectablePickup->Execute_GetPickupAwards(OtherActor, CollectedAwards);
				if (TRPlayerController && TRPlayerController->HasCapacityForAwards(CollectedAwards))
				{
					CollectablePickup->Execute_NotifyPickupCollected(OtherActor);
				}
				else { CollectedAwards.PickupItems.Empty(); }
			}
			else 
			{
				// Else, Execute Interface on Blueprint layer instead:
				if (OtherActor->GetClass()->ImplementsInterface(UCollectableResource::StaticClass()))
				{
					ICollectableResource::Execute_GetResourceGoods(OtherActor, CollectedGoods);
					ICollectableResource::Execute_NotifyCollected(OtherActor);
				} else if (OtherActor->GetClass()->ImplementsInterface(UCollectablePickup::StaticClass()))
				{
					ICollectablePickup::Execute_GetPickupAwards(OtherActor, CollectedAwards);
					if (TRPlayerController && TRPlayerController->HasCapacityForAwards(CollectedAwards))
					{
						ICollectablePickup::Execute_NotifyPickupCollected(OtherActor);
					}
					else { CollectedAwards.PickupItems.Empty(); }
				}
			}
		}
		if (CollectedGoods.Num() > 0)
		{
			Cast<ICollectsResources>(this)->Execute_CollectResourceGoods(this, CollectedGoods);
		}
		if (CollectedAwards.PickupItems.Num() > 0)
		{
			OnCollectedPickupAwards(CollectedAwards);
		}
	}	
}

void ATR_Character::FellOutOfWorld(const class UDamageType& DmgType)
{
	OnFellOutOfWorld();
	//Super::FellOutOfWorld(DmgType);
}

bool ATR_Character::GetCollectionTargetLocation_Implementation(FVector& TargetLocation)
{
	if (IsValid(ResourceCollectionVolume))
	{
		TargetLocation = ResourceCollectionVolume->GetComponentLocation();
		return true;
	}
	return false;
}

