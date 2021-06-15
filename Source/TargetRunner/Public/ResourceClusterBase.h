// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResourceQuantity.h"
#include "ResourceNodeBase.h"
#include "ResourceClusterBase.generated.h"

UCLASS()
class TARGETRUNNER_API AResourceClusterBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AResourceClusterBase();

public:

	// The class of Resource Node to spawn on the various resource node sites of this cluster.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn="true"))
		TSubclassOf<AResourceNodeBase> NodeClass;

	// The list of classes of Resource Node to spawn on the various resource node sites of this cluster.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
		TArray<TSubclassOf<AResourceNodeBase>> NodeClasses;

	// The total resources to be divided amongst the cluster's resource nodes.
	// Note: the ResourceType of each node will be set to the first entry in the TotalResources array.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true", TitleProperty = "ResourceType"))
		TArray<FResourceQuantity> TotalResources;

protected:
	// Root scene for the node
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		USceneComponent* RootScene;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
