// Fill out your copyright notice in the Description page of Project Settings.

#include "GoodsQuantity.h"

FGoodsQuantity::FGoodsQuantity()
{
	Name = NAME_None;
	Quantity = 0.0f;
}

FGoodsQuantity::FGoodsQuantity(FName NewName, float NewQuantity)
{
	Name = NewName;
	Quantity = NewQuantity;
}