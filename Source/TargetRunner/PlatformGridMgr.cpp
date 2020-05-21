// Fill out your copyright notice in the Description page of Project Settings.

#include "PlatformGridMgr.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APlatformGridMgr::APlatformGridMgr()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APlatformGridMgr::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlatformGridMgr::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlatformGridMgr::Setup()
{
	TArray<AActor*> FoundPlatforms;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlatformBase::StaticClass(), FoundPlatforms);

	for (AActor* TActor : FoundPlatforms)
	{
		APlatformBase* Platform = Cast<APlatformBase>(TActor);
		AddPlatformToGridMap(Platform);
	}
}

void APlatformGridMgr::AddPlatformToGridMap(APlatformBase* Platform)
{
	if (Platform != nullptr) {
		int32 row = Platform->GridX;
		int32 col = Platform->GridY;
		Platform->MyGridManager = this;

		if (!PlatformGridMap.Contains(row))
		{
			FPlatformGridRow newRow;
			PlatformGridMap.Add(row, newRow);
		}
		PlatformGridMap[row].RowPlatforms.Add(col, Platform);

		//if (!PlatformGrid.IsValidIndex(row))
		//{
		//	PlatformGrid.InsertDefaulted(row, 1);
		//}
		//if (PlatformGrid[row].Platforms.Num() < col)
		//PlatformGrid[row].Platforms[col] = Platform;

	}
}

APlatformBase * APlatformGridMgr::GetPlatformInGridMap(int32 X, int32 Y, bool& Found)
{
	Found = false;
	if (PlatformGridMap.Contains(X)) {
		if (PlatformGridMap[X].RowPlatforms.Contains(Y))
		{
			Found = true;
			return PlatformGridMap[X].RowPlatforms[Y];
		}	
	}
	return nullptr;
}

APlatformBase * APlatformGridMgr::RemovePlatformFromGridMap(int32 X, int32 Y, bool& Success)
{
	Success = false;
	APlatformBase *Platform = GetPlatformInGridMap(X, Y, Success);
	if (Success && Platform != nullptr) {
		PlatformGridMap[X].RowPlatforms.Remove(Y);
		Success = true;
	}
	return Platform;
}
