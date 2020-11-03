// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPlacer.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AObjectPlacer::AObjectPlacer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ConeAngle = 35.0f;
	ConeLength = 500.0f;
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene"));
	SetRootComponent(RootScene);
	FacingArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("FacingArrow"));
	FacingArrow->AttachToComponent(RootScene, FAttachmentTransformRules::KeepRelativeTransform);
	ConeTarget = CreateDefaultSubobject<USphereComponent>(TEXT("EffectTarget"));
	ConeTarget->AttachToComponent(FacingArrow, FAttachmentTransformRules::KeepRelativeTransform);
	UpdateComponents();
}

// Called when the game starts or when spawned
void AObjectPlacer::BeginPlay()
{
	Super::BeginPlay();	
}


void AObjectPlacer::UpdateComponents()
{
	if (ConeTarget)
	{
		if (ConeAngle >= 90.0f) {
			ConeTarget->SetSphereRadius(ConeLength);
			ConeTarget->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		}
		else if (ConeAngle <= 0.0f) {
			ConeTarget->SetSphereRadius(1.0f);
			ConeTarget->SetRelativeLocation(FVector(ConeLength, 0.0f, 0.0f));
		}
		else {
			float Radius = FMath::Tan(FMath::DegreesToRadians(ConeAngle)) * ConeLength;
			if (Radius > 5000.0f) { Radius = 5000.0f; }
			ConeTarget->SetSphereRadius(Radius);
			ConeTarget->SetRelativeLocation(FVector(ConeLength, 0.0f, 0.0f));
		}		
	}
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
	if (ConeAngle < 0.0f) { ConeAngle = 0.0f; }
	if (ConeAngle > 180.0f) { ConeAngle = 180.0f; }
	if (ConeLength < 0.0f) { ConeLength = 0.0f; }
	UpdateComponents();
}
#endif


// Called every frame
void AObjectPlacer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


AActor* AObjectPlacer::PlaceOne(FRandomStream& RandStream, const AActor* PlacedObjectOwner)
{
	AActor* TmpPlacedObject = nullptr;
	FTransform SpawnTransform = GetPlaceTransformInCone(RandStream);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = const_cast<AActor*>(PlacedObjectOwner);
	TmpPlacedObject = GetWorld()->SpawnActor<AActor>(ClassToPlace, SpawnTransform, SpawnParams);
	
	return TmpPlacedObject;
}


FTransform AObjectPlacer::GetPlaceTransformAtConeCenter()
{
	FTransform PlaceTransform;
	FHitResult Hit;
	bool bHitSomething;
	FVector EndTraceLocation = FacingArrow->GetComponentLocation() + (FacingArrow->GetForwardVector() * ConeLength);
	PlaceTransform.SetIdentity();
	bHitSomething = GetWorld()->LineTraceSingleByChannel(
		Hit,
		FacingArrow->GetComponentLocation(),
		EndTraceLocation,
		ECollisionChannel::ECC_WorldDynamic
	);
	if (bHitSomething)
	{
		FVector RotateFilter = FVector(0.0f, (bRotateYToHitSurface ? 1.0f : 0.0f), (bRotateZToHitSurface ? 1.0f : 0.0f));
		PlaceTransform.SetLocation(Hit.ImpactPoint);
		Hit.
		PlaceTransform.SetRotation((Hit.ImpactNormal.Normalize() * RotateFilter).ToOrientationRotator().Quaternion());
	}
	else
	{
		PlaceTransform.SetLocation(FacingArrow->GetComponentLocation() + (FacingArrow->GetForwardVector() * ConeLength));
	}
	return PlaceTransform;
}


FTransform AObjectPlacer::GetPlaceTransformInCone(UPARAM(ref) FRandomStream& RandStream)
{
	FTransform PlaceTransform;
	FHitResult Hit;
	bool bHitSomething;
	FVector EndTraceLocation = FacingArrow->GetComponentLocation() + (RandStream.VRandCone(FacingArrow->GetForwardVector(), FMath::DegreesToRadians(ConeAngle)) * ConeLength);
	PlaceTransform.SetIdentity();
	bHitSomething = GetWorld()->LineTraceSingleByChannel(
		Hit,
		FacingArrow->GetComponentLocation(),
		EndTraceLocation,
		ECollisionChannel::ECC_WorldDynamic
	);
	if (bHitSomething)
	{
		FVector RotateFilter = FVector(0.0f, (bRotateYToHitSurface ? 1.0f : 0.0f), (bRotateZToHitSurface ? 1.0f : 0.0f));
		PlaceTransform.SetLocation(Hit.ImpactPoint);
		PlaceTransform.SetRotation((Hit.ImpactNormal.Normalize() * RotateFilter).ToOrientationRotator().Quaternion());
	}
	else
	{
		PlaceTransform.SetLocation(FacingArrow->GetComponentLocation() + (FacingArrow->GetForwardVector() * ConeLength));
	}
	return PlaceTransform;
}