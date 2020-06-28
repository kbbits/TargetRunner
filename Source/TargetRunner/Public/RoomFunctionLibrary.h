// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TargetRunner.h"
#include "RoomTemplate.h"
#include "RoomGridTemplate.h"
#include "RoomFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class TARGETRUNNER_API URoomFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		
public:

	// Gets the count of all rooms templates in the grid.
	UFUNCTION(BlueprintPure, Category = "Room Template Functions")
		static int32 GetRoomTemplateCount(const FRoomGridTemplate& RoomGridTemplate, const bool bIncludeBlackout = false);

	// Gets an array of all coordinates of room templates in the grid.
	// Returns the count of all rooms templates in the grid.
	UFUNCTION(BlueprintPure, Category = "Room Template Functions")
		static int32 GetAllRoomTemplateCoords(const FRoomGridTemplate& RoomGridTemplate, TArray<FVector2D>& RoomCoords, const bool bIncludeBlackout = false);
};
