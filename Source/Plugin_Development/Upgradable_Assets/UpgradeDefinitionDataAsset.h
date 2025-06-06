
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UpgradeLevelData.h"
#include "UpgradeDefinitionDataAsset.generated.h"

UCLASS()
class PLUGIN_DEVELOPMENT_API UUpgradeDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Override path ID for this upgrade definition (if None, will use asset name)
	UPROPERTY(EditAnywhere, Category = "Upgrade")
	FName OverrideUpgradePath;

	// Array of upgrade levels
	UPROPERTY(EditAnywhere, Category = "Upgrade")
	TArray<FUpgradeLevelDataAsset> Levels;
};