// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "ObjectPlacer.generated.h"

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

	// Degrees
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = "1.0", UIMax = "90.0"))
		float ConeAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = "1.0", UIMax = "10000.0"))
		float ConeLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bRotateXToHitSurface;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bRotateYToHitSurface;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bRotateZToHitSurface;

protected:

	// Root scene for the node
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USceneComponent* RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UArrowComponent* FacingArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USphereComponent* ConeTarget;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void UpdateComponents();

public:	
	
	void PostInitProperties() override;
#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Meta = (UnsafeDuringActorConstruction = "true"))
		AActor* PlaceOne(UPARAM(ref) FRandomStream& RandStream, const AActor* PlacedObjectOwner);

	UFUNCTION(BlueprintPure)
		FTransform GetPlaceTransformAtConeCenter();

	UFUNCTION(BlueprintPure)
		FTransform GetPlaceTransformInCone(UPARAM(ref) FRandomStream& RandStream);
};
