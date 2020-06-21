// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomFunctionLibrary.h"
#include "..\Public\RoomFunctionLibrary.h"
#include "RoomGridTemplate.h"

int32 URoomFunctionLibrary::GetRoomTemplateCount(const FRoomGridTemplate& RoomGridTemplate, const bool bIncludeBlackout)
{
	TArray<int32> RowNums;
	TArray<int32> ColNums;
	int32 Count = 0;
	RoomGridTemplate.Grid.GenerateKeyArray(RowNums);
	for (int32 Row : RowNums)
	{
		ColNums.Empty(RoomGridTemplate.Grid.Find(Row)->RowRooms.Num());
		RoomGridTemplate.Grid.Find(Row)->RowRooms.GenerateKeyArray(ColNums);
		for (int32 Col : ColNums)
		{
			const FRoomTemplate* Room = RoomGridTemplate.Grid.Find(Row)->RowRooms.Find(Col);
			if (Room != nullptr)
			{
				if (bIncludeBlackout || !Room->bIsBlackout) {
					Count++;
				}
			}
		}
	}
	return Count;
}

int32 URoomFunctionLibrary::GetAllRoomTemplateCoords(const FRoomGridTemplate& RoomGridTemplate, TArray<FVector2D>& RoomCoords, const bool bIncludeBlackout)
{
	TArray<int32> RowNums;
	TArray<int32> ColNums;
	RoomGridTemplate.Grid.GenerateKeyArray(RowNums);
	for (int32 RowNum : RowNums)
	{
		const FRoomGridRow* Row = RoomGridTemplate.Grid.Find(RowNum);
		ColNums.Empty(Row->RowRooms.Num());
		Row->RowRooms.GenerateKeyArray(ColNums);
		for (int32 ColNum : ColNums)
		{
			const FRoomTemplate* Room = RoomGridTemplate.Grid.Find(RowNum)->RowRooms.Find(ColNum);
			if (Room != nullptr)
			{
				if (bIncludeBlackout || !Room->bIsBlackout) {
					RoomCoords.Add(FVector2D(RowNum, ColNum));
				}
			}
		}
	}
	return RoomCoords.Num();
}