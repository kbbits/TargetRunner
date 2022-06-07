// Fill out your copyright notice in the Description page of Project Settings.


#include "TRMath.h"


float UTRMath::Summation(const float Value)
{
	if (Value <= 0.0f) { return 0.0f; }
	if (Value <= 1.0f) { return Value; }
	float TruncVal = FMath::TruncToFloat(Value);
	return ((TruncVal * (TruncVal + 1)) / 2.0f) + (FMath::Fractional(Value) * (TruncVal + 1.0f));

};

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


ETRDirection UTRMath::OppositeDirection(const ETRDirection Direction)
{
	switch (Direction)
	{
	case ETRDirection::North :
		return ETRDirection::South;
	case ETRDirection::NorthEast :
		return ETRDirection::SouthWest;
	case ETRDirection::East:
		return ETRDirection::West;
	case ETRDirection::SouthEast:
		return ETRDirection::NorthWest;
	case ETRDirection::South:
		return ETRDirection::North;
	case ETRDirection::SouthWest:
		return ETRDirection::NorthEast;
	case ETRDirection::West:
		return ETRDirection::East;
	case ETRDirection::NorthWest:
		return ETRDirection::SouthEast;
	}
	return ETRDirection::North;
}


TArray<FIntPoint> UTRMath::VectorsToIntPoints(const TArray<FVector2D>& Vector2DArray)
{
	TArray<FIntPoint> Points;
	Points.Reserve(Vector2DArray.Num());
	for (FVector2D Vec : Vector2DArray) {
		Points.Add(Vec.IntPoint());
	}
	return Points;
}


const FMeshOption& UTRMath::PickMeshOption(UPARAM(ref) FRandomStream& RandStream, const TArray<FMeshOption>& MeshOptions)
{
	return *PickOneFromWeightedList<FMeshOption>(RandStream, MeshOptions);
}