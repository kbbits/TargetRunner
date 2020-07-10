// Fill out your copyright notice in the Description page of Project Settings.


#include "TRGameModeLobby.h"

ATRGameModeLobby::ATRGameModeLobby()
	: Super()
{
	PersistentDataComponent = CreateDefaultSubobject<UTRPersistentDataComponent>(TEXT("PersistentDataComponent"));
	AddOwnedComponent(PersistentDataComponent);
}