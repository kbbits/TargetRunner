// Fill out your copyright notice in the Description page of Project Settings.


#include "GridTemplateCell.h"
#include "TrEnums.h"
#include "..\Public\GridTemplateCell.h"

// Sets default values
UGridTemplateCell::UGridTemplateCell() 
	: Super()
{
	CellState = ETRGridCellState::Open;
	bOnPath = false;
	bFlagged = false;
	Group = 0;
}

const FVector2D UGridTemplateCell::GetCoords()
{
	return FVector2D(X, Y);
}
