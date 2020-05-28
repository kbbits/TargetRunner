// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModeRooms.h"
#include "..\Public\GameModeRooms.h"
//#include "Kismet/GameplayStatics.h"

AGameModeRooms::AGameModeRooms() 
	: Super()
{
	//GoodsDropper = CreateDefaultSubobject<UGoodsDropper>(TEXT("GoodsDropper"));
	//if (IsValid(GoodsDropperTable)) { GoodsDropper->AddDropTableDataToLibrary(GoodsDropperTable); }
}

void AGameModeRooms::BeginPlay()
{
	Super::BeginPlay();
	
}
