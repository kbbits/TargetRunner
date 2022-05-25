// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectPlacerProxyBox.h"
#include "TargetRunner.h"
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
	DefaultExtentHint = FVector(50.f, 50.f, 50.f);
	ActorSpacingMin = 25.f;
	ActorSpacingMax = 50.f;
	SpawnOffsetMin = 0.f;
	SpawnOffsetMax = 0.f;
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene"));
	SetRootComponent(RootScene);
	BoxTarget = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxTarget"));
	BoxTarget->SetupAttachment(RootScene);
	FacingArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("FacingArrow"));
	FacingArrow->SetupAttachment(BoxTarget);
	FacingArrow->bIsScreenSizeScaled = false;
#if WITH_EDITOR_DATAONLY
	FacingArrow->bUseInEditorScaling = false;
#endif
	//FacingArrow->ArrowSize = 1.f;
	PlaceAxisPos = CreateDefaultSubobject<UArrowComponent>(TEXT("PlacingAxis+"));
	PlaceAxisPos->SetupAttachment(BoxTarget);
	PlaceAxisPos->SetArrowColor(FLinearColor::Green);
	PlaceAxisPos->bIsScreenSizeScaled = false;
#if WITH_EDITOR_DATAONLY
	PlaceAxisPos->bUseInEditorScaling = false;
#endif
	//PlaceAxisPos->ArrowSize = 1.f;
	PlaceAxisNeg = CreateDefaultSubobject<UArrowComponent>(TEXT("PlacingAxis-"));
	PlaceAxisNeg->SetupAttachment(BoxTarget);
	PlaceAxisNeg->SetArrowColor(FLinearColor::Yellow);
	PlaceAxisNeg->bIsScreenSizeScaled = false;
#if WITH_EDITOR_DATAONLY
	PlaceAxisNeg->bUseInEditorScaling = false;
#endif
	//PlaceAxisNeg->ArrowSize = 1.f;
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
		FacingArrow->ArrowLength = BoxExtent.Z * 2.f;
	}
	if (PlaceAxisPos && PlaceAxisNeg)
	{
		FVector ScaledBoxExtent = BoxTarget->GetScaledBoxExtent();
		if (ScaledBoxExtent.Y > ScaledBoxExtent.X)
		{
			PlaceAxisPos->ArrowLength = BoxExtent.Y;
			PlaceAxisPos->SetRelativeRotation(FRotator(0.f, 90.f, 0.f).Quaternion());			
			PlaceAxisNeg->ArrowLength = BoxExtent.Y;
			PlaceAxisNeg->SetRelativeRotation(FRotator(0.f, -90.f, 0.f).Quaternion());			
		}
		else
		{
			PlaceAxisPos->ArrowLength = BoxExtent.X;
			PlaceAxisPos->SetRelativeRotation(FRotator(0.f, 0.f, 0.f).Quaternion());
			PlaceAxisNeg->ArrowLength = BoxExtent.X;
			PlaceAxisNeg->SetRelativeRotation(FRotator(0.f, 180.f, 0.f).Quaternion());
		}
		//PlaceAxisPos->SetRelativeLocation(FVector(0.f, 0.f, -BoxExtent.Z));
		//PlaceAxisNeg->SetRelativeLocation(FVector(0.f, 0.f, -BoxExtent.Z));
		PlaceAxisPos->MarkRenderStateDirty();
		PlaceAxisNeg->MarkRenderStateDirty();
	}
}


void AObjectPlacerProxyBox::BeginDestroy()
{
	Super::BeginDestroy();
}


void AObjectPlacerProxyBox::Destroyed()
{
#if WITH_EDITOR
	//ClearPlaced();
#endif
	Super::Destroyed();
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
	//BoxExtent = BoxExtent.BoundToCube(5000.0f);	
	UpdateComponents();
}
#endif


// Called every frame
void AObjectPlacerProxyBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


