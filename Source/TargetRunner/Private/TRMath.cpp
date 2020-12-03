// Fill out your copyright notice in the Description page of Project Settings.


#include "TRMath.h"

FVector2D UTRMath::DirectionToOffsetVector(const ETRDirection Direction)
{
	switch (Direction)
	{
	case ETRDirection::North:
		return FVector2D(1.0f, 0.0f);
	case ETRDirection::NorthEast:
		return FVector2D(1.0f, 1.0f);
	case ETRDirection::East:
		return FVector2D(0.0f, 1.0f);
	case ETRDirection::SouthEast:
		return FVector2D(-1.0f, 1.0f);
	case ETRDirection::South:
		return FVector2D(-1.0f, 0.0f);
	case ETRDirection::SouthWest:
		return FVector2D(-1.0f, -1.0f);
	case ETRDirection::West:
		return FVector2D(0.0f, -1.0f);
	case ETRDirection::NorthWest:
		return FVector2D(1.0f, -1.0f);
	default:
		return FVector2D(0.0f, 0.0f);
	}
	return FVector2D(0.f, 0.f);
}


FIntPoint UTRMath::DirectionToOffset(const ETRDirection Direction)
{
	return DirectionToOffsetVector(Direction).IntPoint();
}