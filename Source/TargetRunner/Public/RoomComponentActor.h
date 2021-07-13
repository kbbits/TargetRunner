// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomComponentActor.generated.h"


USTRUCT(BlueprintType)
struct TARGETRUNNER_API FRoomComponentActorCollectionItem
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "RoomComponentActor")
		TSubclassOf<ARoomComponentActor> RCAClass;

	UPROPERTY(EditAnywhere, Category = "RoomComponentActor")
		float Weight = 1.0f;

	FORCEINLINE bool operator==(const FRoomComponentActorCollectionItem& OtherType) const
	{
		if (RCAClass != OtherType.RCAClass) return false;
		return true;
	}

	FORCEINLINE bool operator==(const FRoomComponentActorCollectionItem& OtherType)
	{
		if (RCAClass != OtherType.RCAClass) return false;
		return true;
	}

	FORCEINLINE bool operator==(FRoomComponentActorCollectionItem& OtherType)
	{
		if (RCAClass != OtherType.RCAClass) return false;
		return true;
	}
};


USTRUCT(BlueprintType)
struct TARGETRUNNER_API FRoomComponentActorCollection
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "RoomComponentActor")
		TArray<FRoomComponentActorCollectionItem> RoomComponentActors;

	UPROPERTY(EditAnywhere, Category = "RoomComponentActor")
		FTransform LocalTransform;
};


//////////////////////////////////////  ARoomComponentActor /////////////////////////////

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

	// Should this move it's ISMCs to the grid manager?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bMoveISMCs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FRoomComponentActorCollection> SubRoomComponents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
		int32 RandSeed;

protected:

	UPROPERTY()
		FRandomStream RandStream;

	bool bCollectionSpawned;

	UPROPERTY()
		TArray<ARoomComponentActorCollectionActor*> SpawnedCollections;

protected:

	bool FilterRCACollections();

	bool SpawnRCACollections();

public:
	virtual void PostInitProperties() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintCallable)
		bool CanContainRoomComponentClass(const TSubclassOf<ARoomComponentActor> ContainClass);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SetMoveISMCs(const bool bMoveThem);

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


///////////////////////////////////////  ARoomComponentActorCollectionActor /////////////////////////////////

UCLASS()
class TARGETRUNNER_API ARoomComponentActorCollectionActor : public AActor
{
	GENERATED_BODY()

public:
	ARoomComponentActorCollectionActor();

	//ARoomComponentActorCollectionActor(const FRoomComponentActorCollection& RCACollection);

public:
	UPROPERTY()
		USceneComponent* RootScene;

	UPROPERTY(VisibleAnywhere)
		UChildActorComponent* PickedRoomComponentActor_CAC;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "RoomComponentActor")
		FRoomComponentActorCollection RoomComponentActorCollection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomComponentActor", Meta = (ExposeOnSpawn = "true"))
		int32 RandSeed;

protected:
	UPROPERTY()
		TSubclassOf<ARoomComponentActor> PickedClass;

	UPROPERTY()
		FRandomStream RandStream;

protected:
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable, CallInEditor)
		void PickAndInit();

	virtual void PostInitProperties() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION()
		TSubclassOf<ARoomComponentActor> PickRoomComponentActorClass(bool& bFoundValid);

	UFUNCTION(BlueprintCallable, Category = "RoomComponentActor")
		TSubclassOf<ARoomComponentActor> GetRoomComponentActorClass(bool& bFoundValid);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "RoomComponentActor")
		void Reset();
};