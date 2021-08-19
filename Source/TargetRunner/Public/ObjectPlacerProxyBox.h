// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "ObjectPlacerProxyBox.generated.h"

/*
* Used during map spawning.
* Placed in world (usually as a child actor of a RoomComponentActor) as a proxy for a random selection of actor that will be placed into the
* world by this ObjectPlacer. Once the ObjectPlacer has placed it's selection(s) of object actor(s) the ObjectPlacer can be destroyed.
* This ObjectPlacer provides a box wherein each placed actor will be spawned.
*/
UCLASS()
class TARGETRUNNER_API AObjectPlacerProxyBox : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AObjectPlacerProxyBox();

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<AActor> DefaultClassToPlace;

	// This is our proxy placement bounding box extents (the half-size of each dimention).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPreserveRatio = "True"))
		FVector BoxExtent;

	// Distance between bounds of placed actors. Default = 0.0
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ActorSpacing;

protected:

	// Root scene for the node
	UPROPERTY(BlueprintReadOnly)
		USceneComponent* RootScene;

	// A visual aid for where the box target rests.
	UPROPERTY(BlueprintReadOnly)
		UBoxComponent* BoxTarget;

	// Indicates location and "up" (+Z) of the placed target actor.
	UPROPERTY(VisibleAnywhere)
		UArrowComponent* FacingArrow;

	UPROPERTY(VisibleAnywhere)
		UArrowComponent* PlaceAxisPos;

	UPROPERTY(VisibleAnywhere)
		UArrowComponent* PlaceAxisNeg;

	// Rotation applied to a placed actor before it is placed in the oriented target box.
	// This initial rotation should result in the "top" of the actor being aligned towards +Z.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRotator InitialRotation;

	// Current count of objects placed
	UPROPERTY(BlueprintReadOnly)
		int32 PlacedObjects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector DefaultExtentHint;

	// Objects this placer has placed
	// When in-editor, these objects will be destroyed when this placer instance is.
	UPROPERTY()
		TArray<TWeakObjectPtr<AActor>> PlacedObjectRefs;

	// "left" bounds extent of our spawned objects. (i.e. min offset along x or y spawn location)
	float SpawnOffsetMin;
	// "right" bounds extent of our spawned objects. (i.e. max offset along x or y spawn location)
	float SpawnOffsetMax;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void UpdateComponents();

public:

	virtual void BeginDestroy() override;
	virtual void Destroyed() override;

	virtual void PostInitProperties() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Places one actor
	// Returns a pointer to the actor placed.
	UFUNCTION(BlueprintCallable, Meta = (UnsafeDuringActorConstruction = "true"))
		AActor* PlaceOne(UPARAM(ref) FRandomStream& RandStream, const AActor* PlacedObjectOwner, bool& bPlacedSuccessfully, const FVector& ExtentHint = FVector(0.f, 0.f, 0.f));

	// Places actors until target box is filled along it's longest axis.
	// Returns an array of pointers to the actors placed.
	UFUNCTION(BlueprintCallable, Meta = (UnsafeDuringActorConstruction = "true"))
		TArray<AActor*> PlaceAll(UPARAM(ref) FRandomStream& RandStream, const AActor* PlacedObjectOwner, const int32 MaxPlacements = 10, const FVector& ExtentHint = FVector(0.f, 0.f, 0.f));

#if WITH_EDITOR
	UFUNCTION(CallInEditor)
		void TryPlaceOne();

	UFUNCTION(CallInEditor)
		void TryPlaceAll();
#endif

	UFUNCTION(CallInEditor)
		void ClearPlaced();

	// Returns the class that this placer should place.
	// Base class implementation just returns DefaultClassToPlace.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		TSubclassOf<AActor> GetClassToPlace();

	UFUNCTION(BlueprintPure)
		bool GetInitialPlaceTransform(UPARAM(ref) FRandomStream& RandStream, FTransform& PlaceTransform, const FVector& ExtentHint = FVector(50.f, 50.f, 50.f));

	// Do the given extents fit fully within this instance's target box?
	UFUNCTION(BlueprintPure)
		bool FitsIntoBoxBounds(const FVector& Extents, const bool bIgnoreZ = false);
};


// For placing room special actors.
UCLASS()
class TARGETRUNNER_API AObjectPlacerProxyBoxSpecial : public AObjectPlacerProxyBox
{
	GENERATED_BODY()
};


// For placing room clutter actors.
UCLASS()
class TARGETRUNNER_API AObjectPlacerProxyBoxClutter : public AObjectPlacerProxyBox
{
	GENERATED_BODY()
};
