// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectPlacerProxyBox.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
// for intellisense
#include "..\public\ObjectPlacerProxyBox.h"


// Sets default values
AObjectPlacerProxyBox::AObjectPlacerProxyBox()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BoxExtent = FVector(100.0f, 100.0f, 100.0f);
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene"));
	SetRootComponent(RootScene);
	BoxTarget = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxTarget"));
	//BoxTarget->AttachToComponent(RootScene, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	BoxTarget->SetupAttachment(RootScene);
	FacingArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("FacingArrow"));
	//FacingArrow->AttachToComponent(BoxTarget, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	FacingArrow->SetupAttachment(BoxTarget);
	UpdateComponents();
}


// Called when the game starts or when spawned
void AObjectPlacerProxyBox::BeginPlay()
{
	Super::BeginPlay();
}


void AObjectPlacerProxyBox::UpdateComponents()
{
	if (BoxTarget) 
	{ 
		BoxTarget->SetVisibility(true);
		BoxTarget->SetRelativeLocation(FVector(0.0f, 0.0f, BoxExtent.Z));
		BoxTarget->SetBoxExtent(BoxExtent, false);
	}
	if (FacingArrow)
	{
		FacingArrow->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());
		FacingArrow->SetRelativeLocation(FVector(0.0f, 0.0f, -BoxExtent.Z));
	}
}


void AObjectPlacerProxyBox::BeginDestroy()
{
#if WITH_EDITOR
	ClearPlaced();
#endif
	Super::BeginDestroy();
}


void AObjectPlacerProxyBox::PostInitProperties()
{
	Super::PostInitProperties();
	UpdateComponents();
}


#if WITH_EDITOR
void AObjectPlacerProxyBox::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (BoxExtent.X <= 0.0f) { BoxExtent.X = 1.0f; }
	if (BoxExtent.Y <= 0.0f) { BoxExtent.Y = 1.0f; }
	if (BoxExtent.Z <= 0.0f) { BoxExtent.Z = 1.0f; }
	BoxExtent = BoxExtent.BoundToCube(5000.0f);	
	UpdateComponents();
}
#endif


// Called every frame
void AObjectPlacerProxyBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


AActor* AObjectPlacerProxyBox::PlaceOne(FRandomStream& RandStream, const AActor* PlacedObjectOwner, bool& bPlacedSuccessfully)
{
	AActor* TmpPlacedObject = nullptr;
	FTransform SpawnTransform;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (PlacedObjectOwner) { SpawnParams.Owner = const_cast<AActor*>(PlacedObjectOwner); }
	SpawnTransform = GetInitialPlaceTransform(RandStream);
	TmpPlacedObject = GetWorld()->SpawnActor<AActor>(ClassToPlace, SpawnTransform, SpawnParams);
	if (TmpPlacedObject)
	{
		bPlacedSuccessfully = true;
		PlacedObjects++;
	}	
	return TmpPlacedObject;
}


#if WITH_EDITOR
void AObjectPlacerProxyBox::TryPlaceOne()
{
	FRandomStream RandStream;
	AActor* PlacedActor;
	bool bSuccess;
	RandStream.GenerateNewSeed();
	PlacedActor = PlaceOne(RandStream, nullptr, bSuccess);
	PlacedObjectRefs.Add(PlacedActor);
}


void AObjectPlacerProxyBox::ClearPlaced()
{
	for (AActor* TmpActor : PlacedObjectRefs)
	{
		if (TmpActor)
		{
			TmpActor->Destroy();
		}
	}
	PlacedObjectRefs.Empty();
}
#endif


FTransform AObjectPlacerProxyBox::GetInitialPlaceTransform(FRandomStream& RandStream)
{
	FTransform PlaceTransform = FTransform::Identity;
	PlaceTransform.SetLocation(FacingArrow->GetComponentLocation());
	PlaceTransform.SetRotation(FacingArrow->GetComponentRotation().Quaternion() * InitialRotation.Quaternion());
	return PlaceTransform;
}


bool AObjectPlacerProxyBox::FitsIntoBoxBounds(const FVector& ActorExtents)
{
	return (BoxTarget->Bounds.BoxExtent.X > ActorExtents.X &&
		BoxTarget->Bounds.BoxExtent.Y > ActorExtents.Y &&
		BoxTarget->Bounds.BoxExtent.Z > ActorExtents.Z);
}