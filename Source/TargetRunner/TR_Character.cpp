// Fill out your copyright notice in the Description page of Project Settings.


#include "TR_Character.h"

// Sets default values
ATR_Character::ATR_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATR_Character::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATR_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ATR_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ATR_Character::FellOutOfWorld(const class UDamageType& DmgType)
{
	OnFellOutOfWorld();
	//Super::FellOutOfWorld(DmgType);
}

