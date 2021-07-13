
#include "RoomComponentActor.h"

ARoomComponentActor::ARoomComponentActor()
{
	bReplicates = true;
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);
	
	PrimaryActorTick.bCanEverTick = false;
}

int32 ARoomComponentActor::GetPrimaryISMCs(TArray<UInstancedStaticMeshComponent*>& PrimaryISMCs)
{
	PrimaryISMCs.Empty();
	TInlineComponentArray<UInstancedStaticMeshComponent*> ISMComps;
	GetComponents(ISMComps, false);
	for (UInstancedStaticMeshComponent* ISMComp : ISMComps)
	{
		if (IsValid(ISMComp) &&  ISMComp->GetAttachParent() == RootScene)
		{
			PrimaryISMCs.Add(ISMComp);
		}
	}
	return PrimaryISMCs.Num();
}

int32 ARoomComponentActor::GetAltISMCs(const UInstancedStaticMeshComponent* ParentISMC, TArray<UInstancedStaticMeshComponent*>& AltISMCs)
{
	TArray<USceneComponent*> AllChildren;
	UInstancedStaticMeshComponent* TmpISMC = nullptr;
	AltISMCs.Empty();
	if (IsValid(ParentISMC))
	{
		ParentISMC->GetChildrenComponents(true, AllChildren);
		for (USceneComponent* SC : AllChildren)
		{
			TmpISMC = Cast<UInstancedStaticMeshComponent>(SC);
			if (IsValid(TmpISMC))
			{
				AltISMCs.Add(TmpISMC);
			}
		}
	}
	return AltISMCs.Num();
}


bool ARoomComponentActor::ApplyAltMeshes()
{
	if (AltMeshPercent <= 0.0f) { return false; }
	TArray<UInstancedStaticMeshComponent*> PrimaryISMCs;
	TArray<UInstancedStaticMeshComponent*> AltISMCs;
	TArray<int32> RemovedPrimaryIndexes;
	TArray<FTransform> AltTransforms;
	FTransform TmpTransform;
	int32 InstanceIndex = 0;
	int32 InstanceCount = 0;	
	bool AnyAltsApplied = false;
	float AltPercent = AltMeshPercent > 1.0f ? 1.0f : AltMeshPercent;
	if (GetPrimaryISMCs(PrimaryISMCs) > 0)
	{
		for (UInstancedStaticMeshComponent* ISMC : PrimaryISMCs)
		{
			if (GetAltISMCs(ISMC, AltISMCs) > 0)
			{
				// Iterate primary instances, giving each a chance to become an alt instance.
				InstanceCount = ISMC->GetInstanceCount();
				RemovedPrimaryIndexes.Empty(static_cast<int32>(InstanceCount * AltPercent));
				AltTransforms.Empty(static_cast<int32>(InstanceCount * AltPercent));
				for (InstanceIndex = 0; InstanceIndex < InstanceCount; InstanceIndex++)
				{
					if (FMath::FRand() <= AltPercent)
					{
						// Get transform in world space
						ISMC->GetInstanceTransform(InstanceIndex, TmpTransform, true);
						AltTransforms.Add(TmpTransform);
						RemovedPrimaryIndexes.Add(InstanceIndex);
						AnyAltsApplied = true;
					}
				}
				// Remove primary instances. Iterate from end to start index because RemoveIndex can compact the array resluting in changed indexes >= the index removed.
				InstanceCount = RemovedPrimaryIndexes.Num();
				for (InstanceIndex = InstanceCount - 1; InstanceIndex >= 0; InstanceIndex--)
				{
					ISMC->RemoveInstance(RemovedPrimaryIndexes[InstanceIndex]);
				}
				// Add the alt instances
				for (FTransform AltTransform : AltTransforms)
				{
					AltISMCs[FMath::RandRange(0, AltISMCs.Num() - 1)]->AddInstanceWorldSpace(AltTransform);
				}
			}
		}
	}
	return AnyAltsApplied;
}


bool ARoomComponentActor::RevertToPrimaryMeshes()
{
	TArray<UInstancedStaticMeshComponent*> PrimaryISMCs;
	TArray<UInstancedStaticMeshComponent*> AltISMCs;
	TArray<FTransform> AltTransforms;
	FTransform TmpTransform;
	int32 InstanceIndex = 0;
	int32 InstanceCount = 0;
	bool AnyAltsRemoved = false;
	if (GetPrimaryISMCs(PrimaryISMCs) > 0)
	{
		for (UInstancedStaticMeshComponent* ISMC : PrimaryISMCs)
		{
			if (GetAltISMCs(ISMC, AltISMCs) > 0)
			{
				for (UInstancedStaticMeshComponent* AltISMC : AltISMCs)
				{
					InstanceCount = AltISMC->GetInstanceCount();
					AltTransforms.Empty(InstanceCount);
					for (InstanceIndex = 0; InstanceIndex < InstanceCount; InstanceIndex++)
					{
						AltISMC->GetInstanceTransform(InstanceIndex, TmpTransform);
						AltTransforms.Add(TmpTransform);
						AnyAltsRemoved = true;
					}
					AltISMC->ClearInstances();
					for (FTransform AltTransform : AltTransforms)
					{
						ISMC->AddInstance(AltTransform);
					}
				}				
			}
		}
	}
	return AnyAltsRemoved;
}