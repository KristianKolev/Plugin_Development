
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UpgradeDataContainers.h"
#include "UpgradeDefinitionDataAsset.generated.h"

UCLASS()
class PLUGIN_DEVELOPMENT_API UUpgradeDefinitionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Provide path ID for this upgrade definition (if None, will use asset name)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade System")
	FName UpgradePathId;

	// Each element in this array represents a level of the upgrade path
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade System")
	TArray<FUpgradeDefinitionAsset> Levels;
};