// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "ObjectPlacer.generated.h"

UENUM(BlueprintType)
enum class EObjectPlacerType : uint8
{
	InCone	UMETA(DisplayName = "In Cone"),
	InBox	UMETA(DisplayName = "In Box")
};


UENUM(BlueprintType)
enum class EObjectPlacerAlign : uint8
{
	Any				UMETA(DisplayName = "Any"),
	HorizontalOnly	UMETA(DisplayName = "Horizontal Only"),
	VerticalOnly	UMETA(DisplayName = "Vertical Only")
};


UCLASS()
class TARGETRUNNER_API AObjectPlacer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AObjectPlacer();

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<AActor> ClassToPlace;

	// The targeting area type to use for placement.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EObjectPlacerType PlaceIn;

	// Determines if placement is allowed on vertial surfaces, horizontal surfaces or both.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EObjectPlacerAlign PlacementAllowed;

	// When using In Box targeting, this is box extents (the half-size of each dimention).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPreserveRatio="True"))
		FVector BoxExtent;

	// When using In Cone targeting, this is the angle of the cone in Degrees
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = "1.0", UIMax = "90.0"))
		float ConeAngle;

	// Distance from placer to trace for placement hits. (i.e. the length of the placement cone or distance of placement box from placer)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = "1.0", UIMax = "10000.0"))
		float TargetDistance;

	// An offset distance from the hit, to "push out" (i.e. offset) the spawn location away from the hit location.
	// Default = 16
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float PlacementDistanceAlongHitNormal;

	// Rotation applied to placed actor before any hit/normal rotations are applied.
	// This initial rotation should result in the "top" of the actor being aligned towards +X.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRotator InitialRotation;

	// This rotation is in context of the actor being aligned with up towards +X.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MinRandomRoll;

	// This rotation is in context of the actor being aligned with up towards +X.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MaxRandomRoll;

	// Should the placed actor be rotated so it "sprouts up" from the normal of the surface hit?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bRotateToHitSurface;

	// The limit of placed objects for this ObjectPlacer instance.
	// <= 0 is no limit. Default = 0.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 MaxPlacedObjects;

	// The number of traces placer will fire in target area trying to get a hit on a surface. Default = 20.
	// If <= 1 it will do one trace
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 MaxTracesPerPlacement;

	// The size of the box used for tracing hits.
	// Default = (0.5, 0.5, 0.5) = a one meter box.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector TraceBoxExtents;

protected:

	// Root scene for the node
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USceneComponent* RootScene;

	// The direction to project along.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UArrowComponent* FacingArrow;

	// A visual aid for where the cone target rests.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USphereComponent* ConeTarget;

	// A visual aid for where the box target rests.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UBoxComponent* BoxTarget;

	// Current count of objects placed
	UPROPERTY(BlueprintReadOnly)
		int32 PlacedObjects;

#if WITH_EDITORONLY_DATA
	// When used in-editor, we track actors we've placed so they can be destroyed when this instance is.
	UPROPERTY(BlueprintReadOnly)
		TArray<AActor*> PlacedObjectRefs;
#endif

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void UpdateComponents();

public:	
	
	void BeginDestroy() override;

	void PostInitProperties() override;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Meta = (UnsafeDuringActorConstruction = "true"))
		AActor* PlaceOne(UPARAM(ref) FRandomStream& RandStream, const AActor* PlacedObjectOwner, bool& bPlacedSuccessfully);

#if WITH_EDITOR
	UFUNCTION(CallInEditor)
		void TryPlaceOne();

	UFUNCTION(CallInEditor)
		void ClearPlaced();
#endif

	UFUNCTION(BlueprintPure)
		FTransform GetPlaceTransform(UPARAM(ref) FRandomStream& RandStream, bool& bFound);

	UFUNCTION(BlueprintPure)
		void GetTraceLocationsInCone(UPARAM(ref) FRandomStream& RandStream, FVector& StartLocation, FVector& EndLocation);

	UFUNCTION(BlueprintPure)
		void GetTraceLocationsInBox(UPARAM(ref) FRandomStream& RandStream, FVector& StartLocation, FVector& EndLocation);
};


// For placing room resource nodes/clusters.
UCLASS()
class TARGETRUNNER_API AObjectPlacerResource : public AObjectPlacer
{
	GENERATED_BODY()
};


// For placing room special actors.
UCLASS()
class TARGETRUNNER_API AObjectPlacerSpecialActor : public AObjectPlacer
{
	GENERATED_BODY()
};

