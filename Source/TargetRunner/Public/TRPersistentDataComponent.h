// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/Delegate.h"
#include "UnrealNetwork.h"
//#include "GameFramework/SaveGame.h"
#include "LevelForgeBase.h"
#include "LevelTemplate.h"
#include "TRPersistentDataComponent.generated.h"


// Event dispatcher for when CurrentValue changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewLevelTemplatesPage, const TArray<FLevelTemplate>&, NewLevelTemplatesPage);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelTemplatesSaved, const bool, bSuccessful);


UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TARGETRUNNER_API UTRPersistentDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTRPersistentDataComponent();

public:

	// The LevelForge class to use.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<ULevelForgeBase> DefaultLevelForgeClass;

	// The current save profile. This is used in building file path/names for save files.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString SaveProfile;

	// Array of all level templates. Will only be valid on server.
	UPROPERTY(BlueprintReadWrite)
		TArray<FLevelTemplate> LevelTemplates;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_LevelTemplatesPageLoaded)
		TArray<FLevelTemplate> LevelTemplatesPage;

	//UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_LevelTemplatesPageLoaded)
	//	int32 LevelTemplatesRepTrigger = 0;

	UPROPERTY(BlueprintReadWrite)
		bool bLevelTemplatesLoaded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRandomStream LevelRandStream;

	// Delegate event when LevelTemplatesPage array has changed.
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnNewLevelTemplatesPage OnNewLevelTemplatesPage;

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FOnLevelTemplatesSaved OnLevelTemplatesSaved;

protected:

	// Use GetLevelForge to access.
	UPROPERTY(BlueprintReadOnly)
		ULevelForgeBase* LevelForge;

	// ##### Functions

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	ULevelForgeBase* GetLevelForge();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// [Client]
	// Replication notification
	UFUNCTION()
		void OnRep_LevelTemplatesPageLoaded();
	//void OnRep_LevelTemplatesPageLoaded_Implementation();
			
	// [Server]
	// Server will generate a new level template. Results will show up in the replicated LevelTemplatesPage property which
	// will fire the OnNewLevelTemplatesPage delegate event.
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerGenerateNewLevelTemplate(const float Tier);

	// [Server]
	// The filename of the level templates data. 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		FString GetLevelTemplatesSaveFilename();

	// [Server]
	// Just calls the overrideable ServerSaveLevelTemplatesImpl
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerSaveLevelTemplatesData();
	// [Server]
	// Override this one to implement.
	UFUNCTION(BlueprintNativeEvent)
		void ServerSaveLevelTemplatesDataImpl();

	// [Server]
	// Just calls the overrideable ServerLoadLevelTemplatesImpl
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerLoadLevelTemplatesData();
	// [Server]
	// Override this one to implement.
	UFUNCTION(BlueprintNativeEvent)
		void ServerLoadLevelTemplatesDataImpl();

	UFUNCTION()
	void OnLevelTemplatesSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccessful);
};
