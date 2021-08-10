// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TargetRunner.h"
#include "RoomTemplate.h"
#include "RoomGridTemplate.h"
#include "RoomFunctionLibrary.generated.h"

/**
 * Helper functions related to Rooms in a RoomGridTemplate.
 */
UCLASS()
class TARGETRUNNER_API URoomFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	static const TArray<ETRDirection> OrthogonalDirections;
		
public:

	// Gets the count of all rooms templates in the grid.
	UFUNCTION(BlueprintPure, Category = "Room Template Functions")
		static int32 GetRoomTemplateCount(const FRoomGridTemplate& RoomGridTemplate, const bool bIncludeBlackout = false);

	// Gets an array of all coordinates of room templates in the grid.
	// Returns the count of all room templates in the grid.
	UFUNCTION(BlueprintPure, Category = "Room Template Functions")
		static int32 GetAllRoomTemplateCoords(const FRoomGridTemplate& RoomGridTemplate, TArray<FVector2D>& RoomCoords, const bool bIncludeBlackout = false);

	// Gets an array of all coordinates of room templates in the grid and a paired array of all the room templates.
	// Returns the count of all room templates in the grid.
	UFUNCTION(BlueprintPure, Category = "Room Template Functions")
		static int32 GetRoomTemplateGridAsArrays(const FRoomGridTemplate& RoomGridTemplate, TArray<FVector2D>& RoomCoords, TArray<FRoomTemplate>& RoomTemplates, const bool bIncludeBlackout = true);

	// Returns false if no room was found at given coords.
	UFUNCTION(BlueprintPure, Category = "Room Template Functions")
		static bool GetRoom(UPARAM(ref) FRoomGridTemplate RoomGridTemplate, const FIntPoint& RoomCoords, FRoomTemplate& Rooom);
	static FRoomTemplate* GetRoom(UPARAM(ref) FRoomGridTemplate& RoomGridTemplate, const FIntPoint& RoomCoords);

	UFUNCTION(BlueprintPure, Category = "Room Template Functions")
		static FRoomExitInfo GetRoomExitInfo(const FRoomGridTemplate& RoomGridTemplate, const FIntPoint& RoomCoords);

	// Returns the WallState of the room for the wall in the given direction. ex: Direction == North, gets the North wall state.
	// Direction must be one of the four orthogonal directions.
	UFUNCTION(BlueprintPure, Category = "Room Template Functions")
		static ETRWallState GetWallState(const FRoomTemplate& Room, const ETRDirection Direction);
};
