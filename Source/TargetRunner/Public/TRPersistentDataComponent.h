// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/Delegate.h"
#include "UnrealNetwork.h"
#include "LevelTemplate.h"
#include "TRPersistentDataComponent.generated.h"


// Event dispatcher for when CurrentValue changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewLevelTemplatesPage, const TArray<FLevelTemplate>&, NewLevelTemplatesPage);


UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TARGETRUNNER_API UTRPersistentDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTRPersistentDataComponent();

public:

	// The current save profile. This is used in building file path/names for save files.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString SaveProfile;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_LevelTemplatesPageLoaded)
		TArray<FLevelTemplate> LevelTemplatesPage;
		
	// Delegate event when LevelTemplatesPage array has changed.
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnNewLevelTemplatesPage OnNewLevelTemplatesPage;

// ##### Functions

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Level Template Save / Load / Paging

	// [Client]
	// Replication notification
	UFUNCTION()
		void OnRep_LevelTemplatesPageLoaded();
			
	// [Server]
	// Server will generate a new level template. Results will show up in the replicated LevelTemplatesPage property which
	// will fire the OnNewLevelTemplatesPage delegate event.
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerGenerateNewLevelTemplate(const float Tier);

	// [Server]
	// Just calls GameInstance to save data
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerSaveLevelTemplatesData();
	
	// [Server]
	// Just calls the overrideable ServerLoadLevelTemplatesImpl
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerLoadLevelTemplatesData();
	
	// [Server]
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerSetLevelTemplateForPlay(const FLevelTemplate& LevelTemplate);

};