AActor* AObjectPlacerProxyBox::PlaceOne(FRandomStream& RandStream, const AActor* PlacedObjectOwner, bool& bPlacedSuccessfully, const FVector& ExtentHint)
{
	AActor* TmpPlacedObject = nullptr;
	FTransform SpawnTransform;
	FVector Direction;
	float Penetration;
	float Offset;
	float ActorSpacing = GetActorSpacing(RandStream);
	FVector Origin;
	FVector Extent;
	FVector FinalExtentHint = ExtentHint.IsZero() ? DefaultExtentHint : ExtentHint;
	FVector ScaledBoxExtent = BoxTarget->GetScaledBoxExtent();
	bool bBoundsInBox = true;
	FActorSpawnParameters SpawnParams;
	TSubclassOf<AActor> ClassToPlace = GetClassToPlace();
	bPlacedSuccessfully = false;
	// Check class is valid
	if (!IsValid(ClassToPlace)) {
		UE_LOG(LogTRGame, Warning, TEXT("ObjectPlacerProxyBox %s PlaceOne - no valid class to place."), *GetName());
		return nullptr;
	}
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	if (PlacedObjectOwner) { SpawnParams.Owner = const_cast<AActor*>(PlacedObjectOwner); }
	if (!GetInitialPlaceTransform(RandStream, SpawnTransform, ActorSpacing, FinalExtentHint)) {
		//UE_LOG(LogTRGame, Log, TEXT("ObjectPlacerProxyBox %s PlaceOne - found no valid initial spawn location."), *GetName());
		return nullptr;
	}
	TmpPlacedObject = GetWorld()->SpawnActor<AActor>(ClassToPlace, SpawnTransform, SpawnParams);
	if (TmpPlacedObject)
	{
		TmpPlacedObject->GetActorBounds(true, Origin, Extent);
		if (PlacedObjects % 2 == 0.f) {
			if (ScaledBoxExtent.Y > ScaledBoxExtent.X) {				
				Direction = FVector(0.f, 1.f, 0.f);
			}
			else {
				Direction = FVector(1.f, 0.f, 0.f);
			}
			Offset = SpawnOffsetMax;
		}
		else 
		{
			if (ScaledBoxExtent.Y > ScaledBoxExtent.X) {
				Direction = FVector(0.f, -1.f, 0.f);
			}
			else {
				Direction = FVector(-1.f, 0.f, 0.f);
			}
			Offset = SpawnOffsetMin;
		}
		// Check if spawned bounds overlaps bounds of existing spawns
		Penetration = Offset - FVector::DistXY(BoxTarget->GetComponentLocation(), Origin - (BoxTarget->GetComponentRotation().RotateVector(Direction) * Extent));
		if (Penetration > 0.f) {
			// If there is an overlap, move the newly spawned actor so they do not overlap
			TmpPlacedObject->SetActorLocation(TmpPlacedObject->GetActorLocation() + (BoxTarget->GetComponentRotation().RotateVector(Direction) * (Penetration + ActorSpacing)), false, nullptr, ETeleportType::TeleportPhysics);
			TmpPlacedObject->GetActorBounds(true, Origin, Extent);
			//UE_LOG(LogTRGame, Log, TEXT("ObjectPlacerProxyBox %s PlaceOne - actor overlaps existing bounds. Location adjusted by %s to %s"), *GetName(), *((Penetration + ActorSpacing) * Direction).ToString(), *TmpPlacedObject->GetActorLocation().ToString());
		}
		// Check if spawned bounds exceed our box extents
		//if (Offset + ((Extent * Direction).GetAbsMax() * 2) + ActorSpacing > (ScaledBoxExtent * Direction).GetAbsMax()) { 
		//if (FVector::DistXY(BoxTarget->GetComponentLocation(), Origin + BoxTarget->GetComponentRotation().RotateVector(Extent * Direction)) > (ScaledBoxExtent * Direction).GetAbsMax())
		if (!FitsIntoBoxBounds(Extent, ActorSpacing))
		{
			// If it endedup outside our box extents, destroy it and return null;
			TmpPlacedObject->Destroy();
			TmpPlacedObject = nullptr;
			//UE_LOG(LogTRGame, Log, TEXT("ObjectPlacerProxyBox %s PlaceOne - placement exceeded bounds, destroyed placed actor."), *GetName());
		}
		else
		{
			if (Offset == 0.f) 
			{
				SpawnOffsetMax = (Extent * Direction).GetAbsMax();
				SpawnOffsetMin = SpawnOffsetMax;
			}
			else 
			{
				if (Direction.X == 1.f || Direction.Y == 1.f) {
					SpawnOffsetMax = (FVector::DistXY(FacingArrow->GetComponentLocation(), Origin) + (Extent * Direction).GetAbsMax());
				}
				else {
					SpawnOffsetMin = (FVector::DistXY(FacingArrow->GetComponentLocation(), Origin) + (Extent * Direction).GetAbsMax());
				}
			}
			//UE_LOG(LogTRGame, Log, TEXT("ObjectPlacerProxyBox %s PlaceOne - placed actor %s at %s. New offsets: Min %.1f  Max %.1f"), *GetName(), *TmpPlacedObject->GetName(), *TmpPlacedObject->GetActorLocation().ToString(), SpawnOffsetMin, SpawnOffsetMax);
			PlacedObjectRefs.Add(TmpPlacedObject);
			bPlacedSuccessfully = true;
			PlacedObjects++;
		}
	}	
	return TmpPlacedObject;
}


