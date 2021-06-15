// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPlacer.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "ResourceClusterBase.h"

// Sets default values
AObjectPlacer::AObjectPlacer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PlacementDistanceAlongHitNormal = 16.f;
	BoxExtent = FVector(20.0f, 250.0f, 150.0f);
	ConeAngle = 35.0f;
	TargetDistance = 300.0f;
	MaxTracesPerPlacement = 20;
	TraceBoxExtents = FVector(0.5f, 0.5f, 0.5f);
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene"));
	SetRootComponent(RootScene);
	FacingArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("FacingArrow"));
	//FacingArrow->AttachToComponent(RootScene, FAttachmentTransformRules::KeepRelativeTransform);
	FacingArrow->SetupAttachment(RootScene);
	ConeTarget = CreateDefaultSubobject<USphereComponent>(TEXT("ConeTarget"));
	//ConeTarget->AttachToComponent(FacingArrow, FAttachmentTransformRules::KeepRelativeTransform);
	ConeTarget->SetupAttachment(FacingArrow);
	BoxTarget = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxTarget"));
	//BoxTarget->AttachToComponent(FacingArrow, FAttachmentTransformRules::KeepRelativeTransform);
	BoxTarget->SetupAttachment(FacingArrow);
	UpdateComponents();
}


// Called when the game starts or when spawned
void AObjectPlacer::BeginPlay()
{
	Super::BeginPlay();	
}


void AObjectPlacer::UpdateComponents()
{
	if (BoxTarget) { BoxTarget->SetVisibility(PlaceIn == EObjectPlacerType::InBox, true); }
	if (ConeTarget) { ConeTarget->SetVisibility(PlaceIn == EObjectPlacerType::InCone, true); }
	if (PlaceIn == EObjectPlacerType::InCone && ConeTarget)
	{
		if (ConeAngle >= 90.0f) {
			ConeTarget->SetSphereRadius(TargetDistance);
			ConeTarget->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		}
		else if (ConeAngle <= 0.0f) {
			ConeTarget->SetSphereRadius(1.0f);
			ConeTarget->SetRelativeLocation(FVector(TargetDistance, 0.0f, 0.0f));
		}
		else {
			float Radius = FMath::Tan(FMath::DegreesToRadians(ConeAngle)) * TargetDistance;
			if (Radius > 5000.0f) { Radius = 5000.0f; }
			ConeTarget->SetSphereRadius(Radius);
			ConeTarget->SetRelativeLocation(FVector(TargetDistance, 0.0f, 0.0f));
		}		
	}
	else if (PlaceIn == EObjectPlacerType::InBox && BoxTarget)
	{
		BoxTarget->SetRelativeLocation(FVector(TargetDistance, 0.0f, 0.0f));
		BoxTarget->SetBoxExtent(BoxExtent, false);
	}
}


void AObjectPlacer::BeginDestroy()
{
#if WITH_EDITOR
	ClearPlaced();
#endif
	Super::BeginDestroy();
}


void AObjectPlacer::PostInitProperties()
{
	Super::PostInitProperties();
	UpdateComponents();
}

#if WITH_EDITOR
void AObjectPlacer::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PlaceIn == EObjectPlacerType::InCone) {
		if (ConeAngle < 0.0f) { ConeAngle = 0.0f; }
		if (ConeAngle > 180.0f) { ConeAngle = 180.0f; }
	}
	else if (PlaceIn == EObjectPlacerType::InBox) {
		if (BoxExtent.X <= 0.0f) { BoxExtent.X = 1.0f; }
		if (BoxExtent.Y <= 0.0f) { BoxExtent.Y = 1.0f; }
		if (BoxExtent.Z <= 0.0f) { BoxExtent.Z = 1.0f; }
		BoxExtent = BoxExtent.BoundToCube(5000.0f);
	}
	if (TargetDistance < 0.0f) { TargetDistance = 0.0f; }
	UpdateComponents();
}
#endif


// Called every frame
void AObjectPlacer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


AActor* AObjectPlacer::PlaceOne(FRandomStream& RandStream, const AActor* PlacedObjectOwner, bool& bPlacedSuccessfully)
{
	AActor* TmpPlacedObject = nullptr;
	FTransform SpawnTransform;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (PlacedObjectOwner) { SpawnParams.Owner = const_cast<AActor*>(PlacedObjectOwner); }
	SpawnTransform = GetPlaceTransform(RandStream, bPlacedSuccessfully);
	if (!bPlacedSuccessfully) { return nullptr; }
	UE_LOG(LogTRGame, Log, TEXT("ObjectPlacer PlaceOne %s"), *ClassToPlace->GetName());
	TmpPlacedObject = GetWorld()->SpawnActor<AActor>(ClassToPlace, SpawnTransform, SpawnParams);
	PlacedObjects++;
	return TmpPlacedObject;
}


