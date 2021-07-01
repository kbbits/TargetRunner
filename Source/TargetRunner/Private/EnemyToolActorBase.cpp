// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyToolActorBase.h"
#include "EnemyToolWeaponBase.h"

AEnemyToolActorBase::AEnemyToolActorBase()
{
	SetReplicates(true);
	
}


ATREnemyCharacter* AEnemyToolActorBase::GetAttachedCharacter_Implementation()
{
	ATREnemyCharacter* EnemyParent;
	EnemyParent = Cast<ATREnemyCharacter>(GetParentActor());
	if (IsValid(EnemyParent)) {
		return EnemyParent;
	}
	EnemyParent = Cast<ATREnemyCharacter>(GetAttachParentActor());
	if (IsValid(EnemyParent)) {
		return EnemyParent;
	}
	return nullptr;
}


void AEnemyToolActorBase::ScaleToLevel_Implementation(const int32 LevelTo)
{
	if (!IsValid(Tool))
	{
		InitToolObject();
	}
	UEnemyToolWeaponBase* EnemyTool = Cast<UEnemyToolWeaponBase>(Tool);
	FToolData TmpToolData = ToolData;
	if (!IsValid(EnemyTool)) {
		return;
	}
	EnemyTool->Level = LevelTo;
	EnemyTool->ScaleToLevel();
	// Update our ToolData to match
	EnemyTool->ToToolData(ToolData);
	bScaleUpApplied = true;
}