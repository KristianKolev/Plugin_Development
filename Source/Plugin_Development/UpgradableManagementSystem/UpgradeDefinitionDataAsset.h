
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade System")
	int32 MaxLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade System")
	TMap<FName, FCostsScalingSegment> CostScalingSegments;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade System")
	TArray<FTimeScalingSegment> TimeScalingSegments;
	
	// Each element in this array represents a level of the upgrade path
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade System")
	TArray<FUpgradeDefinitionAsset> LevelOverrides;
};