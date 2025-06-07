// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UpgradeDataContainers.h"
#include "Engine/DataAsset.h"
#include "LeveUpVisualsDataAsset.generated.h"

/** TODO: Add a method (Maybe in the UpgradableComponent) to set the visuals of the actor on level up.
 * 
 */
UCLASS()
class PLUGIN_DEVELOPMENT_API UOnLeveUpVisualsDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade System")
	TMap<int32, UStaticMesh*> StaticMeshPerLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade System")
	TMap<int32, UMaterialInstance*> MaterialInstancePerLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade System")
	TMap<int32, UNiagaraSystem*> NiagaraSystemPerLevel;
};