TArray<AActor*> AObjectPlacerProxyBox::PlaceAll(FRandomStream& RandStream, const AActor* PlacedObjectOwner, const int32 MaxPlacements, const FVector& ExtentHint)
{
	TArray<AActor*> PlacedActors;
	AActor* PlacedActor;
	bool bSuccessful = true;
	while (bSuccessful && PlacedActors.Num() < MaxPlacements)
	{
		PlacedActor = PlaceOne(RandStream, PlacedObjectOwner, bSuccessful, ExtentHint);
		if (PlacedActor) {
			PlacedActors.Add(PlacedActor);
		}
	}
	return PlacedActors;
}


#if WITH_EDITOR
void AObjectPlacerProxyBox::TryPlaceOne()
{
	FRandomStream RandStream;
	AActor* PlacedActor;
	bool bSuccess;
	RandStream.GenerateNewSeed();
	PlacedActor = PlaceOne(RandStream, nullptr, bSuccess, DefaultExtentHint);
}


void AObjectPlacerProxyBox::TryPlaceAll()
{
	FRandomStream RandStream;
	RandStream.GenerateNewSeed();
	PlaceAll(RandStream, nullptr, 100, DefaultExtentHint);
}
#endif


void AObjectPlacerProxyBox::ClearPlaced()
{
	//UE_LOG(LogTRGame, Log, TEXT("ObjectPlacerProxyBox %s ClearPlaced - destroying %d placed actors"), *GetName(), PlacedObjectRefs.Num());
	for (TWeakObjectPtr<AActor> WeakActor : PlacedObjectRefs)
	{
		if (WeakActor.IsValid() && WeakActor.Get()) {
			WeakActor.Get()->Destroy();
		}
	}
	PlacedObjectRefs.Empty();
	PlacedObjects = 0;
	SpawnOffsetMax = 0.f;
	SpawnOffsetMin = 0.f;
}


TSubclassOf<AActor> AObjectPlacerProxyBox::GetClassToPlace_Implementation()
{
	return DefaultClassToPlace;
}

/*
 * Places spawns starting at box extent center, then alternates between left of center (-SpawnOffsetMin) and right of center (SpawnOffsetMax), 
 * along the longest X or Y axis of the BoxExtent (along X if they are equal).
 */
