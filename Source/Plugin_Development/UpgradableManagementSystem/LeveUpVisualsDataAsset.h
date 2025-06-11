// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UpgradeDataContainers.h"
#include "Engine/DataAsset.h"
#include "LeveUpVisualsDataAsset.generated.h"

UENUM(BlueprintType)
enum class EMeshForMaterialSwap : uint8
{
	None = 0,
	StaticMesh,
	SkeletalMesh
};
/** 
 * Helper struct for describing “at this level, on this mesh, swap this slot to this material.” 
 */
USTRUCT(BlueprintType)
struct FMaterialSwapInfo
{
	GENERATED_BODY()

	/** Which mesh to swap materials on: Static or Skeletal */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMeshForMaterialSwap MeshForMaterialSwap = EMeshForMaterialSwap::SkeletalMesh;

	/** Which material index on that mesh */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaterialSlot = -1;

	/** The material to apply */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInterface* Material = nullptr;
};

USTRUCT(BlueprintType)
struct PLUGIN_DEVELOPMENT_API FMaterialSwapList
{
	GENERATED_BODY()

	/** All the swaps to apply at this level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Upgrade")
	TArray<FMaterialSwapInfo> MaterialSwaps;
};

/** TODO: Add a method (Maybe in the UpgradableComponent) to set the visuals of the actor on level up.
 * 
 */
UCLASS()
class PLUGIN_DEVELOPMENT_API UOnLeveUpVisualsDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Automatically applied if using UpgradableComponent's ChangeActorVisualsPerUpgradeLevel function
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade System")
	TMap<int32, UStaticMesh*> StaticMeshPerLevel;

	// Automatically applied if using UpgradableComponent's ChangeActorVisualsPerUpgradeLevel function
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade System")
	TMap<int32, USkeletalMesh*> SkeletalMeshPerLevel;

	/**
	 * Automatically applied if using UpgradableComponent's ChangeActorVisualsPerUpgradeLevel function
	 * Map each level to an array of material swaps to apply.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Upgrade System")
	TMap<int32, FMaterialSwapList> MaterialSwapsPerLevel;

	// Needs to be activated manually using the Spawn Niagara functions in BP
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade System")
	TMap<int32, UNiagaraSystem*> NiagaraSystemPerLevel;
};
