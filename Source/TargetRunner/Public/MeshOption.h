// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MeshOption.generated.h"


USTRUCT(BlueprintType)
struct FMeshOption
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float Weight = 1.0f;

	// Should we prevent overriding the material on this mesh.
	// Usually set to true for meshes that require their custom material.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		bool bPreventOverrideMaterial = false;
};