// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomComponentActor.generated.h"

UCLASS()
class TARGETRUNNER_API ARoomComponentActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARoomComponentActor();

public:

	// All InstancedStaticMeshComponents should be attached here. 
	// "Alt" versions of an ISMC should be attached as children to the ISMC that they are "alts" of.
	UPROPERTY()
		USceneComponent* RootScene;

	// The percent (0.0-1.0) of meshes to be converted to their "alt" version.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
		float AltMeshPercent;

public:

	// Get all the primary (i.e. non-alt) InstancedStaticMeshComponents of this Room Component. These are the ISMCs attached directly to the RootScene.
	// Returns the count of components in PrimaryISMCs.
	UFUNCTION(BlueprintPure)
		int32 GetPrimaryISMCs(TArray<UInstancedStaticMeshComponent*>& PrimaryISMCs);

	// Gets all the "alt" ISMCs associated with a given primary ISMC. i.e. the primary's children.
	UFUNCTION(BlueprintPure)
		int32 GetAltISMCs(const UInstancedStaticMeshComponent* ParentISMC, TArray<UInstancedStaticMeshComponent*>& AltISMCs);

	// Swaps AltMeshPercent percent of instances of all primary ISMCs with instances of one of their associated Alt ISMCs.
	// This adds alt instances. You may want to call RevertToPrimaryMeshes before calling this. 
	// Returns true if any instances were swapped with an alt.
	UFUNCTION(BlueprintCallable)
		bool ApplyAltMeshes();

	// Reverts all "alt" mesh instances to an instance of their coresponding primary mesh.
	UFUNCTION(BlueprintCallable)
		bool RevertToPrimaryMeshes();
};