#if WITH_EDITOR
void AObjectPlacer::TryPlaceOne()
{
	FRandomStream RandStream;
	AActor* PlacedActor;
	bool bSuccess;
	RandStream.GenerateNewSeed();
	PlacedActor = PlaceOne(RandStream, nullptr, bSuccess);
	PlacedObjectRefs.Add(PlacedActor);
}

void AObjectPlacer::ClearPlaced()
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


FTransform AObjectPlacer::GetPlaceTransform(FRandomStream& RandStream, bool& bFound)
{
	FTransform PlaceTransform = FTransform::Identity;
	FVector StartLocation;
	FVector EndLocation;
	int32 TraceTries = 0;
	int32 MaxTraces = MaxTracesPerPlacement > 0 ? MaxTracesPerPlacement : 1;
	FHitResult Hit;
	bool bHitSomething = false;
	FCollisionShape TraceShape = FCollisionShape::MakeBox(TraceBoxExtents);
	
	bFound = false;
	while (!bHitSomething && TraceTries < MaxTraces)
	{
		TraceTries++;
		switch (PlaceIn)
		{
		case EObjectPlacerType::InCone:
			GetTraceLocationsInCone(RandStream, StartLocation, EndLocation);
			break;
		case EObjectPlacerType::InBox:
			GetTraceLocationsInBox(RandStream, StartLocation, EndLocation);
			break;
		}
		bHitSomething = GetWorld()->SweepSingleByChannel(
			Hit,
			StartLocation,
			EndLocation,
			UKismetMathLibrary::FindLookAtRotation(StartLocation, EndLocation).Quaternion(),
			ECollisionChannel::ECC_Visibility,
			TraceShape
		);
		// Don't allow placed actors to pile up. Try again if we hit actor of type we are placing.
		if (bHitSomething && Hit.Actor->GetClass()->IsChildOf(ClassToPlace)) { bHitSomething = false; }
		if (bHitSomething)
		{
			FRotator ObjectRotation;
			FRotator ImpactNormalRotation = Hit.ImpactNormal.ToOrientationRotator();
			if (bRotateToHitSurface)
			{
				ObjectRotation = FRotator(ImpactNormalRotation.Pitch + FMath::FRandRange(-10.0f, 10.0f), ImpactNormalRotation.Yaw + FMath::FRandRange(-10.0f, 10.0f), FMath::FRandRange(MinRandomRoll, MaxRandomRoll));
			}
			else
			{
				ObjectRotation = FRotator(0.f, 0.f, 0.f);
			}
			PlaceTransform.SetLocation(Hit.ImpactPoint + (Hit.ImpactNormal * PlacementDistanceAlongHitNormal));
			PlaceTransform.SetRotation(ObjectRotation.Quaternion() * InitialRotation.Quaternion());
			bFound = true;
			return PlaceTransform;
		}
	}
	return FTransform::Identity;
}


void AObjectPlacer::GetTraceLocationsInCone(FRandomStream& RandStream, FVector& StartLocation, FVector& EndLocation)
{
	StartLocation = FacingArrow->GetComponentLocation();
	EndLocation = StartLocation + (RandStream.VRandCone(FacingArrow->GetForwardVector(), FMath::DegreesToRadians(ConeAngle)) * TargetDistance);
}


void AObjectPlacer::GetTraceLocationsInBox(FRandomStream& RandStream, FVector& StartLocation, FVector& EndLocation)
{
	StartLocation = FacingArrow->GetComponentLocation();
	FVector EndTraceLocation = FVector(TargetDistance + BoxExtent.X, (2 * (RandStream.GetFraction() * BoxExtent.Y)) - BoxExtent.Y, (2 * (RandStream.GetFraction() * BoxExtent.Z)) - BoxExtent.Z);
	float EndTraceLength = EndTraceLocation.Size(); 
	FQuat RelativeRotation = FacingArrow->GetRelativeRotationFromWorld(FacingArrow->GetForwardVector().Rotation().Quaternion());
	EndLocation = StartLocation + FacingArrow->GetComponentToWorld().TransformVector(RelativeRotation.RotateVector(EndTraceLocation.GetSafeNormal()) * EndTraceLength);
}