// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "MeshOption.h"
#include "ResourceTypeData.generated.h"

/*
* To be used in DataTables to describe detailsand assets for a ResourceType.
* Row names in the table should be in format: Category.Type.SubType
*    ex: Metal.Copper  or example with subtype:  Metal.Copper.Oxidized 
*/ 
USTRUCT(BlueprintType)
struct FResourceTypeData : public FTableRowBase
{
	GENERATED_BODY()

public:
		
	// Required
	// This should match the row name in the table.
	// This should be in format: Category.Type.SubType
    //    ex: Metal.Copper  or example with subtype:  Metal.Copper.Oxidized
	// Also note, the Type portion of the code == the related GoodsType.Name.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		FName Code;

	// Optional. Usually the GoodsType related to a ResourceType is GoodsType.Name == ResourceType.Type and this is specified by the Code.
	// Set this GoodsNameOverride to associate this ResourceType with a different GoodsType.
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	//	FName GoodsNameOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		int32 Tier;

	// The material applied to resource nodes that do not have bPreventOverrideMaterial = true.
	// Also the material used for particle meshes on FX.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TSoftObjectPtr<UMaterialInterface> DefaultMaterial;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	//	TArray<TSoftObjectPtr<UStaticMesh>> DefaultMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TArray<FMeshOption> NodeMeshes;

public:
	FResourceTypeData()
	{
		Code = FName();
		Tier = 0;
	}

	FResourceTypeData(const FResourceTypeData* OtherData)
	{
		if (OtherData)
		{
			Code = OtherData->Code;
			Tier = OtherData->Tier;
			DefaultMaterial = OtherData->DefaultMaterial;
			//DefaultMeshes = OtherData->DefaultMeshes;
			NodeMeshes = OtherData->NodeMeshes;
		}
	}

	//bool operator==(const FResourceTypeData& Other) const
	//{
	//	if (Code != Other.Code) return false;
	//	//if (Category != Other.Category) return false;
	//	//if (Type != Other.Type) return false;
	//	//if (SubType != Other.SubType) return false;
	//	if (GoodsNameOverride != Other.GoodsNameOverride) return false;
	//	return true;
	//}

	//bool operator==(const FResourceTypeData& Other)
	//{
	//	if (Code != Other.Code) return false;
	//	//if (Category != Other.Category) return false;
	//	//if (Type != Other.Type) return false;
	//	//if (SubType != Other.SubType) return false;
	//	if (GoodsNameOverride != Other.GoodsNameOverride) return false;
	//	return true;
	//}

	//bool operator==(FResourceTypeData& Other)
	//{
	//	if (Code != Other.Code) return false; 
	//	//if (Category != Other.Category) return false;
	//	//if (Type != Other.Type) return false;
	//	//if (SubType != Other.SubType) return false;
	//	if (GoodsNameOverride != Other.GoodsNameOverride) return false;
	//	return true;
	//}

	//bool operator!=(const FResourceTypeData& Other) const
	//{
	//	return !(*this == Other);
	//}

	//bool operator!=(const FResourceTypeData& Other)
	//{
	//	return !(*this == Other);
	//}

	//bool operator!=(FResourceTypeData& Other)
	//{
	//	return !(*this == Other);
	//}
};
