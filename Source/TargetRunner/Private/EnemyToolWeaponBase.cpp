// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyToolWeaponBase.h"

UEnemyToolWeaponBase::UEnemyToolWeaponBase()
	: Super()
{
	Level = 1;
}


void UEnemyToolWeaponBase::ScaleToLevel_Implementation()
{
	bScaleUpApplied = true;
}