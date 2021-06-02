// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "ObjectPlacerProxyBox.generated.h"


UCLASS()
class TARGETRUNNER_API AObjectPlacerProxyBox : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AObjectPlacerProxyBox();

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<AActor> ClassToPlace;

	// This is our proxy placement bounding box extents (the half-size of each dimention).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPreserveRatio = "True"))
		FVector BoxExtent;

protected:

	// Root scene for the node
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USceneComponent* RootScene;

	// A visual aid for where the box target rests.
	UPROPERTY()
		UBoxComponent* BoxTarget;

	// Indicates location and "up" (+Z) of the placed target actor.
	UPROPERTY()
		UArrowComponent* FacingArrow;

	// Rotation applied to placed actor before it is placed in the oriented target box.
	// This initial rotation should result in the "top" of the actor being aligned towards +Z.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRotator InitialRotation;

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

	virtual void BeginDestroy() override;

	virtual void PostInitProperties() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
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
		FTransform GetInitialPlaceTransform(UPARAM(ref) FRandomStream& RandStream);

	// Do the given extents fit fully within this instance's target box?
	UFUNCTION(BlueprintPure)
		bool FitsIntoBoxBounds(const FVector& Extents);
};
