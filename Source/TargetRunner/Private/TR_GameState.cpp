// Fill out your copyright notice in the Description page of Project Settings.


#include "TR_GameState.h"
#include "ResourceFunctionLibrary.h"

FName ATR_GameState::GetGoodsNameForResource(const FResourceType& ResourceType)
{
	//FResourceTypeData ResourceData;
	//if (GetResourceTypeData(ResourceType, ResourceData))
	//{
	//	return ResourceData.GoodsNameOverride.IsNone() ? ResourceType.Type : ResourceData.GoodsNameOverride;
	//}
	//return FName();
	return ResourceType.Type;
}

bool ATR_GameState::GetResourceTypeData(const FResourceType& ForResourceType, FResourceTypeData& ResourceData)
{
	if (!IsValid(ResourceTypeDataTable)) { return false; }

	FResourceTypeData* ResourceDataRow = ResourceTypeDataTable->FindRow<FResourceTypeData>(ForResourceType.Code, "", false);
	if (ResourceDataRow)
	{
		ResourceData = *ResourceDataRow; //FResourceTypeData(ResourceDataRow);
		return true;
	}
	return false;
}