bool AObjectPlacerProxyBox::GetInitialPlaceTransform(FRandomStream& RandStream, FTransform& PlaceTransform, const float ActorSpacing, const FVector& ExtentHint)
{
	FVector Direction;
	FVector ScaledBoxExtent = BoxTarget->GetScaledBoxExtent();
	float Offset;
	PlaceTransform = FTransform::Identity;
	// Check if this would end up outside our box extents.
	if (!FitsIntoBoxBounds(ExtentHint, ActorSpacing)) {
		return false;
	}
	if (PlacedObjects % 2 == 0.f) {
		if (ScaledBoxExtent.Y > ScaledBoxExtent.X) { Direction = FVector(0.f, 1.f, 0.f); }
		else { Direction = FVector(1.f, 0.f, 0.f); }
		Offset = SpawnOffsetMax;
	}
	else {
		if (ScaledBoxExtent.Y > ScaledBoxExtent.X) { Direction = FVector(0.f, -1.f, 0.f); }
		else { Direction = FVector(-1.f, 0.f, 0.f); }
		Offset = SpawnOffsetMin;
	}
	if (Offset == 0.f) {
		PlaceTransform.SetLocation(FacingArrow->GetComponentLocation());
	} 
	else {
		Offset += (ExtentHint * Direction).GetAbsMax() + ActorSpacing;
		PlaceTransform.SetLocation(FacingArrow->GetComponentLocation() + (BoxTarget->GetComponentRotation().RotateVector(Direction) * Offset));
	}	
	PlaceTransform.SetRotation(BoxTarget->GetComponentRotation().Quaternion() * InitialRotation.Quaternion());
	//UE_LOG(LogTRGame, Log, TEXT("ObjectPlacerProxyBox %s GetInitialPlaceTransform - using offset %.1f (rotated: %s) found location %s."), *GetName(), Offset, *(BoxTarget->GetComponentRotation().RotateVector(Direction) * Offset).ToString(), *PlaceTransform.GetLocation().ToString());
	return true;
}


bool AObjectPlacerProxyBox::FitsIntoBoxBounds(const FVector& ActorExtents, const float ActorSpacing, const bool bIgnoreZ)
{
	FVector ScaledBoxExtent = BoxTarget->GetScaledBoxExtent();
	if (ActorExtents.X > ScaledBoxExtent.X || ActorExtents.Y > ScaledBoxExtent.Y) {
		return false;
	}
	if (!bIgnoreZ && ActorExtents.Z > ScaledBoxExtent.Z) {
		return false;
	}
	FVector Direction;
	float Offset;
	float ExtentMult;
	if (PlacedObjects % 2 == 0.f) 
	{
		if (ScaledBoxExtent.Y > ScaledBoxExtent.X) { Direction = FVector(0.f, 1.f, 0.f); }
		else { Direction = FVector(1.f, 0.f, 0.f); }
		Offset = SpawnOffsetMax;
	}
	else 
	{
		if (ScaledBoxExtent.Y > ScaledBoxExtent.X) { Direction = FVector(0.f, -1.f, 0.f); }
		else { Direction = FVector(-1.f, 0.f, 0.f); }
		Offset = SpawnOffsetMin;
	}
	ExtentMult = Offset == 0.f ? 1.f : 2.f;
	if (Offset + ((ActorExtents * Direction).GetAbsMax() * ExtentMult) + (ActorSpacing * (ExtentMult - 1)) > (ScaledBoxExtent * Direction).GetAbsMax()) {
		UE_LOG(LogTRGame, Log, TEXT("ObjectPlacerProxyBox %s FitsIntoBoxBounds - extents %s do not fit %s with offset %.1f (spacing: %.1f)."), *GetName(), *ActorExtents.ToString(), *ScaledBoxExtent.ToString(), Offset, ActorSpacing);
		return false;
	}
	return true;
}


float AObjectPlacerProxyBox::GetActorSpacing(FRandomStream& RandStream)
{
	return RandStream.FRandRange(ActorSpacingMin, ActorSpacingMax);
}