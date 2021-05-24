// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformBase.h"
#include "TR_GameMode.h"
#include "TREnemyCharacter.h"
#include "UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APlatformBase::APlatformBase()
{
	bReplicates = true;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootScene);
	PlatformScene = CreateDefaultSubobject<USceneComponent>(TEXT("PlatformScene"));
	PlatformScene->SetupAttachment(GetRootComponent());
	PlatformControlZone = CreateDefaultSubobject<UBoxComponent>(TEXT("PlatformControlZone"));
	PlatformControlZone->AttachToComponent(PlatformScene, FAttachmentTransformRules::KeepRelativeTransform);

	PrimaryActorTick.bCanEverTick = true;	
}

// Called when the game starts or when spawned
void APlatformBase::BeginPlay()
{
	Super::BeginPlay();
}

void APlatformBase::InitPlatformScale()
{
	//ATR_GameMode* = UGameplayStatics::GetGameMode<ATR_GameMode>(GetWorld());
}


APlatformGridMgr* APlatformBase::GetGridManager()
{
	if (MyGridManager == nullptr)
	{
		TArray<AActor*> FoundManagers;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlatformGridMgr::StaticClass(), FoundManagers);
		if (FoundManagers.Num() > 0)
		{
			MyGridManager = Cast<APlatformGridMgr>(FoundManagers[0]);
		}
	}
	return MyGridManager;
}

// Called every frame
void APlatformBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void APlatformBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlatformBase, GridX);
	DOREPLIFETIME(APlatformBase, GridY);
}

FVector2D APlatformBase::GetGridCoordinates()
{
	FVector2D GridVector = FVector2D(float(GridX), float(GridY));
	return GridVector;
}


void APlatformBase::DestroyPlatform_Implementation()
{
	// call native virtual function.
	DestroyPlatformImpl();
}


void APlatformBase::DestroyPlatformImpl()
{
	// Destroy actors in the PlatformActorCache
	for (TPair<FName, AActor*> ActorElem : PlatformActorCache)
	{
		if (ActorElem.Value && IsValid(ActorElem.Value)) { ActorElem.Value->Destroy(); }
	}
	PlatformActorCache.Empty(0);
	Destroy();
}


void APlatformBase::StasisSleepActors_Implementation()
{
	TArray<AActor*> PlatformActors;
	IStasisObject* StasisObject;
	PlatformControlZone->GetOverlappingActors(PlatformActors);
	for (AActor* CurActor : PlatformActors)
	{
		StasisObject = Cast<IStasisObject>(CurActor);
		if (StasisObject && StasisObject->Execute_GetStasisState(CurActor) != ETRStasisState::InStasis)
		{
			StasisObject->Execute_StasisSleep(CurActor);
		}
	}
	bStasisAwake = false;
}


void APlatformBase::StasisWakeActors_Implementation()
{
	if (!bStasisAwake)
	{
		TArray<AActor*> PlatformActors;
		IStasisObject* StasisObject;
		PlatformControlZone->GetOverlappingActors(PlatformActors);
		for (AActor* CurActor : PlatformActors)
		{
			StasisObject = Cast<IStasisObject>(CurActor);
			if (StasisObject && StasisObject->Execute_GetStasisState(CurActor) != ETRStasisState::Awake)
			{
				StasisObject->Execute_StasisWake(CurActor);
			}
		}
		bStasisAwake = true;
	}
}